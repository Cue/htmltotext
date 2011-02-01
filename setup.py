#!/usr/bin/env python
#
# Copyright (C) 2007,2008 Lemur Consulting Ltd
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
"""Setup script for htmltotext extension module.

"""

import os
import sys
from configutils import Extension, setup, config, build_ext, using_setuptools

# List of source files
htmltotext_sources = [
    'src/htmlparse.cc',
    'src/metaxmlparse.cc',
    'src/myhtmlparse.cc',
    'src/pyhtmltotext.cc',
    'src/utf8convert.cc',
    'src/utf8itor.cc',
    'src/xmlparse.cc',
]

# Extra arguments for setup() which we don't always want to supply.
extra_kwargs = {}
if using_setuptools:
    extra_kwargs['test_suite'] = "test.test"

long_description = """
The htmltotext module
=====================

This package was written for a search engine, to allow it to extract the
textual content and metadata from HTML pages.  It tries to cope with
invalid markup and incorrectly specified character sets, and strips out
HTML tags (splitting words at tags appropriately).  It also discards the
contents of script tags and style tags.

As well as text from the body of the page, it extracts the page title,
and the content of meta description and keyword tags.  It also parses
meta robots tags to determine whether the page should be indexed.

The HTML parser used by this module was extracted from the Xapian search
engine library (and specifically, from the omindex indexing utility in
that library).

"""

setup(name = "htmltotext",
      version = "0.7.3",
      author = "Richard Boulton",
      author_email = "richard@lemurconsulting.com",
      maintainer = "Richard Boulton",
      maintainer_email = "richard@lemurconsulting.com",
      url = "http://code.google.com/p/flaxcode/wiki/HtmlToText",
      download_url = "http://flaxcode.googlecode.com/files/htmltotext-0.7.3.tar.gz",
      description = "Extract text and some metainfo from HTML, coping with malformed pages as well as possible.",
      long_description = long_description,
      classifiers = [
          'Development Status :: 3 - Alpha',
          'Intended Audience :: Developers',
          'License :: OSI Approved :: GNU General Public License (GPL)',
          'Programming Language :: C++',
          'Topic :: Internet :: WWW/HTTP :: Indexing/Search',
          'Operating System :: MacOS',
          'Operating System :: Microsoft',
          'Operating System :: POSIX',
      ],
      license = 'GPL',
      platforms = 'Any',

      cmdclass = {
          'config': config,
          'build_ext': build_ext,
      },
      ext_modules = [Extension("htmltotext",
                               htmltotext_sources,
                               include_dirs=['src'],
                              )],

      **extra_kwargs)
