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

import re
import os
import json
from abc import ABCMeta, abstractmethod


class TargetDatabase(object):
    __metaclass__ = ABCMeta

    @abstractmethod
    def get_targets(self):
        pass

    @abstractmethod
    def get_target(self, target_id):
        pass

    @abstractmethod
    def refresh(self):
        pass

    @staticmethod
    def find_target(targets, target_id):
        for target in targets:
            if target["id"] == target_id:
                return target

        return None


class EnvTargetDatabase(TargetDatabase):
    def __init__(self):
        self.targets = None

    def get_targets(self):
        if self.targets is None:
            self.targets = []

            for key, value in os.environ.iteritems():
                match = re.match("NORDICSEMI_TARGET_(?P<target>\d+)_(?P<key>[a-zA-Z_]+)", key)

                if match:
                    key_value = match.groupdict()
                    if "key" in key_value and "target" in key_value:
                        target_id = int(key_value["target"])

                        target = self.find_target(self.targets, target_id)

                        if target is None:
                            target = {"id": int(target_id)}
                            self.targets.append(target)

                        target[key_value["key"].lower()] = value

        return self.targets

    def refresh(self):
        self.targets = None

    def get_target(self, target_id):
        return self.find_target(self.get_targets(), target_id)


class FileTargetDatabase(TargetDatabase):
    def __init__(self, filename):
        self.filename = filename
        self.targets = None

    def get_targets(self):
        if not self.targets:
            self.targets = json.load(open(self.filename, "r"))["targets"]

        return self.targets

    def get_target(self, target_id):
        return self.find_target(self.get_targets(), target_id)

    def refresh(self):
        self.targets = None


class TargetRegistry(object):
    def __init__(self, target_db=EnvTargetDatabase()):
        self.target_db = target_db

    def find_one(self, target_id=None):
        if target_id:
            return self.target_db.get_target(target_id)
        else:
            return None

    def get_all(self):
        return self.target_db.get_targets()
