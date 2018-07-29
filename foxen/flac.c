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

#include <stdint.h>

#include <foxen/bitstream.h>
#include <foxen/flac.h>
#include <foxen/mem.h>

#if 0
/* Set FX_FLAC_NO_CRC if you control the input data and already performed other
   integrity checks. This makes the decoder significantly faster. */
#define FX_FLAC_NO_CRC
#endif

/******************************************************************************
 * DATATYPES                                                                  *
 ******************************************************************************/

/******************************************************************************
 * Enums and constants defined in the FLAC format specifiction                *
 ******************************************************************************/

/**
 * Possible metadata block types.
 */
typedef enum {
	META_TYPE_STREAMINFO = 0,
	META_TYPE_PADDING = 1,
	META_TYPE_APPLICATION = 2,
	META_TYPE_SEEKTABLE = 3,
	META_TYPE_VORBIS_COMMENT = 4,
	META_TYPE_CUESHEET = 5,
	META_TYPE_PICTURE = 6,
	META_TYPE_INVALID = 127
} fx_flac_metadata_type_t;

typedef enum { BLK_FIXED = 0, BLK_VARIABLE = 1 } fx_flac_blocking_strategy_t;

typedef enum {
	INDEPENDENT_MONO = 0,
	INDEPENDENT_STEREO = 1,
	INDEPENDENT_3C = 2,
	INDEPENDENT_4C = 3,
	INDEPENDENT_5C = 4,
	INDEPENDENT_6C = 5,
	INDEPENDENT_7C = 6,
	INDEPENDENT_8C = 7,
	LEFT_SIDE_STEREO = 8,
	RIGHT_SIDE_STEREO = 9,
	MID_SIDE_STEREO = 10,
} fx_flac_channel_assignment_t;

typedef enum {
	BLK_SIZE_RESERVED = 0,
	BLK_SIZE_192 = 1,
	BLK_SIZE_576 = 2,
	BLK_SIZE_1152 = 3,
	BLK_SIZE_2304 = 4,
	BLK_SIZE_4608 = 5,
	BLK_SIZE_READ_8BIT = 6,
	BLK_SIZE_READ_16BIT = 7,
	BLK_SIZE_256 = 8,
	BLK_SIZE_512 = 9,
	BLK_SIZE_1204 = 10,
	BLK_SIZE_2048 = 11,
	BLK_SIZE_4096 = 12,
	BLK_SIZE_8192 = 13,
	BLK_SIZE_16384 = 14,
	BLK_SIZE_32768 = 15
} fx_flac_block_size_t;

static const int32_t fx_flac_block_sizes_[] = {
    -1,  192, 576,  1152, 2304, 4608, 0,     0,
    256, 512, 1204, 2048, 4096, 8192, 16384, 32768};

typedef enum {
	FS_STREAMINFO = 0,
	FS_88_2KHZ = 1,
	FS_176_4KHZ = 2,
	FS_192KHZ = 3,
	FS_8KHZ = 4,
	FS_16KHZ = 5,
	FS_22_05KHZ = 6,
	FS_24KHZ = 7,
	FS_32KHZ = 8,
	FS_44_1KHZ = 9,
	FS_48KHZ = 10,
	FS_96KHZ = 11,
	FS_READ_8BIT_KHZ = 12,
	FS_READ_16BIT_HZ = 13,
	FS_READ_16BIT_DHZ = 14,
	FS_INVALID = 15
} fx_flac_sample_rate_t;

static const int32_t fx_flac_sample_rates_[] = {
    0,     88200, 176400, 192000, 8000, 16000, 22050, 24000,
    32000, 44100, 48000,  96000,  0,    0,     0,     -1};

typedef enum {
	SS_STREAMINFO = 0,
	SS_8BIT = 1,
	SS_12BIT = 2,
	SS_RESERVED_1 = 3,
	SS_16BIT = 4,
	SS_20BIT = 5,
	SS_24BIT = 6,
	SS_RESERVED_2 = 7
} fx_flac_sample_size_t;

static const int8_t fx_flac_sample_sizes_[] = {0, 8, 12, -1, 16, 20, 24, -1};

typedef enum {
	SFT_CONSTANT,
	SFT_VERBATIM,
	SFT_FIXED,
	SFT_LPC
} fx_flac_subframe_type_t;

typedef enum {
	RES_RICE = 0,
	RES_RICE2 = 1,
	RES_RESERVED_1 = 2,
	RES_RESERVED_2 = 3
} fx_flac_residual_method_t;

/******************************************************************************
 * Structs defined in the flac format specification                           *
 ******************************************************************************/

/**
 * Struc holding all the information stored in an individual block header.
 */
typedef struct {
	/**
	 * Set to 1 if this metadata block is the last metadata block in the
	 * stream.
	 */
	bool is_last;

	/**
	 * Type of the metadata block.
	 */
	fx_flac_metadata_type_t type;

	/**
	 * Length of the metadata block in bytes.
	 */
	uint32_t length;
} fx_flac_metadata_t;

/**
 * Data stored in the STREAMINFO header.
 */
typedef struct {
	uint16_t min_block_size;
	uint16_t max_block_size;
	uint32_t min_frame_size;
	uint32_t max_frame_size;
	uint32_t sample_rate;
	uint8_t n_channels;
	uint8_t sample_size;
	uint64_t n_samples;
	uint8_t md5_sum[16];
} fx_flac_streaminfo_t;

/**
 * Frame header prepended to each FLAC audio block.
 */
typedef struct {
	fx_flac_blocking_strategy_t blocking_strategy;
	fx_flac_block_size_t block_size_enum;
	fx_flac_sample_rate_t sample_rate_enum;
	fx_flac_channel_assignment_t channel_assignment;
	fx_flac_sample_size_t sample_size_enum;
	uint32_t block_size;
	uint32_t sample_rate;
	uint8_t channel_count;
	uint8_t sample_size;
	uint64_t sync_info;
	uint8_t crc8;
} fx_flac_frame_header_t;

/**
 * Header prepended to the channel-specific data of each FLAC audio block.
 */
typedef struct {
	/**
	 * Specifies the method used to encode the data.
	 */
	fx_flac_subframe_type_t type;

	/**
	 * Order of this frame.
	 */
	uint8_t order;

	/**
	 * Number of bits the decoded result has to be shifted to the left.
	 */
	uint8_t wasted_bits;

	/**
	 * Number of bits used to encode the linear predictor coefficients.
	 */
	uint8_t lpc_prec;

	/**
	 * Shift applied to the coefficients.
	 */
	int8_t lpc_shift;

	/**
	 * LPC coefficients. Number of used coefficients corresponds to the order.
	 */
	int32_t *lpc_coeffs;

	/**
	 * Method used to code the residual. FLAC currently only supports RICE and
	 * RICE2.
	 */
	fx_flac_residual_method_t residual_method;

	/**
	 * Number of partitions the signal is divided into.
	 */
	uint8_t rice_partition_order;

	/**
	 * RICE parameter, i.e. the logarithm of the divisor.
	 */
	uint8_t rice_parameter;

} fx_flac_subframe_header_t;

