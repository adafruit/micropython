# Copyright (c) 2011, Bernhard Leiner
# All rights reserved.
#
# Redistribution and use in source and binary forms,
# with or without modification, are permitted provided
# that the following conditions are met:
#
# * Redistributions of source code must retain
#   the above copyright notice, this list of conditions
#   and the following disclaimer.
# * Redistributions in binary form must reproduce
#   the above copyright notice, this list of conditions
#   and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
# * Neither the name of the author nor the names
#   of its contributors may be used to endorse
#   or promote products derived from this software
#   without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
# BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
# OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
# OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

'''Compatibility functions for python 2 and 3.

@author     Bernhard Leiner (bleiner AT gmail com)
@version    1.0
'''

__docformat__ = "javadoc"


import sys

if sys.version_info[0] >= 3:
    def asbytes(s):
        if isinstance(s, bytes):
            return s
        return s.encode('latin1')
    def asstr(s):
        if isinstance(s, str):
            return s
        return s.decode('latin1')
else:
    asbytes = str
    asstr = str

