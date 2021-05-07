#! /usr/bin/env python3

# SPDX-FileCopyrightText: 2014 MicroPython & CircuitPython contributors (https://github.com/adafruit/circuitpython/graphs/contributors)
#
# SPDX-License-Identifier: MIT

import os
import multiprocessing
import sys
import subprocess
import shutil
import build_board_info as build_info
import time

for port in build_info.SUPPORTED_PORTS:
    result = subprocess.run(f"rm -rf ../ports/{port}/build*", shell=True)

PARALLEL = "-j 5"
if "GITHUB_ACTION" in os.environ:
    PARALLEL = "-j 2"

all_boards = build_info.get_board_mapping()
build_boards = list(all_boards.keys())
if "BOARDS" in os.environ:
    build_boards = os.environ["BOARDS"].split()

sha, version = build_info.get_version_info()

languages = build_info.get_languages()

all_languages = build_info.get_languages(list_all=True)

print("Note: Not building languages", set(all_languages) - set(languages))

exit_status = 0
cores = multiprocessing.cpu_count()
print(f"building boards with parallelism {cores}")
for board in build_boards:
    bin_directory = f"../bin/{board}/"
    os.makedirs(bin_directory, exist_ok=True)
    board_info = all_boards[board]
    port = board_info["port"]

    variants = [(language, language, "") for language in languages]
    # Add non-language variant builds.
    variants.append(("CIRCUITPY_ERASER", "en_US", "CIRCUITPY_ERASER=1"))

    for (variant_name, language, settings) in variants:
        bin_directory = f"../bin/{board}/{variant_name}"
        os.makedirs(bin_directory, exist_ok=True)
        start_time = time.monotonic()

        # Normally different language/variant builds are all done based on the same set of compiled sources.
        # But sometimes a particular language needs to be built from scratch, if, for instance,
        # CFLAGS_INLINE_LIMIT is set for a particular language to make it fit.
        clean_build_check_result = subprocess.run(
            f"make -C ../ports/{port} TRANSLATION={language} BOARD={board} {settings} check-release-needs-clean-build -j {cores} | fgrep 'RELEASE_NEEDS_CLEAN_BUILD = 1'",
            shell=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )

        # If settings is non-empty, assume we need a clean build.
        clean_build = clean_build_check_result.returncode == 0 or settings

        build_dir = f"build-{board}"
        if clean_build:
            build_dir += f"-{variant_name}"

        make_result = subprocess.run(
            f"make -C ../ports/{port} TRANSLATION={language} BOARD={board} {settings} BUILD={build_dir} -j {cores}",
            shell=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )

        build_duration = time.monotonic() - start_time
        success = "\033[32msucceeded\033[0m"
        if make_result.returncode != 0:
            exit_status = make_result.returncode
            success = "\033[31mfailed\033[0m"

        other_output = ""

        for extension in board_info["extensions"]:
            temp_filename = f"../ports/{port}/{build_dir}/firmware.{extension}"
            for alias in board_info["aliases"] + [board]:
                bin_directory = f"../bin/{alias}/{variant_name}"
                os.makedirs(bin_directory, exist_ok=True)
                final_filename = (
                    f"adafruit-circuitpython-{alias}-{variant_name}-{version}.{extension}"
                )
                final_filename = os.path.join(bin_directory, final_filename)
                try:
                    shutil.copyfile(temp_filename, final_filename)
                except FileNotFoundError:
                    other_output = f"Cannot find file {temp_filename}"
                    if exit_status == 0:
                        exit_status = 1

        clean_build_str = " (clean_build)" if clean_build else ""
        print(
            f"Build {board} for {variant_name}{clean_build_str} took {build_duration:.2f}s and {success}"
        )

        print(make_result.stdout.decode("utf-8"))
        print(other_output)

        # Flush so we will see something before 10 minutes has passed.
        print(flush=True)

sys.exit(exit_status)