/**
 * Array containing the LPC coefficients for the fixed coding mode.
 */
static const int32_t _fx_flac_fixed_coeffs[5][4] = {
    {}, {1}, {2, -1}, {3, -3, 1}, {4, -6, 4, -1}};

/******************************************************************************
 * Internal state machine enums                                               *
 ******************************************************************************/

/**
 * More fine-grained state descriptor used in the internal state machine.
 */
typedef enum {
	FLAC_SYNC_INIT = 0,
	FLAC_SYNC_F = 100,
	FLAC_SYNC_L = 101,
	FLAC_SYNC_A = 102,
	FLAC_METADATA_HEADER = 200,
	FLAC_METADATA_SKIP = 201,
	FLAC_METADATA_SINFO = 202,
	FLAC_FRAME_SYNC = 300,
	FLAC_FRAME_HEADER = 400,
	FLAC_FRAME_HEADER_SYNC_INFO = 401,
	FLAC_FRAME_HEADER_AUX = 402,
	FLAC_FRAME_HEADER_CRC = 403,
	FLAC_SUBFRAME_HEADER = 500,
	FLAC_SUBFRAME_CONSTANT = 502,
	FLAC_SUBFRAME_FIXED = 503,
	FLAC_SUBFRAME_FIXED_RESIDUAL = 504,
	FLAC_SUBFRAME_LPC = 505,
	FLAC_SUBFRAME_LPC_HEADER = 506,
	FLAC_SUBFRAME_LPC_COEFFS = 507,
	FLAC_SUBFRAME_LPC_RESIDUAL = 508,
	FLAC_SUBFRAME_RICE_INIT = 509,
	FLAC_SUBFRAME_RICE = 510,
	FLAC_SUBFRAME_RICE_UNARY = 511,
	FLAC_SUBFRAME_RICE_VERBATIM = 512,
	FLAC_SUBFRAME_RICE_FINALIZE = 513,
	FLAC_SUBFRAME_VERBATIM = 514,
	FLAC_SUBFRAME_FINALIZE = 515
} fx_flac_private_state_t;

/******************************************************************************
 * Internal structs                                                           *
 ******************************************************************************/

/**
 * Private definition of the fx_flac structure.
 */
struct fx_flac {
	/**
	 * Bitstream reader used to read individual bits from the input.
	 */
	fx_bitstream_t bitstream;

	/**
	 * Current state of the decoder.
	 */
	fx_flac_state_t state;

	/**
	 * Current private state of the decoder.
	 */
	fx_flac_private_state_t priv_state;

	/**
	 * Number of bytes remaining to read for the current frame/block.
	 */
	uint32_t n_bytes_rem;

	/**
	 * Maximum numbers of samples in a single block, per channel.
	 */
	uint16_t max_block_size;

	/**
	 * Maximum number of channels supported by the decoder.
	 */
	uint8_t max_channels;

	/**
	 * Current coefficient.
	 */
	uint8_t coef_cur;

	/**
	 * Current rice partition.
	 */
	uint16_t partition_cur;

	/**
	 * Current sample index in the current rice partition (decremented to zero).
	 */
	uint16_t partition_sample;

	/**
	 * Current rice partition unary quotient counter.
	 */
	uint16_t rice_unary_counter;

	/**
	 * Current channel. This is reset at frame boundaries.
	 */
	uint8_t chan_cur;

	/**
	 * Pointer into the current block buffer.
	 */
	uint16_t blk_cur;

	/**
	 * Variable holding the checksum computed when reading the frame_header.
	 */
	uint8_t crc8;

	/**
	 * Variable holding the checksum of an entire frame. If this checksum does
	 * not match after decoding a frame, the entire frame is rejected.
	 */
	uint16_t crc16;

	/**
	 * Flag indicating whether the current metadata block is the last metadata
	 * block.
	 */
	fx_flac_metadata_t *metadata;

	/**
	 * Structure holding the current stream metadata.
	 */
	fx_flac_streaminfo_t *streaminfo;

	/**
	 * Structure holding the frame header.
	 */
	fx_flac_frame_header_t *frame_header;

	/**
	 * Structure holding the subframe header.
	 */
	fx_flac_subframe_header_t *subframe_header;

	/**
	 * Buffer used for storing the LPC coefficients.
	 */
	int32_t *qbuf;

	/**
	 * Structure holding the temporary/output buffers for each channel.
	 */
	int32_t *blkbuf[FLAC_MAX_CHANNEL_COUNT];
};

/******************************************************************************
 * PRIVATE CODE                                                               *
 ******************************************************************************/

/******************************************************************************
 * Initialization code utils                                                  *
 ******************************************************************************/

static bool _fx_flac_check_params(uint16_t max_block_size, uint8_t max_channels)
{
	return (max_block_size > 0U) && (max_channels > 0U) &&
	       (max_channels <= FLAC_MAX_CHANNEL_COUNT);
}

/******************************************************************************
 * FLAC enum decoders                                                         *
 ******************************************************************************/

static bool _fx_flac_decode_block_size(fx_flac_block_size_t block_size_enum,
                                       uint32_t *block_size)
{
	const int32_t bs = fx_flac_block_sizes_[(int)block_size_enum];
	if (bs < 0) {
		return false; /* Invalid */
	}
	else if (bs > 0) {
		*block_size = bs;
	}
	return true;
}

static bool _fx_flac_decode_sample_rate(fx_flac_sample_rate_t sample_rate_enum,
                                        uint32_t *sample_rate)
{
	const int32_t fs = fx_flac_sample_rates_[(int)sample_rate_enum];
	if (fs < 0) {
		return false; /* Invalid */
	}
	else if (fs > 0) {
		*sample_rate = fs;
	}
	return true;
}

static bool _fx_flac_decode_sample_size(fx_flac_sample_size_t sample_size_enum,
                                        uint8_t *sample_size)
{
	const int8_t ss = fx_flac_sample_sizes_[(int)sample_size_enum];
	if (ss < 0) {
		return false; /* Invalid */
	}
	else if (ss > 0) {
		*sample_size = ss;
	}
	return true;
}

/**
 * Returns the number of channels encoded in the frame header.
 */
static bool _fx_flac_decode_channel_count(
    fx_flac_channel_assignment_t channel_assignment, uint8_t *channel_count)
{
	*channel_count = (channel_assignment >= LEFT_SIDE_STEREO)
	                     ? 2U
	                     : (uint8_t)channel_assignment + 1U;
	return true;
}

/******************************************************************************
 * Decoding functions                                                         *
 ******************************************************************************/

