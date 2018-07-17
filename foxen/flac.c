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

#include <stdio.h> /* XXX */

#include <foxen/bitstream.h>
#include <foxen/flac.h>
#include <foxen/mem.h>

/******************************************************************************
 * DATATYPES                                                                  *
 ******************************************************************************/

/******************************************************************************
 * Enums defined in the FLAC format specifiction                              *
 ******************************************************************************/

/**
 * Possible metadata block types.
 */
enum fx_flac_metadata_type {
	META_TYPE_STREAMINFO = 0,
	META_TYPE_PADDING = 1,
	META_TYPE_APPLICATION = 2,
	META_TYPE_SEEKTABLE = 3,
	META_TYPE_VORBIS_COMMENT = 4,
	META_TYPE_CUESHEET = 5,
	META_TYPE_PICTURE = 6,
	META_TYPE_INVALID = 127
};

/**
 * Typedef for the block type enum.
 */
typedef enum fx_flac_metadata_type fx_flac_metadata_type_t;

enum fx_flac_blocking_strategy { BLK_FIXED = 0, BLK_VARIABLE = 1 };

typedef enum fx_flac_blocking_strategy fx_flac_blocking_strategy_t;

enum fx_flac_channel_assignment {
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
};

typedef enum fx_flac_channel_assignment fx_flac_channel_assignment_t;

enum fx_flac_block_size {
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
};

typedef enum fx_flac_block_size fx_flac_block_size_t;

enum fx_flac_sample_rate {
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
};

typedef enum fx_flac_sample_rate fx_flac_sample_rate_t;

enum fx_flac_sample_size {
	SS_STREAMINFO = 0,
	SS_8BIT = 1,
	SS_12BIT = 2,
	SS_RESERVED_1 = 3,
	SS_16BIT = 4,
	SS_20BIT = 5,
	SS_24BIT = 6,
	SS_RESERVED_2 = 7
};

typedef enum fx_flac_sample_size fx_flac_sample_size_t;

/******************************************************************************
 * Structs defined in the flac format specification                           *
 ******************************************************************************/

/**
 * Struc holding all the information stored in an individual block header.
 */
struct fx_flac_metadata {
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
};

/**
 * Typedef for the fx_flac_metadata_header type.
 */
typedef struct fx_flac_metadata fx_flac_metadata_t;

/**
 * Data stored in the STREAMINFO header.
 */
struct fx_flac_streaminfo {
	uint16_t min_block_size;
	uint16_t max_block_size;
	uint32_t min_frame_size;
	uint32_t max_frame_size;
	uint32_t sample_rate;
	uint8_t n_channels;
	uint8_t sample_size;
	uint64_t n_samples;
	uint8_t md5_sum[16];
};

typedef struct fx_flac_streaminfo fx_flac_streaminfo_t;

/**
 * Frame header prepended to each FLAC audio block.
 */
struct fx_flac_frameheader {
	fx_flac_blocking_strategy_t blocking_strategy;
	fx_flac_block_size_t block_size_enum;
	fx_flac_sample_rate_t sample_rate_enum;
	fx_flac_channel_assignment_t channel_assignment;
	fx_flac_sample_size_t sample_size_enum;
	uint32_t block_size;
	uint32_t sample_rate;
	uint8_t sample_size;
	uint64_t sync_info;
	uint8_t crc8;
};

typedef struct fx_flac_frameheader fx_flac_frameheader_t;

/******************************************************************************
 * Internal state machine enums                                               *
 ******************************************************************************/

/**
 * More fine-grained state descriptor used in the internal state machine.
 */
enum fx_flac_private_state {
	FLAC_SYNC_INIT,
	FLAC_SYNC_F,
	FLAC_SYNC_L,
	FLAC_SYNC_A,
	FLAC_METADATA_HEADER,
	FLAC_METADATA_SKIP,
	FLAC_METADATA_SINFO,
	FLAC_FRAME_SYNC,
	FLAC_FRAME_HEADER,
	FLAC_FRAME_HEADER_SYNC_INFO,
	FLAC_FRAME_HEADER_AUX,
	FLAC_FRAME_HEADER_CRC
};

typedef enum fx_flac_private_state fx_flac_private_state_t;

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
	 * Variable holding the checksum computed when reading the frameheader.
	 */
	uint8_t crc8;

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
	fx_flac_frameheader_t *frameheader;
};

