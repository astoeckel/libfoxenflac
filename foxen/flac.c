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

#include <foxen/bitstream.h>
#include <foxen/flac.h>
#include <foxen/mem.h>

/******************************************************************************
 * DATATYPES                                                                  *
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
	uint8_t bits_per_sample;
	uint64_t n_samples;
	uint8_t md5_sum[16];
};

typedef struct fx_flac_streaminfo fx_flac_streaminfo_t;

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
	FLAC_METADATA_SINFO
};

typedef enum fx_flac_private_state fx_flac_private_state_t;

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
	 * Flag indicating whether the current metadata block is the last metadata
	 * block.
	 */
	fx_flac_metadata_t *metadata;

	/**
	 * Structure holding the current stream metadata.
	 */
	fx_flac_streaminfo_t *streaminfo;
};

/******************************************************************************
 * PRIVATE STATE MACHINE FUNCTIONS                                            *
 ******************************************************************************/

#define ENUSURE_BITS(n)                                \
	if (!fx_bitstream_can_read(&inst->bitstream, n)) { \
		return false; /* Need more data */             \
	}

#define READ_BITS(n)                                         \
	(tmp_ = fx_bitstream_try_read_msb(&inst->bitstream, n)); \
	if (tmp_ < 0) {                                          \
		return false; /* Need more data */                   \
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
			ENUSURE_BITS(32U);
			inst->metadata->is_last = READ_BITS(1U);
			inst->metadata->type = READ_BITS(7U);
			if (inst->metadata->type == META_TYPE_INVALID) {
				inst->state = FLAC_ERR;
				break;
			}
			inst->metadata->length = inst->n_bytes_rem = READ_BITS(24);
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
					ENUSURE_BITS(28U);
					inst->streaminfo->sample_rate = READ_BITS(20U);
					inst->streaminfo->n_channels = 1U + READ_BITS(3U);
					inst->streaminfo->bits_per_sample = 1U + READ_BITS(5U);
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

/******************************************************************************
 * PUBLIC API                                                                 *
 ******************************************************************************/

uint32_t fx_flac_size(void)
{
	uint32_t size;
	bool ok = fx_mem_init_size(&size) &&
	          fx_mem_update_size(&size, sizeof(fx_flac_t)) &&
	          fx_mem_update_size(&size, sizeof(fx_flac_metadata_t)) &&
	          fx_mem_update_size(&size, sizeof(fx_flac_streaminfo_t));
	return ok ? size : 0;
}

fx_flac_t *fx_flac_init(void *mem)
{
	if (!mem) {
		return NULL;
	}

	fx_flac_t *inst = fx_mem_align(&mem, sizeof(fx_flac_t));
	inst->metadata = fx_mem_align(&mem, sizeof(fx_flac_metadata_t));
	inst->streaminfo = fx_mem_align(&mem, sizeof(fx_flac_streaminfo_t));
	fx_flac_reset(inst);
	return inst;
}

void fx_flac_reset(fx_flac_t *inst)
{
	inst = (fx_flac_t *)FX_ALIGN_ADDR(inst);

	/* Initialize the bitstream reader */
	fx_bitstream_init(&inst->bitstream);

	/* Initialize the current metadata block header */
	FX_MEM_ZERO_ALIGNED(inst->metadata);
	inst->metadata->type = META_TYPE_INVALID;

	/* Initialize the stream info structure */
	FX_MEM_ZERO_ALIGNED(inst->streaminfo);

	/* Initialize private member variables */
	inst->state = FLAC_INIT;
	inst->priv_state = FLAC_SYNC_INIT;
	inst->n_bytes_rem = 0U;
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
		case FLAC_BITS_PER_SAMPLE:
			return inst->streaminfo->bits_per_sample;
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

