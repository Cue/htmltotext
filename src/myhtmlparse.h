/* myhtmlparse.h: subclass of HtmlParser for extracting text
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2006 Olly Betts
 * Copyright 2007,2008 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef OMEGA_INCLUDED_MYHTMLPARSE_H
#define OMEGA_INCLUDED_MYHTMLPARSE_H

#include "htmlparse.h"
#include <vector>

// FIXME: Should we include \xa0 which is non-breaking space in iso-8859-1, but
// not in all charsets and perhaps spans of all \xa0 should become a single
// \xa0?
#define WHITESPACE " \t\n\r"

struct HtmlTag {
    // Name of the tag.
    string name;

    // Class of the tag.
    string cls;

    // Id of the tag.
    string id;

    HtmlTag() {}
    HtmlTag(const string & name_) : name(name_) {}
};

struct HtmlLink {
    // Target URL of link
    string target;

    // Text in link
    string text;

    // Paragraph text containing link
    string para;

    // Start position of link in paragraph
    size_t start_pos;

    // Parent tags of link (and also the link tag itself).
    std::vector<HtmlTag> parent_tags;

    // Child tags, in order of starting (not necessarily nested).
    std::vector<HtmlTag> child_tags;
};

class MyHtmlParser : public HtmlParser {
    public:
	bool fixed_charset;
	bool in_script_tag;
	bool in_style_tag;
	bool pending_space;
	string title, sample, keywords, dump;
	bool indexing_allowed;
	std::vector<HtmlLink *> links;
	std::vector<unsigned int> parastarts;

    private:
	std::vector<HtmlTag> tags;
	std::vector<HtmlLink*> paralinks;
	HtmlLink * currlink;
	unsigned int parastart;
	unsigned int link_text_start;
	void new_para();
	void start_dump();

    public:
	void process_text(const string &text);
	void opening_tag(const string &tag, const map<string,string> &p);
	void close_link();
	void closing_tag(const string &tag);
	void parse_html(const string &text);
	void parse_html(const string &text, const string &charset_);
	MyHtmlParser() :
		fixed_charset(false),
		in_script_tag(false),
		in_style_tag(false),
		pending_space(false),
		indexing_allowed(true),
		currlink(NULL),
		parastart(0),
		link_text_start(0)
        {
	    start_dump();
	}

	~MyHtmlParser();
};

#endif // OMEGA_INCLUDED_MYHTMLPARSE_H
