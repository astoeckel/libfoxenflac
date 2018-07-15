/*
 *  libfoxenflac -- FLAC decoder
 *  Copyright (C) 2018  Andreas Stöckel
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file flac.h
 *
 * Provides a decoder for FLAC (Free Lossless Audio Codec).
 *
 * @author Andreas Stöckel
 */

#ifndef FOXEN_FLAC_H
#define FOXEN_FLAC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FLAC_INVALID_METADATA_KEY 0x7FFFFFFFFFFFFFFFULL;

/**
 * Opaque struct representing a FLAC decoder.
 */
struct fx_flac;

/**
 * Typedef for the fx_flac struct.
 */
typedef struct fx_flac fx_flac_t;

/**
 * Enum representing the state of a FLAC decoder instance.
 */
enum fx_flac_state {
	/**
	 * The decoder is in an error state; the decoder cannot recover from this
	 * error. This error may for example occur if the data in the stream is
	 * invalid, or the stream has a format that is outside the maximum specs
	 * that are supported by the decoder. Call fx_flac_reset() and start anew!
	 */
	FLAC_ERR = -1,

	/**
	 * The decoder is currently in its initial state, fx_flac_process() has not
	 * been called.
	 */
	FLAC_INIT = 0,

	/**
	 * The decoder found the beginning of the metadata packet!
	 */
	FLAC_IN_METADATA = 1,

	/**
	 * The decoder is done reading the current metadata block, this may be
	 * followed by more metadata blocks, in which case the state is reset to
	 * FLAC_IN_METADATA.
	 */
	FLAC_END_OF_METADATA = 2,

	/**
	 * The decoder is currently searching for an audio frame.
	 */
	FLAC_SEARCH_FRAME = 3,

	/**
	 * The decoder is currently inside the stream of audio frames.
	 */
	FLAC_IN_FRAME = 5,

	/**
	 * The decoder reached the end of a block.
	 */
	FLAC_END_OF_FRAME = 6,

	/**
	 * The decoder reached the end of the stream.
	 */
	FLAC_END_OF_STREAM = 7,
};

/**
 * Typedef allow
 */
typedef enum fx_flac_state fx_flac_state_t;

/**
 * Enum used in fx_flac_get_streaminfo() to query metadata about the stream.
 */
enum fx_flac_streaminfo_key {
	FLAC_MIN_BLOCK_SIZE = 0,
	FLAC_MAX_BLOCK_SIZE = 1,
	FLAC_MIN_FRAME_SIZE = 2,
	FLAC_MAX_FRAME_SIZE = 3,
	FLAC_SAMPLE_RATE = 4,
	FLAC_N_CHANNELS = 5,
	FLAC_BITS_PER_SAMPLE = 6,
	FLAC_N_SAMPLES = 7,
	FLAC_MD5_SUM_0 = 128,
	FLAC_MD5_SUM_1 = 129,
	FLAC_MD5_SUM_2 = 130,
	FLAC_MD5_SUM_3 = 131,
	FLAC_MD5_SUM_4 = 132,
	FLAC_MD5_SUM_5 = 133,
	FLAC_MD5_SUM_6 = 134,
	FLAC_MD5_SUM_7 = 135,
	FLAC_MD5_SUM_8 = 136,
	FLAC_MD5_SUM_9 = 137,
	FLAC_MD5_SUM_A = 138,
	FLAC_MD5_SUM_B = 139,
	FLAC_MD5_SUM_C = 140,
	FLAC_MD5_SUM_D = 141,
	FLAC_MD5_SUM_E = 142,
	FLAC_MD5_SUM_F = 143,
};

typedef enum fx_flac_streaminfo_key fx_flac_streaminfo_key_t;

/**
 * Returns the size of the FLAC decoder instance in bytes. This assumes that the
 * FLAC audio that is being decoded uses the maximum settings, i.e. the largest
 * bit depth and block size.
 */
uint32_t fx_flac_size(void);

/**
 * Initializes the FLAC decoder at the given memory location. Each decoder can
 * decode exactly one stream at a time.
 *
 * @param mem is a pointer at the memory region at which the FLAC decoder should
 * store its private data. The memory region must be at last as large as
 * indicated by fx_flac_size(). May be NULL, in which case NULL is returned.
 * @return a pointer at the FLAC decoder instance; note that this pointer may be
 * different from what was passed to mem. However, you may still pass the
 * original `mem` as `inst` parameter to other functions. Returns NULL if the
 * input pointer is NULL.
 */
fx_flac_t *fx_flac_init(void *mem);

/**
 * Resets the FLAC decoder.
 *
 * @param inst is the FLAC decoder that should be reset.
 */
void fx_flac_reset(fx_flac_t *inst);

/**
 * Returns the current decoder state.
 *
 * @param inst is the FLAC decoder instance for which the state should be
 * returned.
 * @return the current state of the decoder.
 */
fx_flac_state_t fx_flac_get_state(const fx_flac_t *inst);

/**
 * Returns metadata about the FLAC stream that is currently being parsed. This
 * function may only be called if the decoder is in the state
 * FLAC_END_OF_METADATA or greater, otherwise the result may be undefined
 * (it will likely return zero for most of the metadata keys).
 *
 * @param inst is a pointer at the FLAC decoder instance for which the metadata
 * should be retrieved.
 * @param key is the metadata that should be retrieved.
 * @return the requested metadata value or FLAC_INVALID_METADATA_KEY if the
 * given key is unknown.
 */
int64_t fx_flac_get_streaminfo(const fx_flac_t *inst,
                               fx_flac_streaminfo_key_t key);

/**
 * Decodes the given raw FLAC data; the given data must be RAW FLAC data as
 * specified in the FLAC format specification https://xiph.org/flac/format.html
 * This function will always return right after the decoder transitions to a new
 * relevant state.
 *
 * @param inst is the decoder instance.
 * @param in is a pointer at the encoded bytestream.
 * @param in_len is a pointer at a integer containing the number of valid bytes
 * in "in". After the function returns, in will contain the number of bytes that
 * were actually read. This number may be zero if the decoder is in the FLAC_ERR
 * or FLAC_STREAM_DONE state, or the internal buffers are full and need to be
 * flushed to the provided output first.
 * @param out is a pointer at a memory region that will accept the decoded
 * interleaved audio data. Samples are decoded as 32-bit signed integer; the
 * minimum and maximum value will depend on the original bit depth of the audio
 * stored in the bitstream.
 * @param out_len is a pointer at an integer containing the number of available
 * signed 32-bit integers at the memory address pointed at by out. After the
 * function returns, this value will contain the
 * @return the current state of the decoder. If the state transitions to
 * FLAC_END_OF_METADATA, FLAC_END_OF_FRAME or FLAC_END_OF_STREAM this function
 * will return immediately; only the data up to the point causing the transition
 * has been read.
 */
fx_flac_state_t fx_flac_process(fx_flac_t *inst, const uint8_t *in,
                                uint32_t *in_len, int32_t *out,
                                uint32_t *out_len);

#ifdef __cplusplus
}
#endif
#endif /* FOXEN_FLAC_H */
