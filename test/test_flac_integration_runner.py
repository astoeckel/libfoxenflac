#!/usr/bin/env python3

#   libfoxenflac -- Tiny FLAC Decoder Library
#   Copyright (C) 2018-2022  Andreas Stöckel
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <https://www.gnu.org/licenses/>.

# URLs from https://github.com/ruuda/claxon/tree/master/testsamples
URLS = [
    # Note 2022-02-23: for some reason these downloads are very slow.
    # As we've added the IETF FLAC WG test suite, these should no longer be
    # neccessary.

    # "I Have a Dream" by Martin Luther King in 1963, 16 bit mono, 22.05 kHz.
#    'https://archive.org/download/MLKDream/MLKDream.flac',
    # Intro of Karl Denson’s Tiny Universe live in 2015, 24 bit stereo, 96 kHz.
#    'https://archive.org/download/kdtu2015-01-07.cmc641.flac24/kdtu2015-01-07.cmc641-t01.flac',
    # "Disarm" by Smashing Pumpkins live at Lowlands 1993, 16 bit stereo, 44.1 kHz.
#    'https://archive.org/download/tsp1993-08-07.flac16/tsp1993-08-07d2t01.flac',
    # "Lowlands" by The Gourds live at Lowlands 2004, 16 bit stereo, 44.1 kHz.
#    'https://archive.org/download/gds2004-10-16.matrix.flac/gds10-16-2004d2t10.flac',
    # "Once Upon a Time" by Smashing Pumpkins live at Pinkpop 1998, 16 bit stereo, 44.1 kHz.
#    'https://archive.org/download/tsp1998-06-01.flac16/tsp1998-06-01t02.flac',

    # Some additional test files from the FFMpeg sample repository
    'http://samples.ffmpeg.org/flac/short.flac',
    'http://samples.ffmpeg.org/flac/24-bit_96kHz_RICE2_pop.flac',
    'http://samples.ffmpeg.org/flac/children.hall.24bit.5.flac',

    # Test files from  ietf-wg-cellar/flac-test-files
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/01%20-%20blocksize%204096.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/02%20-%20blocksize%204608.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/03%20-%20blocksize%2016.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/04%20-%20blocksize%20192.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/05%20-%20blocksize%20254.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/06%20-%20blocksize%20512.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/07%20-%20blocksize%20725.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/08%20-%20blocksize%201000.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/09%20-%20blocksize%201937.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/10%20-%20blocksize%202304.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/11%20-%20partition%20order%208.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/12%20-%20qlp%20precision%2015%20bit.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/13%20-%20qlp%20precision%202%20bit.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/14%20-%20wasted%20bits.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/15%20-%20only%20verbatim%20subframes.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/16%20-%20partition%20order%208%20containing%20escaped%20partitions.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/17%20-%20all%20fixed%20orders.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/18%20-%20precision%20search.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/19%20-%20samplerate%2035467Hz.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/20%20-%20samplerate%2039kHz.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/21%20-%20samplerate%2022050Hz.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/22%20-%2012%20bit%20per%20sample.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/23%20-%208%20bit%20per%20sample.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/24%20-%20variable%20blocksize%20file%20created%20with%20flake%20revision%20264.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/25%20-%20variable%20blocksize%20file%20created%20with%20flake%20revision%20264%2C%20modified%20to%20create%20smaller%20blocks.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/26%20-%20variable%20blocksize%20file%20created%20with%20CUETools.Flake%202.1.6.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/27%20-%20old%20format%20variable%20blocksize%20file%20created%20with%20Flake%200.11.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/28%20-%20high%20resolution%20audio%2C%20default%20settings.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/29%20-%20high%20resolution%20audio%2C%20blocksize%2016384.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/30%20-%20high%20resolution%20audio%2C%20blocksize%2013456.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/31%20-%20high%20resolution%20audio%2C%20using%20only%2032nd%20order%20predictors.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/32%20-%20high%20resolution%20audio%2C%20partition%20order%208%20containing%20escaped%20partitions.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/33%20-%20samplerate%20192kHz.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/34%20-%20samplerate%20192kHz%2C%20using%20only%2032nd%20order%20predictors.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/35%20-%20samplerate%20134560Hz.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/36%20-%20samplerate%20384kHz.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/37%20-%2020%20bit%20per%20sample.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/38%20-%203%20channels%20%283.0%29.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/39%20-%204%20channels%20%284.0%29.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/40%20-%205%20channels%20%285.0%29.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/41%20-%206%20channels%20%285.1%29.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/42%20-%207%20channels%20%286.1%29.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/43%20-%208%20channels%20%287.1%29.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/44%20-%208-channel%20surround%2C%20192kHz%2C%2024%20bit%2C%20using%20only%2032nd%20order%20predictors.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/45%20-%20no%20total%20number%20of%20samples%20set.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/46%20-%20no%20min-max%20framesize%20set.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/47%20-%20only%20STREAMINFO.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/48%20-%20Extremely%20large%20SEEKTABLE.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/49%20-%20Extremely%20large%20PADDING.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/50%20-%20Extremely%20large%20PICTURE.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/51%20-%20Extremely%20large%20VORBISCOMMENT.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/52%20-%20Extremely%20large%20APPLICATION.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/53%20-%20CUESHEET%20with%20very%20many%20indexes.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/54%20-%201000x%20repeating%20VORBISCOMMENT.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/55%20-%20file%2048-53%20combined.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/56%20-%20JPG%20PICTURE.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/57%20-%20PNG%20PICTURE.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/58%20-%20GIF%20PICTURE.flac',
    'https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset/59%20-%20AVIF%20PICTURE.flac'
]

