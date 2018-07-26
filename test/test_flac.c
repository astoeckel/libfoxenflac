/*
 *  libstanchion -- Introduces musical acts at the EOLIAN
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

#include <stdbool.h>
#include <stdlib.h>

#include <foxen/flac.h>
#include <foxen/unittest.h>

/******************************************************************************
 * Unit tests                                                                 *
 ******************************************************************************/

#include "data_fixed_1.h"
#include "data_fixed_2.h"
#include "data_header.h"

void check_flac_metadata_short(fx_flac_t *inst)
{
	EXPECT_EQ(4096U, fx_flac_get_streaminfo(inst, FLAC_KEY_MIN_BLOCK_SIZE));
	EXPECT_EQ(4096U, fx_flac_get_streaminfo(inst, FLAC_KEY_MAX_BLOCK_SIZE));
	EXPECT_EQ(16U, fx_flac_get_streaminfo(inst, FLAC_KEY_MIN_FRAME_SIZE));
	EXPECT_EQ(12695U, fx_flac_get_streaminfo(inst, FLAC_KEY_MAX_FRAME_SIZE));
	EXPECT_EQ(44100U, fx_flac_get_streaminfo(inst, FLAC_KEY_SAMPLE_RATE));
	EXPECT_EQ(2U, fx_flac_get_streaminfo(inst, FLAC_KEY_N_CHANNELS));
	EXPECT_EQ(16U, fx_flac_get_streaminfo(inst, FLAC_KEY_SAMPLE_SIZE));
	EXPECT_EQ(9062550U, fx_flac_get_streaminfo(inst, FLAC_KEY_N_SAMPLES));
	EXPECT_EQ(0x45, fx_flac_get_streaminfo(inst, FLAC_KEY_MD5_SUM_0));
	EXPECT_EQ(0x61, fx_flac_get_streaminfo(inst, FLAC_KEY_MD5_SUM_1));
	EXPECT_EQ(0x31, fx_flac_get_streaminfo(inst, FLAC_KEY_MD5_SUM_2));
	EXPECT_EQ(0x02, fx_flac_get_streaminfo(inst, FLAC_KEY_MD5_SUM_3));
	EXPECT_EQ(0x8B, fx_flac_get_streaminfo(inst, FLAC_KEY_MD5_SUM_4));
	EXPECT_EQ(0xFB, fx_flac_get_streaminfo(inst, FLAC_KEY_MD5_SUM_5));
	EXPECT_EQ(0x21, fx_flac_get_streaminfo(inst, FLAC_KEY_MD5_SUM_6));
	EXPECT_EQ(0xE5, fx_flac_get_streaminfo(inst, FLAC_KEY_MD5_SUM_7));
	EXPECT_EQ(0x5F, fx_flac_get_streaminfo(inst, FLAC_KEY_MD5_SUM_8));
	EXPECT_EQ(0xFB, fx_flac_get_streaminfo(inst, FLAC_KEY_MD5_SUM_9));
	EXPECT_EQ(0x6E, fx_flac_get_streaminfo(inst, FLAC_KEY_MD5_SUM_A));
	EXPECT_EQ(0xDF, fx_flac_get_streaminfo(inst, FLAC_KEY_MD5_SUM_B));
	EXPECT_EQ(0x48, fx_flac_get_streaminfo(inst, FLAC_KEY_MD5_SUM_C));
	EXPECT_EQ(0xCE, fx_flac_get_streaminfo(inst, FLAC_KEY_MD5_SUM_D));
	EXPECT_EQ(0x9F, fx_flac_get_streaminfo(inst, FLAC_KEY_MD5_SUM_E));
	EXPECT_EQ(0xAE, fx_flac_get_streaminfo(inst, FLAC_KEY_MD5_SUM_F));
}