static inline void _fx_flac_post_process_left_side(int32_t *blk1, int32_t *blk2,
                                                uint32_t blk_size)
{
	blk1 = (int32_t *)FX_ASSUME_ALIGNED(blk1);
	blk2 = (int32_t *)FX_ASSUME_ALIGNED(blk2);
	for (uint32_t i = 0U; i < blk_size; i++) {
		blk2[i] = blk1[i] - blk2[i];
	}
}

static inline void _fx_flac_post_process_right_side(int32_t *blk1, int32_t *blk2, uint32_t blk_size)
{
	blk1 = (int32_t *)FX_ASSUME_ALIGNED(blk1);
	blk2 = (int32_t *)FX_ASSUME_ALIGNED(blk2);
	for (uint32_t i = 0U; i < blk_size; i++) {
		blk1[i] = blk1[i] + blk2[i];
	}
}

static inline void _fx_flac_post_process_mid_side(int32_t *blk1, int32_t *blk2,
                                                uint32_t blk_size)
{
	blk1 = (int32_t *)FX_ASSUME_ALIGNED(blk1);
	blk2 = (int32_t *)FX_ASSUME_ALIGNED(blk2);
	for (uint32_t i = 0U; i < blk_size; i++) {
		/* Code libflac from stream_decoder.c */
		int32_t mid = blk1[i];
		int32_t side = blk2[i];
		mid = ((uint32_t)mid) << 1;
		mid |= (side & 1); /* Round correctly */
		blk1[i] = (mid + side) >> 1;
		blk2[i] = (mid - side) >> 1;
	}
}

static inline void _fx_flac_restore_lpc_signal(int32_t *blk, uint32_t blk_size,
                                               int32_t *lpc_coeffs,
                                               uint8_t lpc_order,
                                               int8_t lpc_shift)
{
	blk = (int32_t *)FX_ASSUME_ALIGNED(blk);
	lpc_coeffs = (int32_t *)FX_ASSUME_ALIGNED(lpc_coeffs);

	for (uint32_t i = lpc_order; i < blk_size; i++) {
		int64_t accu = 0;
		for (uint8_t j = 0; j < lpc_order; j++) {
			accu += (int64_t)lpc_coeffs[j] * (int64_t)blk[i - j - 1];
		}
		blk[i] = blk[i] + (accu >> lpc_shift);
	}
}

/******************************************************************************
 * Stream utility functions and macros                                        *
 ******************************************************************************/

#define ENSURE_BITS(n)                                 \
	if (!fx_bitstream_can_read(&inst->bitstream, n)) { \
		return false; /* Need more data */             \
	}

#define READ_BITS(n)                                         \
	(tmp_ = fx_bitstream_try_read_msb(&inst->bitstream, n)); \
	if (tmp_ < 0) {                                          \
		return false; /* Need more data */                   \
	}

#define READ_BITS_FAST(n) (tmp_ = fx_bitstream_read_msb(&inst->bitstream, n));

#define PEEK_BITS(n)                                         \
	(tmp_ = fx_bitstream_try_peek_msb(&inst->bitstream, n)); \
	if (tmp_ < 0) {                                          \
		return false; /* Need more data */                   \
	}

/* Update the frame checksum while reading data */

#define READ_BITS_CRC(n)                                     \
	(tmp_ = fx_bitstream_try_read_msb(&inst->bitstream, n)); \
	if (tmp_ < 0) {                                          \
		return false; /* Need more data */                   \
	}                                                        \
	_fx_flac_crc16(tmp_, n, &inst->crc16);

#define READ_BITS_FAST_CRC(n)                            \
	(tmp_ = fx_bitstream_read_msb(&inst->bitstream, n)); \
	_fx_flac_crc16(tmp_, n, &inst->crc16);

/* DCRC -> Dual CRC, update both the header and the frame checksum */

#define READ_BITS_DCRC(n)                                    \
	(tmp_ = fx_bitstream_try_read_msb(&inst->bitstream, n)); \
	if (tmp_ < 0) {                                          \
		return false; /* Need more data */                   \
	}                                                        \
	_fx_flac_crc8(tmp_, n, &inst->crc8);                     \
	_fx_flac_crc16(tmp_, n, &inst->crc16);

#define READ_BITS_FAST_DCRC(n)                           \
	(tmp_ = fx_bitstream_read_msb(&inst->bitstream, n)); \
	_fx_flac_crc8(tmp_, n, &inst->crc8);                 \
	_fx_flac_crc16(tmp_, n, &inst->crc16);

#define SYNC_BYTESTREAM()                        \
	{                                            \
		uint8_t n_ = inst->bitstream.pos & 0x07; \
		if (n_) {                                \
			READ_BITS(8U - n_);                  \
		}                                        \
	}

#define SYNC_BYTESTREAM_CRC()                    \
	{                                            \
		uint8_t n_ = inst->bitstream.pos & 0x07; \
		if (n_) {                                \
			READ_BITS_CRC(8U - n_);              \
		}                                        \
	}

/* http://graphics.stanford.edu/~seander/bithacks.html#FixedSignExtend */
#define SIGN_EXTEND(x, b) \
	(int64_t)((x) ^ (1LU << ((b)-1U))) - (int64_t)(1LU << ((b)-1U))

/**
 * Updates the header checksum with the given data.
 */
static void _fx_flac_crc8(uint64_t in, uint8_t n_bits, uint8_t *crc8)
{
#ifndef FX_FLAC_NO_CRC
	for (uint8_t i = n_bits; i > 0U; i--) {
		if (in & (1U << (i - 1U))) {
			*crc8 = (*crc8) ^ 0x80U;
		}
		if ((*crc8) & 0x80) {
			*crc8 = ((*crc8) << 1U) ^ 0x07U;
		}
		else {
			*crc8 = (*crc8) << 1U;
		}
	}
#endif
}

/**
 * Updates the frame payload checksum.
 */
static void _fx_flac_crc16(uint64_t in, uint8_t n_bits, uint16_t *crc16)
{
#ifndef FX_FLAC_NO_CRC
	for (uint8_t i = n_bits; i > 0U; i--) {
		if (in & (1U << (i - 1U))) {
			*crc16 = (*crc16) ^ 0x8000U;
		}
		if ((*crc16) & 0x8000U) {
			*crc16 = ((*crc16) << 1U) ^ 0x8005;
		}
		else {
			*crc16 = (*crc16) << 1U;
		}
	}
#endif
}

