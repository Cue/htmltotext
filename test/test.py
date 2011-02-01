#!/usr/bin/env python
#
# Copyright (C) 2007 Lemur Consulting Ltd
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
r"""test.py: Test htmltotext module.

"""

import unittest
import htmltotext

class TestHtmlToText(unittest.TestCase):

    def test_extract_body(self):
        """Test extracting the body text.

        """
        html = '<html><body><p>Hello  World</p></body></html>'
        self.assertEqual(htmltotext.extract(html).title, u'')
        self.assertEqual(htmltotext.extract(html).description, u'')
        self.assertEqual(htmltotext.extract(html).content, u'Hello World')

    def test_default_charset(self):
        """Test a conversion supplying a string, which should use the default
        character set.
        
        """
        html = '<title>foo\xa3</title>'
        self.assertEqual(htmltotext.extract(html).title, u'foo\xa3')

    def test_unicode_input(self):
        """Test supplying a unicode string as input.

        """
        html = u'<meta http-equiv="content-type" content="charset=latin1"/><title>foo\xa3</title>'
        self.assertEqual(htmltotext.extract(html).title, u'foo\xa3')

    def test_meta_content_type(self):
        """Test supplying a meta http-equiv tag to set the character set.

        """
        html = '<meta http-equiv="content-type" content="charset=utf8"/><title>foo\xc2\xa3</title>'
        self.assertEqual(htmltotext.extract(html).title, u'foo\xa3')

    def test_meta_content_type(self):
        """Test supplying a meta http-equiv tag to set the character set
        incorrectly.

        Will raise an error when trying to convert the output to unicode.

        """
        html = '<meta http-equiv="content-type" content="charset=utf8"/><title>foo\xa3</title>'
        self.assertEqual(htmltotext.extract(html).title, u'foo')
        self.assertEqual(htmltotext.extract(html).badly_encoded, True)

    def test_meta_description(self):
        """Test supplying a meta description tag.

        """
        html = '<meta name="description" content="desc"/><title>foo</title><body>body</body>'
        self.assertEqual(htmltotext.extract(html).title, u'foo')
        self.assertEqual(htmltotext.extract(html).description, u'desc')
        self.assertEqual(htmltotext.extract(html).keywords, u'')
        self.assertEqual(htmltotext.extract(html).indexing_allowed, True)
        self.assertEqual(htmltotext.extract(html).badly_encoded, False)

    def test_meta_description(self):
        """Test supplying a meta description tag.

        """
        html = '<meta name="robots" content="noindex"/><title>foo</title><body>body</body>'
        self.assertEqual(htmltotext.extract(html).title, u'')
        self.assertEqual(htmltotext.extract(html).description, u'')
        self.assertEqual(htmltotext.extract(html).keywords, u'')
        self.assertEqual(htmltotext.extract(html).indexing_allowed, False)

    def test_link1(self):
        """Test parsing some links.

        """
        html = '<title>Here it <p/>is</title><body><foo class="1" id=top>body <a href="bar">link content</a><b/><a href="/foo2">2</p><a class="foo" href="http://bar.com/foo3">3</a> end body</foo><a href="test"><i>mo<em>re</em></i><b>test</a></body>'
        parsed = htmltotext.extract(html)
        self.assertEqual(parsed.title, u'Here it is')
        self.assertEqual(parsed.description, u'')
        self.assertEqual(parsed.keywords, u'')
        self.assertEqual(parsed.indexing_allowed, True)
        self.assertEqual(parsed.content, 'body link content2 3 end bodymoretest')
        self.assertEqual(len(parsed.links), 4)

        self.assertEqual(parsed.links[0].target, u'bar')
        self.assertEqual(parsed.links[0].text, u'link content')
        self.assertEqual(parsed.links[0].para, u'body link content2')
        self.assertEqual(parsed.links[0].start_pos, 4)
        self.assertEqual(parsed.links[1].target, u'/foo2')
        self.assertEqual(parsed.links[1].text, u'2')
        self.assertEqual(parsed.links[1].para, u'body link content2')
        self.assertEqual(parsed.links[1].start_pos, 17)
        self.assertEqual(parsed.links[2].target, u'http://bar.com/foo3')
        self.assertEqual(parsed.links[2].text, u'3')
        self.assertEqual(parsed.links[2].para, u'3 end bodymoretest')
        self.assertEqual(parsed.links[2].start_pos, 18)
        self.assertEqual(parsed.links[3].target, u'test')
        self.assertEqual(parsed.links[3].text, u'moretest')
        self.assertEqual(parsed.links[3].para, u'3 end bodymoretest')
        self.assertEqual(parsed.links[3].start_pos, 29)

        self.assertEqual(len(parsed.links[0].parent_tags), 3)
        self.assertEqual(parsed.links[0].parent_tags[0].name, "body")
        self.assertEqual(parsed.links[0].parent_tags[0].cls, "")
        self.assertEqual(parsed.links[0].parent_tags[0].id, "")
        self.assertEqual(parsed.links[0].parent_tags[1].name, "foo")
        self.assertEqual(parsed.links[0].parent_tags[1].cls, "1")
        self.assertEqual(parsed.links[0].parent_tags[1].id, "top")

        self.assertEqual(parsed.links[0].parent_tags[-1].name, "a")
        self.assertEqual(parsed.links[0].parent_tags[-1].cls, "")
        self.assertEqual(parsed.links[1].parent_tags[-1].name, "a")
        self.assertEqual(parsed.links[1].parent_tags[-1].cls, "")
        self.assertEqual(parsed.links[2].parent_tags[-1].name, "a")
        self.assertEqual(parsed.links[2].parent_tags[-1].cls, "foo")
        self.assertEqual(parsed.links[3].parent_tags[-1].name, "a")
        self.assertEqual(parsed.links[3].parent_tags[-1].cls, "")

        self.assertEqual(parsed.links[0].child_tags, [])
        self.assertEqual(parsed.links[1].child_tags, [])
        self.assertEqual(parsed.links[2].child_tags, [])
        self.assertEqual(len(parsed.links[3].child_tags), 3)
        self.assertEqual(parsed.links[3].child_tags[0].name, 'i')
        self.assertEqual(parsed.links[3].child_tags[1].name, 'em')
        self.assertEqual(parsed.links[3].child_tags[2].name, 'b')

        self.assertEqual(parsed.parastarts, [0, 18, 37])

    def test_link2(self):
        """Test parsing some links interspersed with tags which don't close.

        """
        html = '<body><div><a href="a1"></a><br></div><a href="a2"></a><br><a href="a3"></a></body>'
        parsed = htmltotext.extract(html)
        self.assertEqual([t.name for t in parsed.links[0].parent_tags], ['body', 'div', 'a'])
        self.assertEqual([t.name for t in parsed.links[1].parent_tags], ['body', 'a'])
        self.assertEqual([t.name for t in parsed.links[2].parent_tags], ['body', 'a'])

def suite():
    return unittest.makeSuite(TestHtmlToText)

def test():
    runner = unittest.TextTestRunner()
    runner.run(suite())

if __name__ == '__main__':
    test()

