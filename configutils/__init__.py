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
"""Configuration utilities.

"""

import os
import sys
from config import config
from build import build_ext

# Use setuptools if we're part of a larger build system which is already using it.
if ('setuptools' in sys.modules):
    import setuptools
    from setuptools import setup, Extension
    using_setuptools = True
else:
    import distutils
    from distutils.core import setup, Extension
    from distutils import sysconfig
    using_setuptools = False


# Customise compiler options.
if not using_setuptools:
    distutils_customize_compiler = sysconfig.customize_compiler
    def my_customize_compiler(compiler):
        """Remove the -Wstrict-prototypes option from the compiler command.

        This option isn't supported for C++, so we remove it to avoid annoying
        warnings.

        """
        retval = distutils_customize_compiler(compiler)
        try:
            compiler.compiler_so.remove('-Wstrict-prototypes')
        except (AttributeError, ValueError):
            pass
        return retval
    sysconfig.customize_compiler = my_customize_compiler

del os
del sys
