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

#include <stdlib.h>

#include <foxen/flac.h>
#include <foxen/unittest.h>

/******************************************************************************
 * Unit tests                                                                 *
 ******************************************************************************/

void test_flac_metadata_simple()
{
	const uint8_t data[] = {
	    0x66, 0x4C, 0x61, 0x43, 0x80, 0x00, 0x00, 0x22, 0x10, 0x00, 0x10,
	    0x00, 0x00, 0x00, 0x10, 0x00, 0x31, 0x97, 0x0A, 0xC4, 0x42, 0xF0,
	    0x00, 0x8A, 0x48, 0x96, 0x45, 0x61, 0x31, 0x02, 0x8B, 0xFB, 0x21,
	    0xE5, 0x5F, 0xFB, 0x6E, 0xDF, 0x48, 0xCE, 0x9F, 0xAE};
	fx_flac_t *inst = fx_flac_init(malloc(fx_flac_size()));
	uint32_t len = sizeof(data);
	EXPECT_EQ(FLAC_END_OF_METADATA,
	          fx_flac_process(inst, data, &len, NULL, NULL));
	EXPECT_EQ(sizeof(data), len);
	EXPECT_EQ(4096U, fx_flac_get_streaminfo(inst, FLAC_MIN_BLOCK_SIZE));
	EXPECT_EQ(4096U, fx_flac_get_streaminfo(inst, FLAC_MAX_BLOCK_SIZE));
	EXPECT_EQ(16U, fx_flac_get_streaminfo(inst, FLAC_MIN_FRAME_SIZE));
	EXPECT_EQ(12695U, fx_flac_get_streaminfo(inst, FLAC_MAX_FRAME_SIZE));
	EXPECT_EQ(44100U, fx_flac_get_streaminfo(inst, FLAC_SAMPLE_RATE));
	EXPECT_EQ(2U, fx_flac_get_streaminfo(inst, FLAC_N_CHANNELS));
	EXPECT_EQ(16U, fx_flac_get_streaminfo(inst, FLAC_BITS_PER_SAMPLE));
	EXPECT_EQ(9062550U, fx_flac_get_streaminfo(inst, FLAC_N_SAMPLES));
	EXPECT_EQ(0x45, fx_flac_get_streaminfo(inst, FLAC_MD5_SUM_0));
	EXPECT_EQ(0x61, fx_flac_get_streaminfo(inst, FLAC_MD5_SUM_1));
	EXPECT_EQ(0x31, fx_flac_get_streaminfo(inst, FLAC_MD5_SUM_2));
	EXPECT_EQ(0x02, fx_flac_get_streaminfo(inst, FLAC_MD5_SUM_3));
	EXPECT_EQ(0x8B, fx_flac_get_streaminfo(inst, FLAC_MD5_SUM_4));
	EXPECT_EQ(0xFB, fx_flac_get_streaminfo(inst, FLAC_MD5_SUM_5));
	EXPECT_EQ(0x21, fx_flac_get_streaminfo(inst, FLAC_MD5_SUM_6));
	EXPECT_EQ(0xE5, fx_flac_get_streaminfo(inst, FLAC_MD5_SUM_7));
	EXPECT_EQ(0x5F, fx_flac_get_streaminfo(inst, FLAC_MD5_SUM_8));
	EXPECT_EQ(0xFB, fx_flac_get_streaminfo(inst, FLAC_MD5_SUM_9));
	EXPECT_EQ(0x6E, fx_flac_get_streaminfo(inst, FLAC_MD5_SUM_A));
	EXPECT_EQ(0xDF, fx_flac_get_streaminfo(inst, FLAC_MD5_SUM_B));
	EXPECT_EQ(0x48, fx_flac_get_streaminfo(inst, FLAC_MD5_SUM_C));
	EXPECT_EQ(0xCE, fx_flac_get_streaminfo(inst, FLAC_MD5_SUM_D));
	EXPECT_EQ(0x9F, fx_flac_get_streaminfo(inst, FLAC_MD5_SUM_E));
	EXPECT_EQ(0xAE, fx_flac_get_streaminfo(inst, FLAC_MD5_SUM_F));
	free(inst);
}

/******************************************************************************
 * Main program                                                               *
 ******************************************************************************/

int main()
{
	RUN(test_flac_metadata_simple);
	DONE;
}