static bool _fx_flac_reader_utf8_coded_int(fx_flac_t *inst, uint8_t max_n,
                                           uint64_t *tar)
{
	int64_t tmp_; /* Used by the READ_BITS macro */

	ENSURE_BITS(max_n * 8U);
	/* Read the first byte */
	uint8_t v = READ_BITS_FAST_DCRC(8U);

	/* Count the number of ones in the first byte */
	uint8_t n_ones = 0U;
	while (v & 0x80U) {
		v = v << 1U;
		n_ones++;
	}

	/* Abort if the number of bytes to read is larger than max_n */
	if (n_ones > max_n) {
		inst->priv_state = FLAC_FRAME_SYNC; /* Invalid header */
		return true;
	}

	/* Shift v back and store in var */
	*tar = (v >> n_ones);

	/* Read all continuation bytes */
	for (uint8_t i = 1U; i < n_ones; i++) {
		v = READ_BITS_FAST_DCRC(8U);
		/* Abort if continuation byte doesn't start with correct sequence */
		if ((v & 0xC0U) != 0x80) {
			inst->priv_state = FLAC_FRAME_SYNC; /* Invalid header */
			return true;
		}
		*tar = ((*tar) << 6U) | (v & 0x3F);
	}
	return true;
}

/******************************************************************************
 * Private decoder state machine                                              *
 ******************************************************************************/

/*
 * Note: the boolean return value of these functions indicates whether they
 * rand out of data -- true indicates that there is still enough data left,
 * false indicates that the outer state machine should return to the user code
 * to read more data. The return value does NOT indicate success/failure. This
 * is what inst->state == FLAC_ERR is for.
 */

static bool _fx_flac_handle_err(fx_flac_t *inst)
{
	/* TODO: Add flags to fx_flac_t which control this behaviour */

	/* If an error happens while searching for metadata, this is fatal. */
	if (inst->state < FLAC_END_OF_METADATA) {
		inst->state = FLAC_ERR;
		return false;
	}

	/* Otherwise just try to re-synchronise with the stream by searching for the
	   next frame */
	inst->state = FLAC_SEARCH_FRAME;
	inst->priv_state = FLAC_FRAME_SYNC;
	return true;
}

/**
 * Statemachine used to search the beginning of the stream. This (for example)
 * skips IDv3 tags prepended to the file.
 */
static bool _fx_flac_process_init(fx_flac_t *inst)
{
	int64_t tmp_; /* Used by the READ_BITS macro */
	/* Search for the 'fLaC' sync word */
	uint8_t byte = READ_BITS(8);
	switch (inst->priv_state) {
		case FLAC_SYNC_INIT:
			if (byte == 'f') {
				inst->priv_state = FLAC_SYNC_F;
			}
			break;
		case FLAC_SYNC_F:
			if (byte == 'L') {
				inst->priv_state = FLAC_SYNC_L;
			}
			else {
				inst->priv_state = FLAC_SYNC_INIT;
			}
			break;
		case FLAC_SYNC_L:
			if (byte == 'a') {
				inst->priv_state = FLAC_SYNC_A;
			}
			else {
				inst->priv_state = FLAC_SYNC_INIT;
			}
			break;
		case FLAC_SYNC_A:
			if (byte == 'C') {
				inst->state = FLAC_IN_METADATA;
				inst->priv_state = FLAC_METADATA_HEADER;
			}
			else {
				inst->priv_state = FLAC_SYNC_INIT;
			}
			break;
		default:
			return _fx_flac_handle_err(inst);
	}
	return true;
}

static bool _fx_flac_process_in_metadata(fx_flac_t *inst)
{
	int64_t tmp_; /* Used by the READ_BITS macro */
	switch (inst->priv_state) {
		case FLAC_METADATA_HEADER:
			ENSURE_BITS(32U);
			inst->metadata->is_last = READ_BITS_FAST(1U);
			inst->metadata->type = (fx_flac_metadata_type_t)READ_BITS_FAST(7U);
			if (inst->metadata->type == META_TYPE_INVALID) {
				return _fx_flac_handle_err(inst);
			}
			inst->metadata->length = inst->n_bytes_rem = READ_BITS_FAST(24U);
			if (inst->metadata->type == META_TYPE_STREAMINFO) {
				inst->priv_state = FLAC_METADATA_SINFO;
				/* The stream info header must be exactly 33 bytes long */
				if (inst->metadata->length != 34U) {
					return _fx_flac_handle_err(inst);
				}
			}
			else {
				inst->priv_state = FLAC_METADATA_SKIP;
			}
			break;
		case FLAC_METADATA_SINFO:
			switch (inst->n_bytes_rem) {
				case 34U:
					inst->streaminfo->min_block_size = READ_BITS(16U);
					inst->n_bytes_rem -= 2U;
					break;
				case 32U:
					inst->streaminfo->max_block_size = READ_BITS(16U);
					inst->n_bytes_rem -= 2U;
					break;
				case 30U:
					inst->streaminfo->min_frame_size = READ_BITS(24U);
					inst->n_bytes_rem -= 3U;
					break;
				case 27U:
					inst->streaminfo->max_frame_size = READ_BITS(24U);
					inst->n_bytes_rem -= 3U;
					break;
				case 24U:
					ENSURE_BITS(28U);
					inst->streaminfo->sample_rate = READ_BITS_FAST(20U);
					inst->streaminfo->n_channels = 1U + READ_BITS_FAST(3U);
					inst->streaminfo->sample_size = 1U + READ_BITS_FAST(5U);
					inst->n_bytes_rem -= 4U;
					break;
				case 20U:
					inst->streaminfo->n_samples = READ_BITS(36U);
					inst->n_bytes_rem -= 4U;
					break;
				case 1U ... 16U:
					inst->streaminfo->md5_sum[16U - inst->n_bytes_rem] =
					    READ_BITS(8);
					inst->n_bytes_rem -= 1U;
					break;
				case 0U:
					/* Use the FLAC_END_OF_METADATA_SKIP state logic below */
					inst->priv_state = FLAC_METADATA_SKIP;
					break;
				default:
					return _fx_flac_handle_err(inst);
			}
			break;
		case FLAC_METADATA_SKIP: {
			const uint8_t n_read =
			    (inst->n_bytes_rem >= 7U) ? 7U : inst->n_bytes_rem;
			if (n_read == 0U) { /* We read all the data for this block */
				if (inst->metadata->is_last) {
					/* Last metadata block, transition to the next state */
					inst->state = FLAC_END_OF_METADATA;
				}
				else {
					/* End of metadata block, read the next one */
					inst->priv_state = FLAC_METADATA_HEADER;
				}
				break;
			}
			READ_BITS(n_read * 8U);
			inst->n_bytes_rem -= n_read;
			break;
		}
		default:
			return _fx_flac_handle_err(inst); /* Internal error */
	}
	return true;
}

