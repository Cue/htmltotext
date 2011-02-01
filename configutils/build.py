#!/usr/bin/env python
#
# Copyright (C) 2008 Lemur Consulting Ltd
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
"""Configuration tests.

"""

import os
import sys
import cPickle

if ('setuptools' in sys.modules):
    from setuptools.command.build_ext import build_ext as du_build_ext
else:
    from distutils.command.build_ext import build_ext as du_build_ext

def read_params():
    path = os.path.join('configutils', 'params.cache')
    try:
        fd = open(path, "rb")
        stored_params = cPickle.load(fd)
        fd.close()
        return stored_params
    except IOError:
        return {}

class build_ext(du_build_ext):
    def use_param(self, params, param):
        if param not in params:
            return
        val = getattr(self, param, [])
        val.extend(params[param])
        setattr(self, param, val)
        
    def run(self):
        # FIXME - both of these should be in the build directory.
        configpath = os.path.join('src', 'config.h')
        paramspath = os.path.join('configutils', 'params.cache')
        if not os.path.exists(configpath) or not os.path.exists(paramspath):
            self.run_command('config')
        params = read_params()
        self.use_param(params, 'include_dirs')
        self.use_param(params, 'library_dirs')
        self.use_param(params, 'libraries')

        du_build_ext.run(self)

    def build_extension(self, ext):
        """Remove the -Wstrict-prototypes option from the compiler command.

        This option isn't supported for C++, so we remove it to avoid annoying
        warnings.

        """
        try:
            self.compiler.compiler_so.remove('-Wstrict-prototypes')
        except (AttributeError, ValueError):
            pass
        return du_build_ext.build_extension(self, ext)
