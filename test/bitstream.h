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

/**
 * @file bitstream.h
 *
 * Provides a utilities for reading individual bits from a bitstream. Note: this
 * code is heavily inspired by Fabian "ryg" Giesen's series of blog posts
 * "Reading bits in far too many ways".
 *
 * @author Andreas Stöckel
 */

#ifndef FOXEN_COMMON_BITSTREAM_H
#define FOXEN_COMMON_BITSTREAM_H

#include <assert.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * INTERFACE                                                                  *
 ******************************************************************************/

/**
 * Structure holding the current state of the bitstream reader.
 */
struct fx_bitstream {
	/**
	 * 64 bit word from which the data is extracted.
	 */
	uint64_t buf;

	/**
	 * Pointer at the source byte stream.
	 */
	uint8_t const *src;

	/**
	 * Pointer at the end of the source byte stream.
	 */
	uint8_t const *src_end;

	/**
	 * Position within the source byte stream in bits, i.e. the number of bits
	 * that have been consumed.
	 */
	uint8_t pos;
};

/**
 * Typedef for the fx_bitstream struct.
 */
typedef struct fx_bitstream fx_bitstream_t;

/**
 * Callback called whenever a fully byte has been consumed. This is useful for
 * CRC calculations.
 */
typedef void (*fx_bitstream_byte_callback_t)(uint8_t byte, void *data);

/**
 * Initializes the bitstream reader instance. Call fx_bitstream_set_source()
 * to set the byte buffer from which the bitstream reader should read its
 * data.
 *
 * @param reader is the bitstream reader instance that should be
 * initialized.
 */
static inline void fx_bitstream_init(fx_bitstream_t *reader) {
	reader->buf = 0U;
	reader->pos = sizeof(reader->buf) * 8U;
	reader->src = NULL;
	reader->src_end = NULL;
}

/**
 * Sets the backing source buffer for the bitstream. This function may only be
 * called if fx_bitstream_at_source_end() returns true OR the given pointer
 * are direct continuations of the previous data, i.e. are essentially set to
 * reader->src.
 *
 * @param reader is the bitstream reader instance for which the source byte
 * buffer should be set.
 * @param src is a pointer at the source byte buffer.
 * @param src_len is the length of the source byte buffer in bytes.
 */
static inline void fx_bitstream_set_source(fx_bitstream_t *reader,
                                           const uint8_t *src,
                                           uint32_t src_len);

/**
 * Returns true if the bitstream reader has read all bytes from the current
 * source and it is safe to set a new source. It may still be possible to read
 * a few bits from the stream, even if this function return true.
 */
static inline bool fx_bitstream_at_source_end(fx_bitstream_t *reader) {
	return reader->src == reader->src_end;
}

/**
 * Returns true if the corresponding read operation will be successful.
 *
 * @param reader is the bitstream reader instance from which the data should be
 * read.
 * @param n_bits is the number of bits that should be read from the bitstream
 * reader. Must be in 1 <= n_bits <= 57.
 * @return true if the number of available bits is smaller or equal to n_bits.
 */
static inline bool fx_bitstream_can_read(fx_bitstream_t *reader,
                                         uint8_t n_bits) {
	return (sizeof(reader->buf) * 8U) >= (n_bits + reader->pos);
}

/**
 * Reads up to 64 bits from the input buffer in MSB order. Note that this
 * function does not check whether the read operation returns valid data, so
 * make sure to call fx_bitstream_can_read() before reading.
 *
 * @param reader is the bitstream reader instance from which the data should be
 * read.
 * @param n_bits is the number of bits that should be read. Must be in
 * 1 <= n_bits <= 57.
 * @return an integer corresponding the the specified number of bits.
 */
static inline uint64_t fx_bitstream_read_msb(fx_bitstream_t *reader,
                                             uint8_t n_bits);

/**
 * Reads up to 64 bits from the input buffer in MSB order. Note that this
 * function does not check whether the read operation returns valid data, so
 * make sure to call fx_bitstream_can_read() before reading.
 *
 * @param reader is the bitstream reader instance from which the data should be
 * read.
 * @param n_bits is the number of bits that should be read. Must be in
 * 1 <= n_bits <= 57.
 * @param callback is called whenever a full byte is consumed. Note that this
 * includes a "virtual" set of zeros at the beginning of the bitstream.
 * @param callback_data is a user-defined pointer passed to the byte callback.
 * @return an integer corresponding the the specified number of bits.
 */
static inline uint64_t fx_bitstream_read_msb_ex(
    fx_bitstream_t *reader, uint8_t n_bits,
    fx_bitstream_byte_callback_t callback, void *callback_data);

/**
 * Reads up to 64 bits from the input buffer in MSB order without advancing the
 * buffer location. Note that this function does not check whether the read
 * operation returns valid data, so make sure to call fx_bitstream_can_read()
 * before reading.
 *
 * @param reader is the bitstream reader instance from which the data should be
 * read.
 * @param n_bits is the number of bits that should be read. Must be in
 * 1 <= n_bits <= 57.
 * @return an integer corresponding to the specified number of bits.
 */
static inline uint64_t fx_bitstream_peek_msb(fx_bitstream_t *reader,
                                             uint8_t n_bits);

