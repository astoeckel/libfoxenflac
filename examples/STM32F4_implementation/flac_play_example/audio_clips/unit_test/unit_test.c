#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include "audio_mem.h"

#include "foxen/flac.h"

#define FLAC_PRINT_FRAMES true

int main(int argc, char *argv[])
{

  // i have libfoxenflac. the following code is the examples/flac_decoder.c
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <FLAC FILE>\n", argv[0]);
		return 1;
	}
	FILE *f = *argv[1] == '-' ? stdin : fopen(argv[1], "rb");
	if (!f) {
		fprintf(stderr, "Error opening file \"%s\"", argv[1]);
		return 1;
	}
  
  char fn[] = "decoded/stereo.mat";
  FILE *fo = fopen(fn, "w");
	if (!fo) {
		fprintf(stderr, "Error opening file \"%s\"", fn);
		return 1;
	}

	uint8_t buf[128];
	int32_t out_buf[512];
	uint32_t buf_wr_cur = 0;
	fx_flac_t *flac = FX_FLAC_ALLOC_SUBSET_FORMAT_DAT();
	bool done = false;

  // flac blocking decoding
	while (!done) {
		/* Read data from the input file */
		size_t to_read =  sizeof(buf) - buf_wr_cur;
		if (to_read > 0) {
			size_t n_read = fread(buf + buf_wr_cur, 1, to_read, f);
			if (n_read == 0) {
				fprintf(stderr, "%s: Reached end of file.\n", argv[1]);
				done = true;
				break;
			}

			/* Advance the write cursor */
			buf_wr_cur += n_read;
		}

		/* Read from the buffer */
		uint32_t buf_len = buf_wr_cur;
		uint32_t out_buf_len = 512;
		switch (fx_flac_process(flac, buf, &buf_len, out_buf, &out_buf_len)) {
			case FLAC_END_OF_METADATA:
				/* Can read metadata here */
				break;
			case FLAC_ERR:
				fprintf(stderr, "FLAC decoder in error state!\n");
				done = true;
				break;
			default:
				break;
		}

		/* Write decoded samples to stdout */
#if FLAC_PRINT_FRAMES
    bool wsel = false;
    uint64_t smpl_idx = 0;
		for (uint32_t i = 0; i < out_buf_len; i++) {
      if (!wsel) {
			  fprintf(stdout, "%11d, ", out_buf[i] >> 16);
			  fprintf(fo, "%11d, ", out_buf[i] >> 16);
      }
      else {
			  fprintf(stdout, "%11d\n", out_buf[i] >> 16);
			  fprintf(fo, "%11d\n", out_buf[i] >> 16);
      }
      wsel = !wsel;
		}
#else
		fwrite(out_buf, 4, out_buf_len, stdout);
#endif

		/* Copy unread bytes to the beginning of the buffer and adjust the write
		   cursor. */
		for (uint32_t i = 0; i < buf_wr_cur - buf_len; i++) {
			buf[i] = buf[i + buf_len];
		}
		buf_wr_cur = buf_wr_cur - buf_len;
	}
	free(flac);

	if (f != stdin) {
		fclose(f);
	}

  fclose(fo);
  return 0;
}
