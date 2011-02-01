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

import cPickle
import os
import platform
import sys

from distutils import log
from distutils.command.config import config as du_config

# Add common paths to look in make compilation more likely to work without extra
# command line parameters.
if platform.system() == 'Windows':
    extra_include_dirs = [r'c:\program files\gnuwin32\include']
    extra_library_dirs = [r'c:\program files\gnuwin32\lib']
else:
    extra_include_dirs = ['/usr/local/include']
    extra_library_dirs = ['/usr/local/lib']


iconv_inputtype_code = '''
int main(int argc, const char ** argv) {
    char buf[20];
    %(t)s in = "hello";
    size_t in_len = 5;
    char * out = buf;
    size_t out_len = 20;
    iconv((iconv_t)-1, &in, &in_len, &out, &out_len);
    return 0;
}
'''.strip()

class config(du_config):

    def _check_compiler(self):
        du_config._check_compiler(self)
        try:
            self.compiler.compiler_so.remove('-Wstrict-prototypes')
            self.compiler.compiler_so.remove('-Wall')
        except (AttributeError, ValueError):
            pass

    def search_libs(self, func, libraries_list, library_dirs_list,
                    headers=None, include_dirs=None):
        """Search for the libraries needed for func.

        Returns None if no set of libraries which provide func are found.

        Otherwise, returns a list of the libraries needed (which may be the
        empty list, if no libraries are needed).

        If headers is not None, it is a list of headers to include to get the
        definition of the function.  Otherwise, a definition for the function
        will be automatically generated (but this may fail to be linkable,
        particularly if the header file uses macro magic to refer to a
        different symbol).
        
        """
        log.info("Searching for library containing %s() ..." % func)
        liblist = [None, ]
        liblist.extend(libraries_list)
        libdirslist = [(), ]
        libdirslist.extend(library_dirs_list)

        for lib in liblist:
            libraries = (lib, )
            if lib is None:
                libraries = ()
            for library_dirs in libdirslist:
                decl = False
                if headers is None:
                    decl = True
                if self.check_func(func,
                                   headers=headers,
                                   include_dirs=include_dirs,
                                   libraries=libraries,
                                   library_dirs=library_dirs,
                                   decl=decl, call=True):
                    if lib is None:
                        log.info("... no additional library required for %s()" % func)
                    else:
                        log.info("... %s() is in %s" % (func, lib))
                    return (libraries, library_dirs)
        log.info("... no library found containing %s()" % func)
        return None

    def search_headers(self, header, include_dirs_list):
        """Search for a header file.

        Returns the list of include directories needed (which may be the empty
        list), or None if the header file couldn't be found.

        """
        log.info("Searching for header file \"%s()\" ..." % header)
        incdirslist = [(), ]
        incdirslist.extend(include_dirs_list)
        for include_dirs in incdirslist:
            if self.check_header(header, include_dirs=include_dirs):
                if len(include_dirs) == 0:
                    log.info("... header file \"%s()\" found" % header)
                else:
                    log.info("... header file \"%s()\" found in %s" % (header, include_dirs))
                return include_dirs
        log.info("... header file \"%s()\" not found" % header)
        return None

    def find_iconv_inputtype(self, include_dirs=None,
                             libraries=None, library_dirs=None):
        """Find the type of the input to the iconv function.

        Returns the type to use.

        """
        log.info("Checking for iconv input type ...")
        self._check_compiler()
        for t in ("char *", "const char *"):
            if self.try_link(iconv_inputtype_code % {'t': t},
                             headers=('iconv.h', ),
                             lang="c++",
                             include_dirs=include_dirs,
                             library_dirs=library_dirs,
                             libraries=libraries):
                log.info("... found iconv input type to be: " + t)
                return t
        log.info("... couldn't find input type for iconv")
        return None

    def initialize_options(self):
        du_config.initialize_options(self)
        # Disable the dumping of source by default - doesn't seem particularly
        # useful and makes the output hard to follow.  It can be reenabled by a
        # command line parameter, anyway.
        self.dump_source = 0

    def write_config_h(self):
        path = os.path.join('src', 'config.h')
        fd = open(path, "w")
        fd.write("""
/* config.h: configuration settings.
 *
 * This file is automatically generated by setup.py
 */
        """.strip())
        fd.write("\n")
        for key, (comment, val) in self.defines.iteritems():
            fd.write("\n/* %s */\n" % comment)
            if val is None:
                fd.write("#undef %s\n" % key)
            else:
                fd.write("#define %s %s\n" % (key, val))
        fd.close()

    def write_params(self):
        path = os.path.join('configutils', 'params.cache')
        fd = open(path, "wb")
        cPickle.dump(self.params, fd)
        fd.close()

    def set_define(self, key, value, comment):
        if value is False:
            value = None
        elif value is True:
            value = 1
        self.defines[key] = (comment, value)

    def run(self):
        self.defines = {}
        self.params = {}

        library_dirs_list = []
        library_dirs_list += [extra_library_dirs]
        include_dirs_list = []
        include_dirs_list += [extra_include_dirs]

        iconv_include_dirs = self.search_headers('iconv.h', include_dirs_list)
        self.params['include_dirs'] = iconv_include_dirs

        iconv_lib = self.search_libs('iconv',
                                     ('iconv', 'libiconv'),
                                     library_dirs_list,
                                     headers=('iconv.h',),
                                     include_dirs=iconv_include_dirs)
        if iconv_lib is None:
            have_iconv = False
            # Doesn't matter what type we use, but might as well be a
            # plausible type
            iconv_inputtype = "char *"
        else:
            have_iconv = True
            iconv_lib, iconv_libdirs = iconv_lib
            iconv_inputtype = self.find_iconv_inputtype(
                libraries=iconv_lib,
                library_dirs=iconv_libdirs)
            self.params['libraries'] = iconv_lib
            self.params['library_dirs'] = iconv_libdirs

        have_sys_errno_h = self.check_header("sys/errno.h")
        have_strings_h = self.check_header("strings.h")

        self.set_define("PACKAGE",
                        "htmlparser",
                        "The name of this package.")
        self.set_define("HAVE_SYS_ERRNO_H",
                        have_sys_errno_h,
                        "Define to 1 if you have the <sys/errno.h> header file.")
        self.set_define("HAVE_STRINGS_H",
                        have_strings_h,
                        "Define to 1 if you have the <strings.h> header file.")
        self.set_define("USE_ICONV",
                        have_iconv,
                        "Define to 1 if iconv() should be used for converting character sets.")
        self.set_define("ICONV_INPUT_TYPE",
                        iconv_inputtype,
                        "Type used for input to iconv.")

        self.write_config_h()
        self.write_params()