/**
 * Combination of fx_bitstream_can_read and fx_bitstream_read_msb. Returns a
 * negative value if the desired number of bits cannot be read from the source.
 * If the given number of threads are available, returns the desired integer.
 *
 * @param reads is the bitstream reader instance from which the data should be
 * read.
 * @param n_bits is the number of bits that should be read. Must be in
 * 1 <= n_bits <= 57.
 * @return -1 if the desired number of bits is not available in the bitstream.
 * Otherwise the integer corresponding to the specified number of bits is
 * returned.
 */
static inline int64_t fx_bitstream_try_read_msb(fx_bitstream_t *reader,
                                                uint8_t n_bits) {
	return fx_bitstream_can_read(reader, n_bits)
	           ? (int64_t)fx_bitstream_read_msb(reader, n_bits)
	           : -1;
}

/**
 * Combination of fx_bitstream_can_read and fx_bitstream_read_msb. Returns a
 * negative value if the desired number of bits cannot be read from the source.
 * If the given number of threads are available, returns the desired integer.
 *
 * @param reads is the bitstream reader instance from which the data should be
 * read.
 * @param n_bits is the number of bits that should be read. Must be in
 * 1 <= n_bits <= 57.
 * @param callback is called whenever a full byte is consumed. Note that this
 * includes a "virtual" set of zeros at the beginning of the bitstream.
 * @param callback_data is a user-defined pointer passed to the byte callback.
 * @return -1 if the desired number of bits is not available in the bitstream.
 * Otherwise the integer corresponding to the specified number of bits is
 * returned.
 */
static inline int64_t fx_bitstream_try_read_msb_ex(
    fx_bitstream_t *reader, uint8_t n_bits,
    fx_bitstream_byte_callback_t callback, void *callback_data) {
	return fx_bitstream_can_read(reader, n_bits)
	           ? (int64_t)fx_bitstream_read_msb_ex(reader, n_bits, callback,
	                                               callback_data)
	           : -1;
}

/**
 * Combination of fx_bitstream_can_read and fx_bitstream_peek. Returns a
 * negative value if the desired number of bits cannot be read from the source.
 * If the given number of threads are available, returns the desired integer.
 * In contrast to fx_bitstream_try_read_msb() this function does not advance
 * the actual reader pointer.
 *
 * @param reads is the bitstream reader instance from which the data should be
 * read.
 * @param n_bits is the number of bits that should be read. Must be in
 * 1 <= n_bits <= 57.
 * @return -1 if the desired number of bits is not available in the bitstream.
 * Otherwise the integer corresponding to the specified number of bits is
 * returned.
 */
static inline int64_t fx_bitstream_try_peek_msb(fx_bitstream_t *reader,
                                                uint8_t n_bits) {
	return fx_bitstream_can_read(reader, n_bits)
	           ? (int64_t)fx_bitstream_peek_msb(reader, n_bits)
	           : -1;
}

/******************************************************************************
 * IMPLEMENTATION                                                             *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

#define BUFSIZE (sizeof(((fx_bitstream_t *)NULL)->buf) * 8U)

static inline void _fx_bitstream_fill_buf(fx_bitstream_t *reader) {
	while (reader->pos >= 8U && reader->src != reader->src_end) {
		reader->buf = (reader->buf << 8U) | *(reader->src++);
		reader->pos -= 8U;
	}
}

static inline uint64_t _fx_bitstream_read_msb(
    fx_bitstream_t *reader, uint8_t n_bits,
    fx_bitstream_byte_callback_t callback, void *callback_data) {
	assert((n_bits >= 1U) && (n_bits <= (BUFSIZE - 7U)));

	/* Copy the current buffer content, skip already read bits */
	uint64_t bits = reader->buf << reader->pos;

	/* If the callback is specified, issue bytes that were read entirely */
	const uint8_t pos_new = reader->pos + n_bits;
	if (callback) {
		const uint8_t i0 = reader->pos / 8U, i1 = pos_new / 8U;
		uint64_t buf = reader->buf << (i0 * 8U);
		for (uint8_t i = i0; i < i1; i++) {
			uint8_t byte = buf >> (BUFSIZE - 8U);
			callback(byte, callback_data);
			buf = buf << 8U;
		}
	}

	/* Advance the position */
	reader->pos = pos_new;

	/* Read new bytes from the byte stream */
	_fx_bitstream_fill_buf(reader);

	/* Mask out the "low" bits */
	return bits >> (BUFSIZE - n_bits);
}

/******************************************************************************
 * Public API                                                                 *
 ******************************************************************************/

static inline void fx_bitstream_set_source(fx_bitstream_t *reader,
                                           const uint8_t *src,
                                           uint32_t src_len) {
	reader->src = src;
	reader->src_end = src + src_len;
	_fx_bitstream_fill_buf(reader);
}

static inline uint64_t fx_bitstream_read_msb(fx_bitstream_t *reader,
                                             uint8_t n_bits) {
	return _fx_bitstream_read_msb(reader, n_bits, NULL, NULL);
}

static inline uint64_t fx_bitstream_read_msb_ex(
    fx_bitstream_t *reader, uint8_t n_bits,
    fx_bitstream_byte_callback_t callback, void *callback_data) {
	return _fx_bitstream_read_msb(reader, n_bits, callback, callback_data);
}

static inline uint64_t fx_bitstream_peek_msb(fx_bitstream_t *reader,
                                             uint8_t n_bits) {
	assert((n_bits >= 1U) && (n_bits <= (BUFSIZE - 7U)));
	return (reader->buf << reader->pos) >> (BUFSIZE - n_bits);
}

#ifdef __cplusplus
}
#endif

#endif /* FOXEN_COMMON_BITSTREAM_H */
