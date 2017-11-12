#!/usr/bin/env python
#
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

"""
Setup script for nrfutil.

USAGE:
    python setup.py install
    python setup.py py2exe

"""
import os
import platform

from setuptools import setup, find_packages
from setuptools.command.test import test as TestCommand
#from setuptools_behave import behave_test

from nordicsemi import version

if platform.system() == 'Windows':
    import py2exe  # Required even if it is not used in this file. This import adds py2exe to distutils.

excludes = ["Tkconstants",
            "Tkinter",
            "tcl",
            "pickle",
            "unittest",
            "pyreadline"]

# DFU component cli interface
includes = ["nordicsemi.dfu.dfu"]

packages = []

dll_excludes = [
    "w9xpopen.exe",
    "OLEAUT32.DLL",
    "OLE32.DLL",
    "USER32.DLL",
    "SHELL32.DLL",
    "ADVAPI32.DLL",
    "KERNEL32.DLL",
    "WS2_32.DLL",
    "GDI32.DLL"]

build_dir = os.environ.get("NRFUTIL_BUILD_DIR", "./{}".format(version.NRFUTIL_VERSION))
description = """A Python package that includes the nrfutil utility and the nordicsemi library"""


class NoseTestCommand(TestCommand):
    def finalize_options(self):
        TestCommand.finalize_options(self)
        self.test_args = []
        self.test_suite = True

    def run_tests(self):
        import nose
        nose.run_exit(argv=['nosetests', '--with-xunit', '--xunit-file=test-reports/unittests.xml'])

common_requirements=[]

setup(
    name="nrfutil",
    version=version.NRFUTIL_VERSION,
    license="Nordic Semicondictor proprietary license",
    url="https://github.com/NordicSemiconductor/pc-nrfutil",
    description="Nordic Semiconductor nrfutil utility and Python library",
    long_description=description,
    packages=find_packages(exclude=["tests.*", "tests"]),
    include_package_data=False,
    install_requires=common_requirements,
    setup_requires=common_requirements,
    zipfile=None,
    tests_require=[
        "nose >= 1.3.4",
        "behave"
    ],
    zip_safe=False,
    classifiers=[
        "Programming Language :: Python :: 2.7",
    ],
    cmdclass={
        'test': NoseTestCommand
        # 'bdd_test': behave_test
    },
    entry_points='''
      [console_scripts]
      nrfutil = nordicsemi.__main__:cli
    ''',
    console=[{
        "script": "./nordicsemi/__main__.py",
        "dest_base": "nrfutil"
    }],
    options={
        "py2exe": {
            "includes": includes,
            "excludes": excludes,
            "ascii": False,
            "bundle_files": 1,  # 1 for bunding into exe, 3 for to distdir
            "dist_dir": build_dir,
            "verbose": True,
            "dll_excludes": dll_excludes
        }
    }
)