static bool _fx_flac_process_search_frame(fx_flac_t *inst)
{
	int64_t tmp_; /* Used by the READ_BITS macro */
	fx_flac_frame_header_t *fh = inst->frame_header;
	fx_flac_streaminfo_t *si = inst->streaminfo;
	switch (inst->priv_state) {
		case FLAC_FRAME_SYNC:
			/* Synchronise with the underlying bytestream */
			SYNC_BYTESTREAM();

			ENSURE_BITS(15U);
			uint16_t sync_code = PEEK_BITS(15U);
			if (sync_code != 0x7FFCU) {
				READ_BITS(8U); /* Next byte (assume frames are byte aligned). */
				return true;
			}
			else {
				inst->crc8 = 0U; /* Reset the checksums */
				inst->crc16 = 0U;
				inst->priv_state = FLAC_FRAME_HEADER;
				READ_BITS_FAST_DCRC(15U);
			}
			break;
		case FLAC_FRAME_HEADER:
			ENSURE_BITS(17U);

			/* Read the frame header bits */
			fh->blocking_strategy =
			    (fx_flac_blocking_strategy_t)READ_BITS_FAST_DCRC(1U);
			fh->block_size_enum = (fx_flac_block_size_t)READ_BITS_FAST_DCRC(4U);
			fh->sample_rate_enum =
			    (fx_flac_sample_rate_t)READ_BITS_FAST_DCRC(4U);
			fh->channel_assignment =
			    (fx_flac_channel_assignment_t)READ_BITS_FAST_DCRC(4U);
			fh->sample_size_enum =
			    (fx_flac_sample_size_t)READ_BITS_FAST_DCRC(3U);
			READ_BITS_FAST_DCRC(1U);
			if (tmp_ != 0U || fh->channel_assignment > MID_SIDE_STEREO) {
				return _fx_flac_handle_err(inst); /* Invalid header */
			}

			/* Copy sample rate and sample size from the streaminfo */
			fh->sample_rate = si->sample_rate;
			fh->sample_size = si->sample_size;

			/* Decode the individual enums */
			if (!_fx_flac_decode_block_size(fh->block_size_enum,
			                                &fh->block_size) ||
			    !_fx_flac_decode_sample_rate(fh->sample_rate_enum,
			                                 &fh->sample_rate) ||
			    !_fx_flac_decode_sample_size(fh->sample_size_enum,
			                                 &fh->sample_size) ||
			    !_fx_flac_decode_channel_count(fh->channel_assignment,
			                                   &fh->channel_count)) {
				inst->priv_state = FLAC_FRAME_SYNC; /* Got invalid value */
				break;
			}
			inst->priv_state = FLAC_FRAME_HEADER_SYNC_INFO;
			break;
		case FLAC_FRAME_HEADER_SYNC_INFO:
			if (!_fx_flac_reader_utf8_coded_int(
			        inst, (fh->blocking_strategy == BLK_VARIABLE) ? 7U : 6U,
			        &fh->sync_info)) {
				return false;
			}
			inst->priv_state = FLAC_FRAME_HEADER_AUX;
			break;
		case FLAC_FRAME_HEADER_AUX:
			ENSURE_BITS(32U);
			/* Read block size/sample rate if not directly packed into the
			   previous header */
			switch (fh->block_size_enum) {
				case BLK_SIZE_READ_8BIT:
					fh->block_size = 1U + READ_BITS_FAST_DCRC(8U);
					break;
				case BLK_SIZE_READ_16BIT:
					fh->block_size = 1U + READ_BITS_FAST_DCRC(16U);
					break;
				default:
					break;
			}
			switch (fh->sample_rate_enum) {
				case FS_READ_8BIT_KHZ:
					fh->sample_rate = 1000UL * READ_BITS_FAST_DCRC(8U);
					break;
				case FS_READ_16BIT_HZ:
					fh->sample_rate = READ_BITS_FAST_DCRC(16U);
					break;
				case FS_READ_16BIT_DHZ:
					fh->sample_rate = 10UL * READ_BITS_FAST_DCRC(16U);
					break;
				default:
					break;
			}
			inst->priv_state = FLAC_FRAME_HEADER_CRC;
			break;
		case FLAC_FRAME_HEADER_CRC:
			/* Read the CRC8 checksum, make sure it equals the checksum written
			   to the header. If not, this is not a valid header. Continue
			   searching. */
			fh->crc8 = READ_BITS_CRC(8U);
#ifndef FX_FLAC_NO_CRC
			if (fh->crc8 != inst->crc8) {
				return _fx_flac_handle_err(inst);
			}
#endif

			/* Make sure the decode has enough space */
			if ((fh->block_size > inst->max_block_size) ||
			    (fh->channel_count > inst->max_channels)) {
				return _fx_flac_handle_err(inst);
			}

			/* Decode the subframes */
			inst->state = FLAC_IN_FRAME;
			inst->priv_state = FLAC_SUBFRAME_HEADER;
			inst->chan_cur = 0U; /* Start with the first channel */
			break;
		default:
			return _fx_flac_handle_err(inst);
	}
	return true;
}

