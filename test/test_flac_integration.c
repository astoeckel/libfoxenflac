/*
 *  libfoxenflac -- Tiny FLAC Decoder Library
 *  Copyright (C) 2018-2022  Andreas Stöckel
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#define _DEFAULT_SOURCE /* for popen() */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <foxen/flac.h>

#include "bitstream.h"

/* http://graphics.stanford.edu/~seander/bithacks.html#FixedSignExtend */
#define SIGN_EXTEND(x, b) \
	(int64_t)((x) ^ (1LU << ((b)-1U))) - (int64_t)(1LU << ((b)-1U))

/**
 * Safe version of strcpy copying to buffer of fixed size. Resulting string is
 * always NULL terminated.
 */
static char *strcpy_safe(char *tar, int *l, const char *src) {
	if (tar) {
		while (*l > 1U && *src) {
			*(tar++) = *(src++);
			(*l)--;
		}
		*tar = '\0';
	}
	return (*src) ? NULL : tar;
}

/**
 * Escapes argument for passing it to popen()
 */
static char *strcpy_escape_safe(char *tar, int *l, const char *src) {
	if (tar) {
		while (*l > 1U && *src) {
			const char c = *(src++);
			if (c == '\'') {
				if (*l > 4U) {
					*(tar++) = '\'';
					*(tar++) = '\\';
					*(tar++) = '\'';
					(*l) -= 3U;
				} else {
					return NULL; /* Abort, do not use half-escaped string */
				}
			}
			*(tar++) = c;
			(*l)--;
		}
		*tar = '\0';
	}
	return (*src) ? NULL : tar;
}

/**
 * Assembles the call to the reference flac decoder.
 */
static bool build_flac_command(char *tar, int l, const char *file) {
	const char *P = "flac -d '";
	const char *S =
	    "' --totally-silent --force-raw-format --endian big --sign signed -o -";
	tar = strcpy_safe(tar, &l, P);
	tar = strcpy_escape_safe(tar, &l, file);
	tar = strcpy_safe(tar, &l, S);
	return tar != NULL;
}

static void progress(fx_flac_t *flac, const char *fmt, uint64_t smpl_idx) {
	fprintf(stderr, fmt,
	        smpl_idx / fx_flac_get_streaminfo(flac, FLAC_KEY_N_CHANNELS),
	        fx_flac_get_streaminfo(flac, FLAC_KEY_N_SAMPLES));
}

static bool compare_to_reference_decoder(const char *file) {
	FILE *fin = NULL, *fflac = NULL;
	fx_flac_t *flac = NULL;
	bool ok = false;

	/* Open the input file */
	fin = fopen(file, "r");
	if (!fin) {
		fprintf(stderr, "Cannot open \"%s\"\n", file);
		goto fail;
	}

	/* Assemble the flac decoder comand */
	char buf[2048];
	if (!build_flac_command(buf, sizeof(buf), file)) {
		fprintf(stderr, "Filename too long.\n");
		goto fail;
	}

	/* Open the reference FLAC decoder */
	fflac = popen(buf, "r");
	if (!fflac) {
		fprintf(stderr,
		        "Command %s failed; cannot launch flac reference decoder.\n",
		        buf);
		goto fail;
	}

	uint8_t flac_buf[128];
	fx_bitstream_t flac_bitstream;
	fx_bitstream_init(&flac_bitstream);

	/* Instantiate the FLAC decoder and start decoding the file */
	flac = FX_FLAC_ALLOC_DEFAULT();
	int32_t out_buf[64];
	uint32_t buf_wr_cur = 0;
	uint64_t smpl_idx = 0;
	uint64_t byte_idx = 0;
	int bps = 0;
	while (true) {
		/* Read data from the input file */
		size_t to_read = sizeof(buf) - buf_wr_cur, n_read = 0;
		if (to_read > 0) {
			n_read = fread(buf + buf_wr_cur, 1, to_read, fin);
			buf_wr_cur += n_read;
		}

		/* Read from the buffer */
		uint32_t in_buf_len = buf_wr_cur;
		uint32_t out_buf_len = sizeof(out_buf) / sizeof(out_buf[0]);
		switch (fx_flac_process(flac, (uint8_t *)buf, &in_buf_len, out_buf,
		                        &out_buf_len)) {
			case FLAC_END_OF_METADATA:
				bps = fx_flac_get_streaminfo(flac, FLAC_KEY_SAMPLE_SIZE);
				if (bps != 16 && bps != 24) {
					fprintf(stderr,
					        "\n[WRN] %s: Not supported by reference decoder "
					        "RAW output!\n",
					        file);
					/* Not supported by reference decoder right now; we're
					 * probably fine :-) */
					ok = true;
					goto fail;
				}
				break;
			case FLAC_ERR:
				goto fail;
			default:
				break;
		}
		byte_idx += in_buf_len; /* Count the number of bytes read */

		/* Check whether we are at the end of the file */
		if (in_buf_len == 0U && out_buf_len == 0U && (n_read < to_read)) {
			break;
		}

		/* Compare the samples to those produced by the reference decoder */
		for (uint32_t i = 0; i < out_buf_len; i++) {
			/* Feed new data into the bitstream reader */
			while (!fx_bitstream_can_read(&flac_bitstream, bps)) {
				size_t n = fread(flac_buf, 1, sizeof(flac_buf), fflac);
				if (n == 0) {
					fprintf(stderr,
					        "\n[ERR] %s: Decoder producing phantom data.\n",
					        file);
					goto fail;
				}
				fx_bitstream_set_source(&flac_bitstream, flac_buf, n);
			}
			uint32_t in = fx_bitstream_read_msb(&flac_bitstream, bps);
			int32_t expected = SIGN_EXTEND(in, bps);
			int32_t is = (out_buf[i]) >> (32 - bps);
			if (is != expected) {
				fprintf(stderr, "\n[ERR] %s: 0x%08lX #%011ld %-11d != %-11d\n",
				        file, byte_idx, smpl_idx, is, expected);
				goto fail;
			}
			smpl_idx++;
			if (smpl_idx % 200000 == 0) {
				progress(flac, "\r[-->] Compared %11ld/%11ld samples...",
				         smpl_idx);
			}
		}

		/* Copy unread bytes to the beginning of the buffer and adjust the write
		   cursor. */
		for (uint32_t i = 0; i < buf_wr_cur - in_buf_len; i++) {
			buf[i] = buf[i + in_buf_len];
		}
		buf_wr_cur = buf_wr_cur - in_buf_len;
	}

	progress(flac, "\r[-->] Compared %11ld/%11ld samples...", smpl_idx);

	/* Make sure the flac decoder does not have any data left */
	/* TODO */
	if (fx_bitstream_can_read(&flac_bitstream, 1U)) {
		fprintf(stderr, "\n[ERR] %s: Premature end.\n", file);
		goto fail;
	}

	progress(flac, "\r[OK ] Compared %11ld/%11ld samples. OK!\n", smpl_idx);

	ok = true; /* Phew, we're done! */

fail:
	if (fin) {
		fclose(fin);
	}
	if (fflac) {
		fclose(fflac);
	}
	if (flac) {
		free(flac);
	}
	return ok;
}

int main(int argc, const char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: ./test_flac_integration <FLAC FILE>\n");
		return 1;
	}
	if (!compare_to_reference_decoder(argv[1])) {
		return 1;
	}
	return 0;
}