/******************************************************************************
 * PRIVATE CODE                                                               *
 ******************************************************************************/

/******************************************************************************
 * FLAC enum decoders                                                         *
 ******************************************************************************/

static bool _fx_flac_decode_block_size(fx_flac_block_size_t block_size_enum,
                                       uint32_t *block_size)
{
	switch (block_size_enum) {
		case BLK_SIZE_RESERVED:
			return false;
		case BLK_SIZE_192:
			*block_size = 192U;
			return true;
		case BLK_SIZE_576:
			*block_size = 576U;
			return true;
		case BLK_SIZE_1152:
			*block_size = 1152U;
			return true;
		case BLK_SIZE_2304:
			*block_size = 2304U;
			return true;
		case BLK_SIZE_4608:
			*block_size = 4608U;
			return true;
		case BLK_SIZE_256 ... BLK_SIZE_32768:
			*block_size = (1U << (int)block_size_enum);
			return true;
		default:
			return true; /* Will read later */
	}
}

static bool _fx_flac_decode_sample_rate(fx_flac_sample_rate_t sample_rate_enum,
                                        uint32_t *sample_rate)
{
	switch (sample_rate_enum) {
		case FS_STREAMINFO:
			return true; /* Already set to the STREAMINFO value */
		case FS_88_2KHZ:
			*sample_rate = 88200U;
			return true;
		case FS_176_4KHZ:
			*sample_rate = 176400U;
			return true;
		case FS_192KHZ:
			*sample_rate = 192000U;
			return true;
		case FS_8KHZ:
			*sample_rate = 8000U;
			return true;
		case FS_16KHZ:
			*sample_rate = 16000U;
			return true;
		case FS_22_05KHZ:
			*sample_rate = 22050U;
			return true;
		case FS_24KHZ:
			*sample_rate = 24000U;
			return true;
		case FS_32KHZ:
			*sample_rate = 32000U;
			return true;
		case FS_44_1KHZ:
			*sample_rate = 44100U;
			return true;
		case FS_48KHZ:
			*sample_rate = 48000U;
			return true;
		case FS_96KHZ:
			*sample_rate = 96000U;
			return true;
		case FS_INVALID:
			return false;
		default:
			return true; /* Will read later */
	}
}

static bool _fx_flac_decode_sample_size(fx_flac_sample_size_t sample_size_enum,
                                        uint8_t *sample_size)
{
	switch (sample_size_enum) {
		case SS_STREAMINFO:
			return true; /* Already set to the STREAMINFO value */
		case SS_8BIT:
			*sample_size = 8U;
			return true;
		case SS_12BIT:
			*sample_size = 12U;
			return true;
		case SS_16BIT:
			*sample_size = 16U;
			return true;
		case SS_20BIT:
			*sample_size = 20U;
			return true;
		case SS_24BIT:
			*sample_size = 24U;
			return true;
		default:
		case SS_RESERVED_1:
		case SS_RESERVED_2:
			return false;
	}
}

/******************************************************************************
 * Utility functions and macros                                               *
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

#define READ_BITS_CRC8(n)                                \
	(tmp_ = fx_bitstream_read_msb(&inst->bitstream, n)); \
	if (tmp_ < 0) {                                      \
		return false; /* Need more data */               \
	}                                                    \
	_fx_flac_crc8(tmp_, n, &inst->crc8);

#define READ_BITS_FAST_CRC8(n)                           \
	(tmp_ = fx_bitstream_read_msb(&inst->bitstream, n)); \
	_fx_flac_crc8(tmp_, n, &inst->crc8);

static void _fx_flac_crc8(uint64_t in, uint8_t n_bits, uint8_t *crc8)
{
	for (uint8_t i = n_bits; i > 0U; i--) {
		if (in & (1U << (i - 1U))) {
			*crc8 = (*crc8) ^ 0x80;
		}
		if ((*crc8) & 0x80) {
			*crc8 = ((*crc8) << 1) ^ 0x07;
		}
		else {
			*crc8 = (*crc8) << 1;
		}
	}
}