static bool _fx_flac_process_in_frame(fx_flac_t *inst)
{
	int64_t tmp_; /* Used by the READ_BITS macro */
	fx_flac_frame_header_t *fh = inst->frame_header;
	fx_flac_subframe_header_t *sfh = inst->subframe_header;
	int32_t *blk = inst->blkbuf[inst->chan_cur % FLAC_MAX_CHANNEL_COUNT];
	const uint32_t blk_n = fh->block_size;

	/* Figure out the number of bits to read for sample. This depends on the
	   channel assignment. */
	uint8_t bps = fh->sample_size - sfh->wasted_bits;
	if ((fh->channel_assignment == LEFT_SIDE_STEREO && inst->chan_cur == 1) ||
	    (fh->channel_assignment == RIGHT_SIDE_STEREO && inst->chan_cur == 0) ||
	    (fh->channel_assignment == MID_SIDE_STEREO && inst->chan_cur == 1)) {
		bps++;
	}

	/* Discard frames with invalid bits per sample values */
	if (bps == 0U || bps > 32U) {
		return _fx_flac_handle_err(inst);
	}

	/* This flag is set to false whenever a state in the state machine
	   encounters and error. */
	switch (inst->priv_state) {
		case FLAC_SUBFRAME_HEADER: {
			ENSURE_BITS(40U);

			/* Reset the block write cursor, make sure initial blk sample is set
			   to zero for zero-order fixed LPC */
			inst->blk_cur = 0U;
			blk[0U] = 0U;

			/* Read a zero padding bit. This must be zero. */
			uint8_t padding = READ_BITS_FAST_CRC(1U);
			bool valid = padding == 0U;

			/* Read the frame type and order */
			uint8_t type = READ_BITS_FAST_CRC(6U);
			if (type & 0x20U) {
				sfh->order = (type & 0x1FU) + 1U;
				sfh->type = SFT_LPC;
				sfh->lpc_coeffs = inst->qbuf;
				inst->priv_state = FLAC_SUBFRAME_LPC;
			}
			else if (type & 0x10U) {
				return _fx_flac_handle_err(inst);
			}
			else if (type & 0x08U) {
				sfh->order = type & 0x07U;
				sfh->type = SFT_FIXED;
				sfh->lpc_shift = 0;
				inst->priv_state = FLAC_SUBFRAME_FIXED;
				valid = valid && (sfh->order <= 4U);
				if (valid) {
					sfh->lpc_coeffs =
					    (int32_t *)_fx_flac_fixed_coeffs[sfh->order];
				}
			}
			else if ((type & 0x04U) || (type & 0x02U)) {
				return _fx_flac_handle_err(inst);
			}
			else if (type & 0x01U) {
				sfh->type = SFT_VERBATIM;
				inst->priv_state = FLAC_SUBFRAME_VERBATIM;
			}
			else {
				sfh->type = SFT_CONSTANT;
				inst->priv_state = FLAC_SUBFRAME_CONSTANT;
			}

			/* Read the "wasted_bits" flag */
			uint8_t wasted_bits = READ_BITS_FAST_CRC(1U);
			if (wasted_bits) {
				for (uint8_t i = 1U; i <= 30U; i++) {
					const uint8_t bit = READ_BITS_FAST_CRC(1U);
					if (bit == 1U) {
						sfh->wasted_bits = i;
						break;
					}
				}
				valid = valid && (sfh->wasted_bits > 0U) &&
				        (sfh->wasted_bits < fh->sample_size);
			}

			/* Make sure the block is large enough for the initial samples */
			valid = valid && (blk_n >= sfh->order);
			if (!valid) {
				_fx_flac_handle_err(inst);
			}
			break;
		}
		case FLAC_SUBFRAME_CONSTANT: {
			/* Read a single sample value and spread it over the entire block
			   buffer for this subframe. */
			blk[0U] = READ_BITS_CRC(bps);
			blk[0U] = SIGN_EXTEND(blk[0U], bps);
			for (uint16_t i = 1U; i < blk_n; i++) {
				blk[i] = blk[0U];
			}
			inst->priv_state = FLAC_SUBFRAME_FINALIZE;
			break;
		}
		case FLAC_SUBFRAME_VERBATIM:
		case FLAC_SUBFRAME_FIXED:
		case FLAC_SUBFRAME_LPC: {
			/* Either just read up to "order" samples, or the entire block */
			const uint32_t n = (sfh->type == SFT_VERBATIM) ? blk_n : sfh->order;
			while (inst->blk_cur < n) {
				blk[inst->blk_cur] = READ_BITS_CRC(bps);
				blk[inst->blk_cur] = SIGN_EXTEND(blk[inst->blk_cur], bps);
				inst->blk_cur++;
			}
			if (inst->blk_cur == n) { /* Go to the next state */
				inst->priv_state =
				    (fx_flac_private_state_t)((int)inst->priv_state + 1U);
			}
			break;
		}
		case FLAC_SUBFRAME_LPC_HEADER: {
			/* Read the coefficient precision as well as the shift value */
			ENSURE_BITS(9U);
			const uint8_t prec = READ_BITS_FAST_CRC(4U);
			const uint8_t shift = READ_BITS_FAST_CRC(5U);
			if (prec == 15U) { /* Precision of 15 bits is invalid */
				return _fx_flac_handle_err(inst);
			}
			sfh->lpc_prec = prec + 1U;
			sfh->lpc_shift = SIGN_EXTEND(shift, 5U);
			if (sfh->lpc_shift < 0) {
				return _fx_flac_handle_err(inst);
			}
			inst->coef_cur = 0U;
			inst->priv_state = FLAC_SUBFRAME_LPC_COEFFS;
			break;
		}
		case FLAC_SUBFRAME_LPC_COEFFS:
			/* Read the individual predictor coefficients */
			while (inst->coef_cur < sfh->order) {
				uint32_t coef = READ_BITS_CRC(sfh->lpc_prec);
				sfh->lpc_coeffs[inst->coef_cur] =
				    SIGN_EXTEND(coef, sfh->lpc_prec);
				inst->coef_cur++;
			}
			inst->priv_state = FLAC_SUBFRAME_LPC_RESIDUAL;
			break;
		case FLAC_SUBFRAME_FIXED_RESIDUAL:
		case FLAC_SUBFRAME_LPC_RESIDUAL: {
			ENSURE_BITS(6U);

			/* Read the residual encoding type and the rice partition order */
			sfh->residual_method =
			    (fx_flac_residual_method_t)READ_BITS_FAST_CRC(2U);
			if (sfh->residual_method > RES_RICE2) {
				return _fx_flac_handle_err(inst);
			}
			sfh->rice_partition_order = READ_BITS_FAST_CRC(4U);
			inst->partition_cur = 0U;
			inst->priv_state = FLAC_SUBFRAME_RICE_INIT;
			break;
		}
		case FLAC_SUBFRAME_RICE_INIT: {
			/* Read the Rice parameter */
			ENSURE_BITS(10U);

			uint8_t n_bits = (sfh->residual_method == RES_RICE) ? 4U : 5U;
			sfh->rice_parameter = READ_BITS_FAST_CRC(n_bits);
			if (sfh->rice_parameter == ((1U << n_bits) - 1U)) {
				sfh->rice_parameter = READ_BITS_FAST_CRC(5U);
				inst->priv_state = FLAC_SUBFRAME_RICE_VERBATIM;
			}
			else {
				inst->priv_state = FLAC_SUBFRAME_RICE_UNARY;
				inst->rice_unary_counter = 0U;
			}

			/* Compute the number of samples to read */
			inst->partition_sample = blk_n >> sfh->rice_partition_order;
			if (inst->partition_cur == 0U) {
				/* First partition alread includes verbatim samples */
				if (inst->partition_sample < sfh->order) {
					return _fx_flac_handle_err(
					    inst); /* Number of samples is negative */
				}
				inst->partition_sample -= sfh->order;
			}

			/* Make sure we're never writing beyond the buffer for this
			   channel */
			if ((inst->partition_sample + inst->blk_cur) > blk_n) {
				return _fx_flac_handle_err(inst);
			}
			break;
		}
		case FLAC_SUBFRAME_RICE:
		case FLAC_SUBFRAME_RICE_UNARY:
			/* Read the individual rice samples */
			while (inst->partition_sample > 0U) {
				/* Read the unary part of the Rice encoded sample bit-by-bit */
				if (inst->priv_state == FLAC_SUBFRAME_RICE_UNARY) {
					while (true) {
						const uint8_t bit = READ_BITS_CRC(1U);
						if (bit) {
							break;
						}
						inst->rice_unary_counter++;
					}
				}

				/* If there are no more bits left below, make sure we end up
				   here instead of going through the unary decoder again. */
				inst->priv_state = FLAC_SUBFRAME_RICE;

				/* Read the remainder */
				uint32_t r = 0U;
				if (sfh->rice_parameter > 0U) {
					r = READ_BITS_CRC(sfh->rice_parameter);
				}
				const uint16_t q = inst->rice_unary_counter;
				const uint32_t val = (q << sfh->rice_parameter) | r;

				/* Last bit determines sign */
				if (val & 1) {
					blk[inst->blk_cur] = -((int32_t)(val >> 1)) - 1;
				}
				else {
					blk[inst->blk_cur] = (int32_t)(val >> 1);
				}

				/* Read the next sample */
				inst->rice_unary_counter = 0U;
				inst->priv_state = FLAC_SUBFRAME_RICE_UNARY;
				inst->blk_cur++;
				inst->partition_sample--;
			}
			inst->priv_state = FLAC_SUBFRAME_RICE_FINALIZE;
			break;
		case FLAC_SUBFRAME_RICE_VERBATIM: {
			/* Samples are encoded in verbatim in this partition */
			const uint8_t bps = sfh->rice_parameter;
			while (inst->partition_sample > 0U) {
				blk[inst->blk_cur] = (bps == 0) ? 0U : READ_BITS_CRC(bps);
				blk[inst->blk_cur] = SIGN_EXTEND(blk[inst->blk_cur], bps);
				inst->blk_cur++;
				inst->partition_sample--;
			}
			inst->priv_state = FLAC_SUBFRAME_RICE_FINALIZE;
			break;
		}
		case FLAC_SUBFRAME_RICE_FINALIZE:
			/* Go to the next partition or finalize this subframe */
			inst->partition_cur++;
			if (inst->partition_cur == (1U << sfh->rice_partition_order)) {
				/* Decode the residual */
				_fx_flac_restore_lpc_signal(blk, blk_n, sfh->lpc_coeffs,
				                            sfh->order, sfh->lpc_shift);
				inst->priv_state = FLAC_SUBFRAME_FINALIZE;
			}
			else {
				inst->priv_state = FLAC_SUBFRAME_RICE_INIT;
			}
			break;
		case FLAC_SUBFRAME_FINALIZE: {
			/* There is another subframe to read, continue! */
			inst->chan_cur++; /* Go to the next channel */
			if (inst->chan_cur < fh->channel_count) {
				inst->priv_state = FLAC_SUBFRAME_HEADER;
				break;
			}

			/* Synchronise with the underlying byte stream */
			SYNC_BYTESTREAM_CRC();

			/* Read the CRC16 sum, resync if it doesn't match our own */
			uint16_t crc16 = READ_BITS(16U);
#ifndef FX_FLAC_NO_CRC
			if (crc16 != inst->crc16) {
				return _fx_flac_handle_err(inst);
			}
#else
			(void)crc16;
#endif

			/* Post process side-stereo */
			int32_t *c1 = inst->blkbuf[0], *c2 = inst->blkbuf[1];
			switch (fh->channel_assignment) {
				case LEFT_SIDE_STEREO:
					_fx_flac_post_process_left_side(c1, c2, blk_n);
					break;
				case RIGHT_SIDE_STEREO:
					_fx_flac_post_process_right_side(c1, c2, blk_n);
					break;
				case MID_SIDE_STEREO:
					_fx_flac_post_process_mid_side(c1, c2, blk_n);
					break;
				default:
					break;
			}

			/* Apply the "wasted bits" transformation, i.e. multiply the output
			   by the corresponding power of two. Furthermore, shift the output
			   such that the resulting int32 stream can be played back. */
			uint8_t shift = sfh->wasted_bits + (32U - fh->sample_size);
			if (shift) {
				for (uint8_t c = 0U; c < fh->channel_count; c++) {
					int32_t *blk = inst->blkbuf[c];
					for (uint16_t i = 0U; i < blk_n; i++) {
						blk[i] = blk[i] * (1 << shift);
					}
				}
			}

			/* We're done decoding this frame! Notify the outer loop! */
			inst->blk_cur = 0U; /* Reset the read cursor */
			inst->chan_cur = 0U;
			inst->state = FLAC_DECODED_FRAME;
			break;
		}
		default:
			inst->state = FLAC_ERR;
			break;
	}
	return true;
}