HASHES = [
#    '4db90f6a5061560bd0318ce94e35d4897ba65094e22d29ebaba381f615011c8c',
#    'f5f55f6780fa6aa8905cb11d72e901e4b2db5ff7edcaff457537ad8c4865d829',
#    'b7c60aa5d15975ca856096f93359faeed6f4c24931cee68430ffe5c9164f7eb6',
#    '7e9df9134879f35c830433fa7591605115b50ddba36c1ebd509e36c16d5981d4',
#    'd1792d3f89debd57e60f54a296de3e347ce866b1d324bb1128e7c9aa61de1294',
    '29e1b72e4c0bc39b0c7c0a1cfd5a1b2c46e41966210e1d0f1f276bc2bfe4b912',
    'e376dcc1ad951227e0e78b9b0bf44f4b87c379419ea6e8d7b4216d98dd75a0a0',
    '78dbb18b3719d0f10e11e80cea2b007a83219dcf2d698c89cfa66a3e1bdb2212',
    '02c7e60ae7788c2cb6898a0a46985d759f1e62482ddd965a374dfd09a2a8390f',
    'f4e97914fb9a8cd95b9be90c9970f6d553d76ded55a7a443dbd0d5dfa7bd2819',
    '164d0747e8c4855407990182b784fcd7475da228bf20a6cff67e947c683959b3',
    '0f9f7e2fc262e16e8cd92de3e8869d13197a5e9c3ef7239f8ee749cbd2689980',
    '8965c9616f769573d3bf8ef5b92ee5a2827e797c098a9436a5e3366feb9c9ae2',
    'ba705cc365abf5b8bd6b4d4392a2668ae395844f82d04e74be3e19b60cb03260',
    'ffc15a2ead7a6651e2cd36fa886edcffffc32e831077dce852fb7df6f131d2b3',
    'fa189c885be140510348f18899fe151a903f0199d502982ecc7863cb85810ba6',
    '8786ea66bf5d0083fcf760b874216ac566d0b2688ba58e5711f46525841a0028',
    '8ec67465e9041373a67f2440220825ba9d7706789584bffbbb3f109424df25ee',
    '07596315fc9b297702984deab49041f4695e1482ec3e1d081888d1bf13198e53',
    'd33731350269929542c6e0a788c1b9a27eb91dcb3a20f5f960b8569be012c6fa',
    'b60e113a6a3b97d7a3c48be90913ced8c85f1ee332c6c5dd5a79b46cdc38108a',
    '58fa05681bd646168ea2a19bc9a6f4ee9d6de295fd832c1f70da1bdbf32d74cb',
    '80459f8ad5a9c7edf081a10f9269c55c29136749021e5a55483bc173aaaa82b0',
    '5562cff546a1b0d93cea31c0c375c5df65658311de1fc3d322d14b8a2c5ac930',
    'd6f082cbbbcb03015f861b8fa30cf16708a62b85520905a126da28db29645feb',
    '562dd053a92a83bf64baf90ae5f9b3d0af58121c5534b3c05353d77b2ff25aaa',
    '72ee52c82ee46eea28d659392ce75557e66df1051563fde1936d3e5a8fd8d2c2',
    '9c87394f650fda9d08bc6b4a7c56a4fd56dcac3531913aa536e7d4d576580ee0',
    'a808ffa5c3cb99c7bc8268141f2d01fb7ccb4c3ced617fd7909f578fb9363f88',
    'd9c385129c4253da4c0de34ed5b163b404523aa971773277b8b74ef92f7d4f44',
    'd72ad54e1d0ebe5d65b1b1addd13ee873d38c58cb3357774a1943ced2d984cf9',
    'c6664710c935e4d862ab04f146ddd7f6df9e2278b651c17a432934e61488eaf7',
    'df5658969201d6867a1cc64d69e30a8ec833b53f7239345eda7eefff1a4a2ad8',
    'ceedbc2e070ed73d34f3d941acefdbfd8509e1c219cfd5997d759db478e5f354',
    '52afdf4bcabe55feeb99f5d52d28f7bc3a376400e13c4e52f9a98e3c54c9f5db',
    '62322a5c7821081bae99cc1a3f398f177a17cf9312131a5fbcfcc2ee38539346',
    '7551605d8a90a7e74e788b31273493ce5e24059d839b1d44dbad89cf9fb132a5',
    'd2e6b73cc0888419d709a03c317f59dfb8d48fcdc29e73fb0c099d5de7d512be',
    'caf84fbf740a98e3371a58df03422722a7d6058289b11a291f0b39103ea20133',
    '479474ac6a101725976aa9a4e94580040d650779da41db2981be03aa41ef7fdd',
    '4833aafc6a7d652bac05de6d304d3e63cc419a731df995b146d91294062bbcd0',
    'b32a7e21114034610fea4d43610980111dc9c40faa315a3111196102a7bac36e',
    'ffe10b26321fdc1a1be31cfd7034a4f98b1c0cb3e32e9c1cc129b7ff4a259177',
    '63fc6b1ae16fc0285f7053f49ce967087f6a041630ee105d3d67fd8d34e45b7c',
    '8d8c0526b7823099974a348a6ae80eff9c7eeb4be4e3da270e8e2e9e45d74b3c',
    '44d14d0efa374049fd9f56aaee3a068a30045e62a571df574c7b824ae596a3cf',
    'ac670ce52561023dca32f7bd20fbe94c1e1459f06188cd0f870801ea5fb23cee',
    'bf616e933caab9b7a5da64f9527390f83f208134d327c72a16225c0779ba0da0',
    'c12650fda54a384abf017d4b931491de043eaa7a815ae74c7a9b49d4087d6193',
    '07a5d7fc8c4dd74c634b2f0e912a063496f67dd2ba9fbe14f7b5fa37fee8f638',
    '9c39ff436f316b3a8e72200c8f38af2d9ef15418c25be555aa621d0c793a6254',
    '1a0e8c17392ae03b2cb04e8c82fcc1f924580fdc978884101faa3e9cf7ed9197',
    '336a18eb7a78f7fc0ab34980348e2895bc3f82db440a2430d9f92e996f889f9a',
    '9dc39732ce17815832790901b768bb50cd5ff0cd21b28a123c1cabc16ed776cc',
    '9a62c79f634849e74cb2183f9e3a9bd284f51e2591c553008d3e6449967eef85',
    '4417aca6b5f90971c50c28766d2f32b3acaa7f9f9667bd313336242dae8b2531',
    '7bc44fa2754536279fde4f8fb31d824f43b8d0b3f93d27d055d209682914f20e',
    '1f04f237d74836104993a8072d4223e84a5d3bd76fbc44555c221c7e69a23594',
    '033160e8124ed287b0b5d615c94ac4139477e47d6e4059b1c19b7141566f5ef9',
    '0e45a4f8dbef15cbebdd8dfe690d8ae60e0c6abb596db1270a9161b62a7a3f1c',
    '513fad18578f3225fae5de1bda8f700415be6fd8aa1e7af533b5eb796ed2d461',
    'b68dc6644784fac35aa07581be8603a360d1697e07a2265d7eb24001936fd247',
    'd70849079d811c414b880558463899922f44851cf4d863eb38ea8407c3fe1ff0',
    '5cebe7a3710cf8924bd2913854e9ca60b4cd53cfee5a3af0c3c73fddc1888963',
    'c6abff7f8bb63c2821bd21dd9052c543f10ba0be878e83cb419c248f14f72697',
    '7c2b1a963a665847167a7275f9924f65baeb85c21726c218f61bf3f803f301c8',
    '7395d02bf8d9533dc554cce02dee9de98c77f8731a45f62d0a243bd0d6f9a45c',
]