static bool _fx_flac_reader_utf8_coded_int(fx_flac_t *inst, uint8_t max_n,
                                           uint64_t *tar)
{
	int64_t tmp_; /* Used by the READ_BITS macro */

	ENSURE_BITS(max_n * 8U);
	/* Read the first byte */
	uint8_t v = READ_BITS_FAST_CRC8(8U);

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
		v = READ_BITS_FAST_CRC8(8U);
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
			inst->state = FLAC_ERR;
			break;
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
				inst->state = FLAC_ERR;
				break;
			}
			inst->metadata->length = inst->n_bytes_rem = READ_BITS_FAST(24);
			if (inst->metadata->type == META_TYPE_STREAMINFO) {
				inst->priv_state = FLAC_METADATA_SINFO;
				/* The stream info header must be exactly 33 bytes long */
				if (inst->metadata->length != 34U) {
					inst->state = FLAC_ERR;
					break;
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
					inst->state = FLAC_ERR;
					break;
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
			inst->state = FLAC_ERR; /* Internal error */
			break;
	}
	return true;
}

static bool _fx_flac_process_search_frame(fx_flac_t *inst)
{
	int64_t tmp_; /* Used by the READ_BITS macro */
	fx_flac_frameheader_t *fh = inst->frameheader;
	fx_flac_streaminfo_t *si = inst->streaminfo;
	switch (inst->priv_state) {
		case FLAC_FRAME_SYNC:
			ENSURE_BITS(15U);
			uint16_t sync_code = PEEK_BITS(15U);
			if (sync_code != 0x7FFCU) {
				READ_BITS(8U); /* Next byte (assume frames are byte aligned). */
				return true;
			}
			else {
				inst->crc8 = 0U; /* Start computing the header checksum */
				inst->priv_state = FLAC_FRAME_HEADER;
				READ_BITS_FAST_CRC8(15U);
			}
			break;
		case FLAC_FRAME_HEADER:
			ENSURE_BITS(17U);
			/* Read the frame header bits */
			fh->blocking_strategy =
			    (fx_flac_blocking_strategy_t)READ_BITS_FAST_CRC8(1U);
			fh->block_size_enum = (fx_flac_block_size_t)READ_BITS_FAST_CRC8(4U);
			fh->sample_rate_enum =
			    (fx_flac_sample_rate_t)READ_BITS_FAST_CRC8(4U);
			fh->channel_assignment =
			    (fx_flac_channel_assignment_t)READ_BITS_FAST_CRC8(4U);
			fh->sample_size_enum =
			    (fx_flac_sample_size_t)READ_BITS_FAST_CRC8(3U);
			READ_BITS_FAST_CRC8(1U);
			if (tmp_ != 0U || fh->channel_assignment > MID_SIDE_STEREO) {
				inst->priv_state = FLAC_FRAME_SYNC; /* Invalid header */
				return true;
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
			                                 &fh->sample_size)) {
				inst->priv_state = FLAC_FRAME_SYNC; /* Got invalid value */
				break;
			}
			inst->priv_state = FLAC_FRAME_HEADER_SYNC_INFO;
			break;
		case FLAC_FRAME_HEADER_SYNC_INFO:
			if (!_fx_flac_reader_utf8_coded_int(
			        inst, (fh->blocking_strategy == BLK_VARIABLE) ? 7 : 6,
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
					fh->block_size = 1U + READ_BITS_FAST_CRC8(8U);
					break;
				case BLK_SIZE_READ_16BIT:
					fh->block_size = READ_BITS_FAST_CRC8(16U);
					break;
				default:
					break;
			}
			switch (fh->sample_rate_enum) {
				case FS_READ_8BIT_KHZ:
					fh->sample_rate = 1000UL * READ_BITS_FAST_CRC8(8U);
					break;
				case FS_READ_16BIT_HZ:
					fh->sample_rate = READ_BITS_FAST_CRC8(16U);
					break;
				case FS_READ_16BIT_DHZ:
					fh->sample_rate = 10UL * READ_BITS_FAST_CRC8(16U);
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
			fh->crc8 = READ_BITS(8U);
			if (fh->crc8 != inst->crc8) {
				inst->priv_state = FLAC_FRAME_SYNC;
				return true;
			}
			fprintf(stderr, "------------------------------------\n");
			fprintf(stderr, "Found frame, sync info %ld\n", fh->sync_info);
			fprintf(stderr, "Variable block size %d\n", fh->blocking_strategy);
			fprintf(stderr, "Sample rate %d\n", fh->sample_rate);
			fprintf(stderr, "Bit depth %d\n", fh->sample_size);
			fprintf(stderr, "Block size %d\n", fh->block_size);
			fprintf(stderr, "Channel assignment %d\n", fh->channel_assignment);
			fprintf(stderr, "CRC8 %d %d\n", fh->crc8, inst->crc8);
			inst->priv_state = FLAC_FRAME_SYNC;
			break;
		default:
			inst->state = FLAC_ERR;
			break;
	}
	return true;
}

/******************************************************************************
 * PUBLIC API                                                                 *
 ******************************************************************************/

uint32_t fx_flac_size(void)
{
	uint32_t size;
	bool ok = fx_mem_init_size(&size) &&
	          fx_mem_update_size(&size, sizeof(fx_flac_t)) &&
	          fx_mem_update_size(&size, sizeof(fx_flac_metadata_t)) &&
	          fx_mem_update_size(&size, sizeof(fx_flac_streaminfo_t)) &&
	          fx_mem_update_size(&size, sizeof(fx_flac_frameheader_t));
	return ok ? size : 0;
}

fx_flac_t *fx_flac_init(void *mem)
{
	/* Abort if mem is NULL to allow passing malloc as a direct argument to this
	   code. Furthermore, store the original "mem" pointer and return it later
	   so the calling code is safe to pass the returned pointer to free. */
	fx_flac_t *inst_unaligned = (fx_flac_t *)mem;
	if (mem) {
		/* Fetch the base address of the flac_t address. */
		fx_flac_t *inst = (fx_flac_t *)fx_mem_align(&mem, sizeof(fx_flac_t));

		/* Fetch the base addresses of the internal pointers. */
		inst->metadata = (fx_flac_metadata_t *)fx_mem_align(
		    &mem, sizeof(fx_flac_metadata_t));
		inst->streaminfo = (fx_flac_streaminfo_t *)fx_mem_align(
		    &mem, sizeof(fx_flac_streaminfo_t));
		inst->frameheader = (fx_flac_frameheader_t *)fx_mem_align(
		    &mem, sizeof(fx_flac_frameheader_t));

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

	/* Initialize the frameheader structure */
	FX_MEM_ZERO_ALIGNED(inst->frameheader);

	/* Initialize private member variables */
	inst->state = FLAC_INIT;
	inst->priv_state = FLAC_SYNC_INIT;
	inst->n_bytes_rem = 0U;
	inst->crc8 = 0U;
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
		case FLAC_MIN_BLOCK_SIZE:
			return inst->streaminfo->min_block_size;
		case FLAC_MAX_BLOCK_SIZE:
			return inst->streaminfo->max_block_size;
		case FLAC_MIN_FRAME_SIZE:
			return inst->streaminfo->min_frame_size;
		case FLAC_MAX_FRAME_SIZE:
			return inst->streaminfo->max_frame_size;
		case FLAC_SAMPLE_RATE:
			return inst->streaminfo->sample_rate;
		case FLAC_N_CHANNELS:
			return inst->streaminfo->n_channels;
		case FLAC_SAMPLE_SIZE:
			return inst->streaminfo->sample_size;
		case FLAC_N_SAMPLES:
			return inst->streaminfo->n_samples;
		case FLAC_MD5_SUM_0 ... FLAC_MD5_SUM_F:
			return inst->streaminfo->md5_sum[key - FLAC_MD5_SUM_0];
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
				case FLAC_END_OF_STREAM:
					done = true; /* Good point to return to the caller */
					continue;
				default:
					break;
			}
		}

		switch (inst->state) {
			case FLAC_INIT:
				done = !_fx_flac_process_init(inst);
				break;
			case FLAC_IN_METADATA:
				done = !_fx_flac_process_in_metadata(inst);
				break;
			case FLAC_END_OF_METADATA:
				inst->state = FLAC_SEARCH_FRAME;
				inst->priv_state = FLAC_FRAME_SYNC;
				break;
			case FLAC_SEARCH_FRAME:
				done = !_fx_flac_process_search_frame(inst);
				break;
			default:
				inst->state = FLAC_ERR; /* Internal error */
				break;
		}
	}

	/* Write the number of bytes we read from the input stream to in_len, the
	 * caller must not provide these bytes again. */
	*in_len = bs->src - in;

	/* Return the current state */
	return inst->state;
}