static bool _fx_flac_process_decoded_frame(fx_flac_t *inst, int32_t *out,
                                           uint32_t *out_len)
{
	/* Fetch the current stream and frame info. */
	const fx_flac_frame_header_t *fh = inst->frame_header;

	/* Fetch channel count and number of samples left to write */
	const uint8_t cc = fh->channel_count;
	uint32_t n_smpls_rem =
	    (fh->block_size - inst->blk_cur - 1U) * cc + (cc - inst->chan_cur);

	/* Truncate to the actually available space. */
	if (n_smpls_rem > *out_len) {
		n_smpls_rem = *out_len;
	}

	/* Interlace the decoded samples in the output array */
	uint32_t tar = 0U; /* Number of samples written. */
	while (tar < n_smpls_rem) {
		/* Write to the output buffer */
		out[tar] = inst->blkbuf[inst->chan_cur][inst->blk_cur];

		/* Advance the read and write cursors */
		inst->chan_cur++;
		if (inst->chan_cur == cc) {
			inst->chan_cur = 0U;
			inst->blk_cur++;
		}
		tar++;
	}

	/* Inform the caller about the number of samples written */
	*out_len = tar;

	/* We're done with this frame! */
	if (inst->blk_cur == fh->block_size) {
		inst->state = FLAC_END_OF_FRAME;
		return true;
	}

	/* Since we're here, we need more space in the output array. */
	return false;
}

/******************************************************************************
 * PUBLIC API                                                                 *
 ******************************************************************************/

uint32_t fx_flac_size(uint32_t max_block_size, uint8_t max_channels)
{
	/* Calculate the size of the fixed-size structures */
	uint32_t size;
	bool ok = _fx_flac_check_params(max_block_size, max_channels) &&
	          fx_mem_init_size(&size) &&
	          fx_mem_update_size(&size, sizeof(fx_flac_t)) &&
	          fx_mem_update_size(&size, sizeof(fx_flac_metadata_t)) &&
	          fx_mem_update_size(&size, sizeof(fx_flac_streaminfo_t)) &&
	          fx_mem_update_size(&size, sizeof(fx_flac_frame_header_t)) &&
	          fx_mem_update_size(&size, sizeof(fx_flac_subframe_header_t)) &&
	          fx_mem_update_size(&size, sizeof(int32_t) * 32U);

	/* Calculate the size of the structures depending on the given parameters.
	 */
	for (uint8_t i = 0; i < max_channels; i++) {
		ok = ok && fx_mem_update_size(&size, sizeof(int32_t) * max_block_size);
	}
	return ok ? size : 0;
}

