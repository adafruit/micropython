# Copyright (c) 2015, Nordic Semiconductor
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# * Neither the name of Nordic Semiconductor ASA nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

from random import randint
import time
import sys
import math

ON_POSIX = 'posix' in sys.builtin_module_names


def process_pipe(pipe, queue):
    for line in iter(pipe.readline, b''):
        queue.put({'type': 'output', 'data': line})

    pipe.close()
    queue.put({'type': 'output_terminated'})


def kill_process(target):
    if 'proc' in target:
        target['proc'].kill()

        # Close file descriptors
        target['proc'].stdin.close()
        time.sleep(1)  # Let the application terminate before proceeding


def kill_processes(context):
    targets = context.target_registry.get_all()

    for target in targets:
        kill_process(target)


def generate_options_table_for_cucumber():
    retval = ""

    number_of_2_option_options = 1
    number_of_3_option_options = 4
    number_of_4_option_options = 1

    number_of_optional_option_permutations = 1
    number_of_optional_option_permutations *= int(math.pow(2, number_of_2_option_options))
    number_of_optional_option_permutations *= int(math.pow(3, number_of_3_option_options))
    number_of_optional_option_permutations *= int(math.pow(4, number_of_4_option_options))

    for x in xrange(0, number_of_optional_option_permutations):
        retval += "{0:<8}".format(" ")
        retval += "| {0:<12}| {1:<29}| {2:<29}|".format("blinky.bin", "not_set", "not_set")

        permutation_name = ""
        options_factor = 1

        option = int(x / options_factor % 3)
        options_factor *= 3
        permutation_name = str(option) + permutation_name

        if option == 0:
            retval += " {0:<8}|".format("none")
        if option == 1:
            retval += " {0:<8}|".format("not_set")
        if option == 2:
            retval += " {0:<8}|".format("0x{0:02x}".format(randint(0, 255)))

        option = int(x / options_factor % 3)
        options_factor *= 3
        permutation_name = str(option) + permutation_name

        if option == 0:
            retval += " {0:<8}|".format("none")
        if option == 1:
            retval += " {0:<8}|".format("not_set")
        if option == 2:
            retval += " {0:<8}|".format("0x{0:02x}".format(randint(0, 255)))

        option = int(x / options_factor % 3)
        options_factor *= 3
        permutation_name = str(option) + permutation_name

        if option == 0:
            retval += " {0:<9}|".format("none")
        if option == 1:
            retval += " {0:<9}|".format("not_set")
        if option == 2:
            retval += " {0:<9}|".format("0x{0:02x}".format(randint(0, 255)))


        option = int(x / options_factor % 4)
        options_factor *= 4
        permutation_name = str(option) + permutation_name

        if option == 0:
            retval += " {0:<8}|".format("not_set")
        if option == 1:
            retval += " {0:<8}|".format("0.5")
        if option == 2:
            retval += " {0:<8}|".format("0.6")
        if option == 3:
            retval += " {0:<8}|".format("0.7")

        option = int(x / options_factor % 3)
        options_factor *= 3
        permutation_name = str(option) + permutation_name

        if option == 0:
            retval += " {0:<28}|".format("none")
        if option == 1:
            retval += " {0:<28}|".format("not_set")
        if option == 2:
            sd_reqs = []

            for i in xrange(0, randint(1, 4)):
                sd_reqs.append("0x{0:04x}".format(randint(0, 65535)))

            retval += " {0:<28}|".format(",".join(sd_reqs))

        option = int(x / options_factor % 2)
        permutation_name = str(option) + permutation_name

        if option == 0:
            retval += " {0:<9}|".format("not_set")
        if option == 1:
            retval += " {0:<9}|".format("test.pem")

        retval += " {0:<15}|".format("100_{0:0>6}.zip".format(permutation_name))
        retval += "\n"

    return retval