void check_flac_metadata_long(fx_flac_t *inst)
{
	EXPECT_EQ(4608U, fx_flac_get_streaminfo(inst, FLAC_KEY_MIN_BLOCK_SIZE));
	EXPECT_EQ(4608U, fx_flac_get_streaminfo(inst, FLAC_KEY_MAX_BLOCK_SIZE));
	EXPECT_EQ(0U, fx_flac_get_streaminfo(inst, FLAC_KEY_MIN_FRAME_SIZE));
	EXPECT_EQ(0U, fx_flac_get_streaminfo(inst, FLAC_KEY_MAX_FRAME_SIZE));
	EXPECT_EQ(44100U, fx_flac_get_streaminfo(inst, FLAC_KEY_SAMPLE_RATE));
	EXPECT_EQ(2U, fx_flac_get_streaminfo(inst, FLAC_KEY_N_CHANNELS));
	EXPECT_EQ(16U, fx_flac_get_streaminfo(inst, FLAC_KEY_SAMPLE_SIZE));
	EXPECT_EQ(13935600U, fx_flac_get_streaminfo(inst, FLAC_KEY_N_SAMPLES));
	for (int key = FLAC_KEY_MD5_SUM_0; key <= FLAC_KEY_MD5_SUM_F; key++) {
		EXPECT_EQ(0x00,
		          fx_flac_get_streaminfo(inst, (fx_flac_streaminfo_key_t)key));
	}
}

void test_flac_metadata_single()
{
	fx_flac_t *inst = FX_FLAC_ALLOC_DEFAULT();
	ASSERT_NE(NULL, inst);
	uint32_t len = sizeof(FLAC_SHORT_HEADER);
	EXPECT_EQ(FLAC_END_OF_METADATA,
	          fx_flac_process(inst, FLAC_SHORT_HEADER, &len, NULL, NULL));
	EXPECT_EQ(sizeof(FLAC_SHORT_HEADER), len);
	check_flac_metadata_short(inst);
	free(inst);
}

void test_flac_metadata_multiple()
{
	fx_flac_t *inst = FX_FLAC_ALLOC_DEFAULT();
	ASSERT_NE(NULL, inst);
	uint32_t len = sizeof(FLAC_LONG_HEADER);
	EXPECT_EQ(FLAC_END_OF_METADATA,
	          fx_flac_process(inst, FLAC_LONG_HEADER, &len, NULL, NULL));
	EXPECT_EQ(sizeof(FLAC_LONG_HEADER), len);
	check_flac_metadata_long(inst);
	free(inst);
}

void test_flac_metadata_single_one_byte()
{
	fx_flac_t *inst = FX_FLAC_ALLOC_DEFAULT();
	ASSERT_NE(NULL, inst);
	for (uint32_t i = 0; i < sizeof(FLAC_SHORT_HEADER); i++) {
		uint32_t len = 1U;
		fx_flac_process(inst, &FLAC_SHORT_HEADER[i], &len, NULL, NULL);
		ASSERT_EQ(1U, len);
	}
	EXPECT_EQ(FLAC_END_OF_METADATA, fx_flac_get_state(inst));
	check_flac_metadata_short(inst);
	free(inst);
}

void test_flac_metadata_multiple_one_byte()
{
	fx_flac_t *inst = FX_FLAC_ALLOC_DEFAULT();
	ASSERT_NE(NULL, inst);
	for (uint32_t i = 0; i < sizeof(FLAC_LONG_HEADER); i++) {
		uint32_t len = 1U;
		fx_flac_process(inst, &FLAC_LONG_HEADER[i], &len, NULL, NULL);
		ASSERT_EQ(1U, len);
	}
	EXPECT_EQ(FLAC_END_OF_METADATA, fx_flac_get_state(inst));
	check_flac_metadata_long(inst);
	free(inst);
}