fx_flac_t *fx_flac_init(void *mem, uint16_t max_block_size,
                        uint8_t max_channels)
{
	/* Make sure the parameters are valid. */
	if (!_fx_flac_check_params(max_block_size, max_channels)) {
		return NULL;
	}

	/* Abort if mem is NULL to allow passing malloc as a direct argument to this
	   code. Furthermore, store the original "mem" pointer and return it later
	   so the calling code is safe to pass the returned pointer to free. */
	fx_flac_t *inst_unaligned = (fx_flac_t *)mem;
	if (mem) {
		/* Fetch the base address of the flac_t address. */
		fx_flac_t *inst = (fx_flac_t *)fx_mem_align(&mem, sizeof(fx_flac_t));

		/* Copy the given parameters */
		inst->max_block_size = max_block_size;
		inst->max_channels = max_channels;

		/* Fetch the base addresses of the internal pointers. */
		inst->metadata = (fx_flac_metadata_t *)fx_mem_align(
		    &mem, sizeof(fx_flac_metadata_t));
		inst->streaminfo = (fx_flac_streaminfo_t *)fx_mem_align(
		    &mem, sizeof(fx_flac_streaminfo_t));
		inst->frame_header = (fx_flac_frame_header_t *)fx_mem_align(
		    &mem, sizeof(fx_flac_frame_header_t));
		inst->subframe_header = (fx_flac_subframe_header_t *)fx_mem_align(
		    &mem, sizeof(fx_flac_subframe_header_t));
		inst->qbuf = (int32_t *)fx_mem_align(&mem, sizeof(int32_t) * 32U);

		/* Compute the addresses of the per-channel buffers */
		for (uint8_t i = 0; i < FLAC_MAX_CHANNEL_COUNT; i++) {
			inst->blkbuf[i] = NULL;
		}
		for (uint8_t i = 0; i < max_channels; i++) {
			inst->blkbuf[i] =
			    (int32_t *)fx_mem_align(&mem, sizeof(int32_t) * max_block_size);
		}

		/* Reset the instance, i.e. zero most/all fields. */
		fx_flac_reset(inst);
	}
	/* Return the original pointer. */
	return inst_unaligned;
}

void fx_flac_reset(fx_flac_t *inst)
{
	inst = (fx_flac_t *)FX_ALIGN_ADDR(inst);

	/* Initialize the bitstream reader */
	fx_bitstream_init(&inst->bitstream);

	/* Initialize the current metadata block header */
	FX_MEM_ZERO_ALIGNED(inst->metadata);
	inst->metadata->type = META_TYPE_INVALID;

	/* Initialize the streaminfo structure */
	FX_MEM_ZERO_ALIGNED(inst->streaminfo);

	/* Initialize the frame_header structure */
	FX_MEM_ZERO_ALIGNED(inst->frame_header);

	/* Initialize the subframe_header structure */
	FX_MEM_ZERO_ALIGNED(inst->subframe_header);

	/* Initialize private member variables */
	inst->state = FLAC_INIT;
	inst->priv_state = FLAC_SYNC_INIT;
	inst->n_bytes_rem = 0U;
	inst->crc8 = 0U;
	inst->coef_cur = 0U;
	inst->partition_cur = 0U;
	inst->partition_sample = 0U;
	inst->rice_unary_counter = 0U;
	inst->chan_cur = 0U;
	inst->blk_cur = 0U;
}

fx_flac_state_t fx_flac_get_state(const fx_flac_t *inst)
{
	return ((const fx_flac_t *)FX_ALIGN_ADDR(inst))->state;
}

int64_t fx_flac_get_streaminfo(fx_flac_t const *inst,
                               fx_flac_streaminfo_key_t key)
{
	inst = (fx_flac_t *)FX_ALIGN_ADDR(inst);
	switch (key) {
		case FLAC_KEY_MIN_BLOCK_SIZE:
			return inst->streaminfo->min_block_size;
		case FLAC_KEY_MAX_BLOCK_SIZE:
			return inst->streaminfo->max_block_size;
		case FLAC_KEY_MIN_FRAME_SIZE:
			return inst->streaminfo->min_frame_size;
		case FLAC_KEY_MAX_FRAME_SIZE:
			return inst->streaminfo->max_frame_size;
		case FLAC_KEY_SAMPLE_RATE:
			return inst->streaminfo->sample_rate;
		case FLAC_KEY_N_CHANNELS:
			return inst->streaminfo->n_channels;
		case FLAC_KEY_SAMPLE_SIZE:
			return inst->streaminfo->sample_size;
		case FLAC_KEY_N_SAMPLES:
			return inst->streaminfo->n_samples;
		case FLAC_KEY_MD5_SUM_0 ... FLAC_KEY_MD5_SUM_F:
			return inst->streaminfo->md5_sum[key - FLAC_KEY_MD5_SUM_0];
		default:
			return FLAC_INVALID_METADATA_KEY;
	}
}

fx_flac_state_t fx_flac_process(fx_flac_t *inst, const uint8_t *in,
                                uint32_t *in_len, int32_t *out,
                                uint32_t *out_len)
{
	inst = (fx_flac_t *)FX_ALIGN_ADDR(inst);

	/* Set the current bytestream source to the provided input buffer */
	fx_bitstream_t *bs = &inst->bitstream; /* Alias */
	fx_bitstream_set_source(bs, in, *in_len);

	/* Advance the statemachine */
	bool done = false;
	uint32_t out_len_ = 0U;
	fx_flac_state_t old_state = inst->state;
	while (!done) {
		/* Abort once we've reached an error state. */
		if (inst->state == FLAC_ERR) {
			done = true;
			continue; /* Panic, all hope is lost! */
		}

		/* Automatically return once the state transitions to a relevant state,
		   even if there is still data to read. */
		if (old_state != inst->state) {
			old_state = inst->state;
			switch (inst->state) {
				case FLAC_END_OF_METADATA:
				case FLAC_END_OF_FRAME:
					done = true; /* Good point to return to the caller */
					continue;
				default:
					break;
			}
		}

		/* Main state machine. Dispatch calls to the corresponding state
		   handlers. These will returns false in case there is no more data
		   to read/space to write to. */
		switch (inst->state) {
			case FLAC_INIT:
				done = !_fx_flac_process_init(inst);
				break;
			case FLAC_IN_METADATA:
				done = !_fx_flac_process_in_metadata(inst);
				break;
			case FLAC_END_OF_METADATA:
			case FLAC_END_OF_FRAME:
				inst->state = FLAC_SEARCH_FRAME;
				inst->priv_state = FLAC_FRAME_SYNC;
				break;
			case FLAC_SEARCH_FRAME:
				done = !_fx_flac_process_search_frame(inst);
				break;
			case FLAC_IN_FRAME:
				done = !_fx_flac_process_in_frame(inst);
				break;
			case FLAC_DECODED_FRAME:
				/* If no output buffers are given, just discard the data. */
				if (!out || !out_len) {
					inst->state = FLAC_END_OF_FRAME;
					break;
				}
				out_len_ = *out_len;
				done = !_fx_flac_process_decoded_frame(inst, out, &out_len_);
				break;
			default:
				inst->state = FLAC_ERR; /* Internal error */
				break;
		}
	}

	/* Write the number of bytes we read from the input stream to in_len, the
	   caller must not provide these bytes again. Also write the number of
	   samples we wrote to the output buffer. */
	if (out_len) {
		*out_len = out_len_;
	}
	*in_len = bs->src - in;

	/* Return the current state */
	return inst->state;
}

