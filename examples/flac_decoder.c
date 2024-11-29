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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <foxen/flac.h>

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <FLAC FILE>\n", argv[0]);
		return 1;
	}
	FILE *f = *argv[1] == '-' ? stdin : fopen(argv[1], "rb");
	if (!f) {
		fprintf(stderr, "Error opening file \"%s\"", argv[1]);
		return 1;
	}

	uint8_t in_buf[128];
	int32_t out_buf[512];
	uint32_t in_buf_wr_cur = 0;
	fx_flac_t *flac = FX_FLAC_ALLOC_DEFAULT();
	bool done_reading = false;
#if 0
	uint64_t smpl_idx = 0;
	uint64_t byte_idx = 0;
#endif
	while (true) {
		/* Read data from the input file */
		size_t to_read = sizeof(in_buf) - in_buf_wr_cur;
		size_t n_read = 0;
		if ((!done_reading) && (to_read > 0)) {
			n_read = fread(in_buf + in_buf_wr_cur, 1, to_read, f);
			if (n_read == 0) {
				done_reading = true;
				fprintf(stderr, "%s: Reached end of file.\n", argv[1]);
			}

			/* Advance the write cursor */
			in_buf_wr_cur += n_read;
		}

		/* Read from the buffer */
		uint32_t in_buf_len = in_buf_wr_cur;
		uint32_t out_buf_len = 512;
		switch (
		    fx_flac_process(flac, in_buf, &in_buf_len, out_buf, &out_buf_len)) {
			case FLAC_END_OF_METADATA:
				/* Can read metadata here */
				break;
			case FLAC_ERR:
				fprintf(stderr, "FLAC decoder in error state!\n");
				break;
			default:
				break;
		}

			/* Write decoded samples to stdout */
#if 0
		byte_idx += in_buf_len;
		for (uint32_t i = 0; i < out_buf_len; i++) {
/*			fprintf(stdout, "%08lX %09ld %11d\n", byte_idx, smpl_idx++, out_buf[i] >> 16);*/
			fprintf(stdout, "%09ld %11d\n", smpl_idx++, out_buf[i] >> 16);
		}
#else
		fwrite(out_buf, 4, out_buf_len, stdout);
#endif

		/* Copy unread bytes to the beginning of the buffer and adjust the write
		   cursor. */
		for (uint32_t i = 0; i < (in_buf_wr_cur - in_buf_len); i++) {
			in_buf[i] = in_buf[i + in_buf_len];
		}
		in_buf_wr_cur = in_buf_wr_cur - in_buf_len;

		/* Check whether we are at the end of the file */
		if ((in_buf_len == 0U) && (out_buf_len == 0U) && (n_read < to_read)) {
			break;
		}
	}
	free(flac);

	if (f != stdin) {
		fclose(f);
	}
}