void generic_test_flac_single_frame_ex(const uint8_t *in_,
                                       const uint32_t in_len_,
                                       const int32_t *out_,
                                       const uint32_t out_len_,
                                       bool samplewise)
{
	/* For a single frame we expect the following state transitions if the data
	   is read bytewise. */
	const fx_flac_state_t expected_states[7][2] = {
	    {FLAC_INIT, FLAC_IN_METADATA},
	    {FLAC_IN_METADATA, FLAC_END_OF_METADATA},
	    {FLAC_END_OF_METADATA, FLAC_SEARCH_FRAME},
	    {FLAC_SEARCH_FRAME, FLAC_IN_FRAME},
	    {FLAC_IN_FRAME, FLAC_DECODED_FRAME},
	    {FLAC_DECODED_FRAME, FLAC_END_OF_FRAME},
	    {FLAC_END_OF_FRAME, FLAC_SEARCH_FRAME}};
	int state_transition_idx = 0;

	/* Create the FLAC instance and read the current state */
	fx_flac_t *inst = FX_FLAC_ALLOC_DEFAULT();
	ASSERT_NE(NULL, inst);
	fx_flac_state_t last_state = fx_flac_get_state(inst);

	int32_t out[512U];
	uint8_t const *in = in_;
	uint32_t out_ptr = 0U;
	int bps = 1;
	while (true) {
		/* Advance the FLAC state machine */
		uint32_t in_rem = in_len_ - (in - in_);
		uint32_t in_len = (in_rem > 0U) ? 1U : 0U;
		uint32_t out_len = samplewise ? 1U : 512U;
		fx_flac_state_t state =
		    fx_flac_process(inst, in, &in_len, out, &out_len);

		/* Check state transitions */
		if (state != last_state) {
			if (state == FLAC_END_OF_METADATA) {
				bps = fx_flac_get_streaminfo(inst, FLAC_KEY_SAMPLE_SIZE);
			}
			EXPECT_GT(7U, state_transition_idx);
			EXPECT_EQ(expected_states[state_transition_idx][0], last_state);
			EXPECT_EQ(expected_states[state_transition_idx][1], state);
			state_transition_idx++;
		}
		last_state = state;

		/* Advance the input pointer */
		in += in_len;
		ASSERT_GE(in_ + in_len_, in);

		/* Check any potential output */
		for (uint32_t i = 0U; i < out_len; i++) {
			ASSERT_GT(out_len_, out_ptr);

			const int32_t expect = out_[out_ptr++];
			const int32_t is = out[i] >> (32 - bps);
			ASSERT_EQ(expect, is);
		}

		/* Abort once all input data has been read and all output data has been
		   written. */
		if (state == FLAC_ERR || (out_len == 0U && (in - in_) >= in_len_)) {
			break;
		}
	}
	EXPECT_EQ(7, state_transition_idx);
	free(inst);
}

void generic_test_flac_single_frame(const uint8_t *in, const uint32_t in_len,
                                    const int32_t *out, const uint32_t out_len)
{
/*	generic_test_flac_single_frame_ex(in, in_len, out, out_len, false);*/
	generic_test_flac_single_frame_ex(in, in_len, out, out_len, true);
}

void test_flac_fixed_1()
{
	generic_test_flac_single_frame(FLAC_FIXED_1, sizeof(FLAC_FIXED_1),
	                               FLAC_FIXED_1_OUT,
	                               sizeof(FLAC_FIXED_1_OUT) / 4U);
}

void test_flac_fixed_2()
{
	generic_test_flac_single_frame(FLAC_FIXED_2, sizeof(FLAC_FIXED_2),
	                               FLAC_FIXED_2_OUT,
	                               sizeof(FLAC_FIXED_2_OUT) / 4U);
}

/******************************************************************************
 * Main program                                                               *
 ******************************************************************************/

int main()
{
	RUN(test_flac_metadata_single);
	RUN(test_flac_metadata_multiple);
	RUN(test_flac_metadata_single_one_byte);
	RUN(test_flac_metadata_multiple_one_byte);
	RUN(test_flac_fixed_1);
	RUN(test_flac_fixed_2);
	DONE;
}