# Parse the arguments
import argparse
parser = argparse.ArgumentParser()
parser.add_argument('--song-dir', type=str, required=True)
parser.add_argument('--exe', type=str, required=True)
parser.add_argument('--download', dest='download', action='store_true')
parser.add_argument('--no-download', dest='download', action='store_false')
parser.add_argument('--run', dest='run', action='store_true')
parser.add_argument('--no-run', dest='run', action='store_false')
parser.add_argument('--threads', type=int, default=0)
parser.set_defaults(download=True)
parser.set_defaults(run=True)
args = parser.parse_args()

# Download the files if they do not exist yet
import hashlib
import os
files = []
for i, url in enumerate(URLS):
    # Generate the file name
    file = hashlib.sha256(url.encode('utf-8')).hexdigest()[:8]
    file = os.path.join(args.song_dir, file + '.flac')
    files.append((url, file))

    # Download the file if it does not yet exist
    if (not os.path.isfile(file)) and args.download:
        import urllib.request
        print('Downloading {} to {}...'.format(url, file))
        try:
            response = urllib.request.urlopen(url)
            data = response.read()
            os.makedirs(os.path.dirname(file), exist_ok=True)
            with open(file, 'wb') as f:
                f.write(data)
            print('Done.')
        except:
            print('Warning: Error downloading file. Skipping...')

