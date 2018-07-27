#  libfoxenflac -- FLAC decoder
#  Copyright (C) 2018  Andreas Stöckel
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Affero General Public License as
#  published by the Free Software Foundation, either version 3 of the
#  License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Affero General Public License for more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.

project('libfoxenflac', 'c', default_options : ['c_std=c99'])
compiler = meson.get_compiler('c')
if compiler.get_id() == 'gcc' or compiler.get_id() == 'clang'
	add_global_arguments('-Wc++-compat', language : 'c')
endif

# Include directory
inc_foxen = include_directories('.')

# FLAC depends on the bitstream and memory library
dep_foxenbitstream = dependency(
    'libfoxenbitstream',
    fallback:['libfoxenbitstream', 'dep_foxenbitstream'])
dep_foxenmem = dependency(
    'libfoxenmem',
    fallback:['libfoxenmem', 'dep_foxenmem'])

# Define the contents of the actual library
lib_foxenflac = library(
    'foxenflac',
    'foxen/flac.c',
    include_directories: inc_foxen,
    dependencies: [dep_foxenbitstream, dep_foxenmem],
    install: true)

# Compile the example programs
exe_flac_decoder = executable(
    'flac_decoder',
    'examples/flac_decoder.c',
    include_directories: inc_foxen,
    link_with: lib_foxenflac,
    install: false)

# Compile and register the unit tests
dep_foxenunit = dependency(
    'libfoxenunit',
    fallback:['libfoxenunit', 'dep_foxenunit'])
exe_test_flac = executable(
    'test_flac',
    'test/test_flac.c',
    include_directories: inc_foxen,
    link_with: lib_foxenflac,
    dependencies: dep_foxenunit,
    install: false)
test('test_flac', exe_test_flac)

exe_test_flac_integration = executable(
    'test_flac_integration',
    'test/test_flac_integration.c',
    include_directories: inc_foxen,
    link_with: lib_foxenflac,
    dependencies: [dep_foxenunit, dep_foxenbitstream],
    install: false)
# TODO: Use Python script instead
test(
    'test_flac_integration_wasted_bits',
    exe_test_flac_integration,
    args: [join_paths(meson.source_root(), 'test/data/wasted_bits.flac')])
test(
    'test_flac_integration_short',
    exe_test_flac_integration,
    args: [join_paths(meson.source_root(), 'test/data/short.flac')])

# Install the header file
install_headers(
    ['foxen/flac.h'],
    subdir: 'foxen')

# Generate a Pkg config file
pkg = import('pkgconfig')
pkg.generate(
    libraries: [lib_foxenflac],
    subdirs: [],
    name: 'libfoxenflac',
    version: '1.0',
    filebase: 'libfoxenflac',
    description: 'FLAC decoder')

# Export the dependency
dep_foxenflac = declare_dependency(
    link_with: lib_foxenflac,
    include_directories: inc_foxen)
