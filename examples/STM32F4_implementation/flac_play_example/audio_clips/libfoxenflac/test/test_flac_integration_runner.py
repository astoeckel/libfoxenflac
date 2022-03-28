#!/usr/bin/env python3

#   libfoxenflac -- FLAC decoder
#   Copyright (C) 2018  Andreas Stöckel
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU Affero General Public License as
#   published by the Free Software Foundation, either version 3 of the
#   License, or (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Affero General Public License for more details.
#
#   You should have received a copy of the GNU Affero General Public License
#   along with this program.  If not, see <https://www.gnu.org/licenses/>.

# URLs from https://github.com/ruuda/claxon/tree/master/testsamples
URLS = [
    # "I Have a Dream" by Martin Luther King in 1963, 16 bit mono, 22.05 kHz.
    'https://archive.org/download/MLKDream/MLKDream.flac',
    # Intro of Karl Denson’s Tiny Universe live in 2015, 24 bit stereo, 96 kHz.
    'https://archive.org/download/kdtu2015-01-07.cmc641.flac24/kdtu2015-01-07.cmc641-t01.flac',
    # "Disarm" by Smashing Pumpkins live at Lowlands 1993, 16 bit stereo, 44.1 kHz.
    'https://archive.org/download/tsp1993-08-07.flac16/tsp1993-08-07d2t01.flac',
    # "Lowlands" by The Gourds live at Lowlands 2004, 16 bit stereo, 44.1 kHz.
    'https://archive.org/download/gds2004-10-16.matrix.flac/gds10-16-2004d2t10.flac',
    # "Once Upon a Time" by Smashing Pumpkins live at Pinkpop 1998, 16 bit stereo, 44.1 kHz.
    'https://archive.org/download/tsp1998-06-01.flac16/tsp1998-06-01t02.flac',

    # Some additional test files from the FFMpeg sample repository
    'http://samples.ffmpeg.org/flac/short.flac',
    'http://samples.ffmpeg.org/flac/24-bit_96kHz_RICE2_pop.flac',
    'http://samples.ffmpeg.org/flac/children.hall.24bit.5.flac',
]

HASHES = [
    '4db90f6a5061560bd0318ce94e35d4897ba65094e22d29ebaba381f615011c8c',
    'f5f55f6780fa6aa8905cb11d72e901e4b2db5ff7edcaff457537ad8c4865d829',
    'b7c60aa5d15975ca856096f93359faeed6f4c24931cee68430ffe5c9164f7eb6',
    '7e9df9134879f35c830433fa7591605115b50ddba36c1ebd509e36c16d5981d4',
    'd1792d3f89debd57e60f54a296de3e347ce866b1d324bb1128e7c9aa61de1294',
    '29e1b72e4c0bc39b0c7c0a1cfd5a1b2c46e41966210e1d0f1f276bc2bfe4b912',
    'e376dcc1ad951227e0e78b9b0bf44f4b87c379419ea6e8d7b4216d98dd75a0a0',
    '78dbb18b3719d0f10e11e80cea2b007a83219dcf2d698c89cfa66a3e1bdb2212'
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
    files.append(file)

    # Download the file if it does not yet exist
    if (not os.path.isfile(file)) and args.download:
        import urllib.request
        print('Downloading {}...'.format(url))
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
has_err = False
processes = []
def wait_for_processes():
    global has_err, processes
    for p in processes:
        if p.wait() != 0:
            has_err = True
    processes = []

import subprocess
import multiprocessing
for i, file in enumerate(files):
    if os.path.isfile(file) and args.run:
        # Compute the file hash
        with open(file, 'rb') as f:
            data = f.read()
            hash_ = hashlib.sha256(data).hexdigest();

        # Abort if the hash does not match
        if hash_ != HASHES[i]:
            print('Warning: Hash mismatch for file \"{}\": expected \"{}\", got \"{}\"'.format(file, HASHES[i], hash_))
            continue

        # Start the integration test for the file
        processes.append(subprocess.Popen([os.path.abspath(args.exe), file]))

        # Make sure that there is only a limited number of processes running
        # at a time
        if len(processes) >= multiprocessing.cpu_count():
            wait_for_processes()

wait_for_processes()

# Return the error code to the top-level script
import sys
sys.exit(1 if has_err else 0)

