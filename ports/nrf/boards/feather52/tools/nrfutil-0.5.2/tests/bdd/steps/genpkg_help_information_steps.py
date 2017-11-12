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

from Queue import Empty
import logging
import os
import time
import sys

from click.testing import CliRunner
from behave import then, given, when

from nordicsemi.__main__ import cli, int_as_text_to_int


logger = logging.getLogger(__file__)

STDOUT_TEXT_WAIT_TIME = 50  # Number of seconds to wait for expected output from stdout


@given(u'user types \'{command}\'')
def step_impl(context, command):
    args = command.split(' ')
    assert args[0] == 'nrfutil'

    exec_args = args[1:]

    runner = CliRunner()
    context.runner = runner
    context.args = exec_args


@then(u'output contains \'{stdout_text}\' and exit code is {exit_code}')
def step_impl(context, stdout_text, exit_code):
    result = context.runner.invoke(cli, context.args)
    logger.debug("exit_code: %s, output: \'%s\'", result.exit_code, result.output)
    assert result.exit_code == int_as_text_to_int(exit_code)
    assert result.output != None
    assert result.output.find(stdout_text) >= 0