# Iterate over the files and execute the integration test on them
errors = []
processes = []
def wait_for_processes():
    global has_err, processes
    for p, file in processes:
        code = p.wait()
        if code != 0:
            errors.append((file, code))
    processes = []

import subprocess
import multiprocessing
for i, (url, file) in enumerate(files):
    if os.path.isfile(file):
        # Compute the file hash
        with open(file, 'rb') as f:
            data = f.read()
            hash_ = hashlib.sha256(data).hexdigest();

        # Abort if the hash does not match
        if i < len(HASHES):
            if hash_ != HASHES[i]:
                print('Warning: Hash mismatch for file \"{}\": expected \"{}\", got \"{}\"'.format(file, HASHES[i], hash_))
                continue
        else:
            print("Not checking file hash, append {} to the HASHES list".format(hash_))

    if args.run:
        # Start the integration test for the file
        processes.append((subprocess.Popen([os.path.abspath(args.exe), file]), (url, file)))

        # Make sure that there is only a limited number of processes running
        # at a time
        n_threads = args.threads if args.threads > 0 else multiprocessing.cpu_count()
        if len(processes) >= n_threads:
            wait_for_processes()

wait_for_processes()

for file, code in errors:
    print("Error while processing {} (code {})".format(file, code))
    has_err = True


# Return the error code to the top-level script
import sys
sys.exit(1 if errors else 0)

