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

from Queue import Queue
import logging
import os
import subprocess
from threading import Thread
from util import process_pipe, ON_POSIX

logger = logging.getLogger(__file__)


class Exec(object):
    def __init__(self, exec_path):
        self.path = exec_path
        self.name = os.path.basename(self.path)
        self.dir = os.path.dirname(self.path)
        self.out_queue = Queue()
        self.stdout_thread = None
        self.stderr_thread = None
        self.process = None

    def execute(self, args, working_directory):
        args = args
        shell = False

        args.insert(0, self.path)

        self.process = subprocess.Popen(args=args,
                                        bufsize=0,
                                        cwd=working_directory,
                                        executable=self.path,
                                        stdin=subprocess.PIPE,
                                        stdout=subprocess.PIPE,
                                        stderr=subprocess.PIPE,
                                        close_fds=ON_POSIX,
                                        universal_newlines=True,
                                        shell=shell)

        if self.process.poll() is not None:
            raise Exception("Error starting {} application {}, return code is {}".format(
                self.path,
                self.process.poll()))

        self.stdout_thread = Thread(target=process_pipe, args=(self.process.stdout, self.out_queue))
        self.stdout_thread.start()

        self.stderr_thread = Thread(target=process_pipe, args=(self.process.stderr, self.out_queue))
        self.stderr_thread.start()

    def kill(self):
        if self.process is not None:
            self.process.kill()
            self.process.stdin.close()


def get_resources_path():
    return os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "resources")
