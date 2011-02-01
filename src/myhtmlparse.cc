/* myhtmlparse.cc: subclass of HtmlParser for extracting text.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2006,2007 Olly Betts
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

#include <config.h>

#include "myhtmlparse.h"

#include <ctype.h>
#include <string.h>

inline void
lowercase_string(string &str)
{
    for (string::iterator i = str.begin(); i != str.end(); ++i) {
	*i = tolower(static_cast<unsigned char>(*i));
    }
}

MyHtmlParser::~MyHtmlParser()
{
    std::vector<HtmlLink*>::const_iterator i;
    for (i = links.begin(); i != links.end(); ++i)
	delete *i;
}

void
MyHtmlParser::parse_html(const string &text)
{
    // Default HTML character set is latin 1, though not specifying one is
    // deprecated these days.
    charset = "ISO-8859-1";
    fixed_charset = false;
    try {
	HtmlParser::parse_html(text);
    } catch(bool) {
    }
    new_para();
    int i;
    for (i = tags.size() - 1; i >= 0; --i) {
	if (tags[i].name == "a") close_link();
    }
}

void
MyHtmlParser::parse_html(const string &text, const string &charset_)
{
    charset = charset_;
    fixed_charset = true;
    try{
	HtmlParser::parse_html(text);
    } catch(bool) {
    }
    new_para();
    int i;
    for (i = tags.size() - 1; i >= 0; --i) {
	if (tags[i].name == "a") close_link();
    }
}

void
MyHtmlParser::process_text(const string &text)
{
    if (!text.empty() && !in_script_tag && !in_style_tag) {
	string::size_type b = text.find_first_not_of(WHITESPACE);
	if (b) pending_space = true;
	while (b != string::npos) {
	    if (pending_space && !dump.empty()) {
		if (parastart == dump.size())
		    parastart += 1;
		if (link_text_start == dump.size())
		    link_text_start += 1;
		dump += ' ';
	    }
	    string::size_type e = text.find_first_of(WHITESPACE, b);
	    pending_space = (e != string::npos);
	    if (!pending_space) {
		dump.append(text.data() + b, text.size() - b);
		return;
	    }
	    dump.append(text.data() + b, e - b);
	    b = text.find_first_not_of(WHITESPACE, e + 1);
	}
    }
}

bool
startswith(const string & s, const string & p)
{
    return (s.size() >= p.size() &&
	    memcmp(s.data(), p.data(), p.size()) == 0);
}

void
MyHtmlParser::new_para()
{
    pending_space = true;
    std::string paratext = dump.substr(parastart);
    std::vector<HtmlLink*>::const_iterator i;
    for (i = paralinks.begin(); i != paralinks.end(); ++i) {
	(*i)->para = paratext;
    }
    paralinks.clear();

    parastart = dump.size();
    parastarts.push_back(parastart);
}

void
MyHtmlParser::start_dump()
{
    dump = "";
    parastart = 0;
    parastarts.clear();
    parastarts.push_back(parastart);
}

bool
get_autocloses(const string & tag)
{
    // Set to true if the tag doesn't expect a matching close tag (like br)
    switch (tag[0]) {
	case 'a':
	    if (tag == "area") return true;
	    break;
	case 'b':
	    if (tag == "base" ||
		tag == "basefont" ||
		tag == "br") return true;
	    break;
	case 'c':
	    if (tag == "col") return true;
	    break;
	case 'f':
	    if (tag == "frame") return true;
	    break;
	case 'h':
	    if (tag == "hr") return true;
	    break;
	case 'i':
	    if (tag == "img" ||
		tag == "input" ||
		tag == "isindex") return true;
	    break;
	case 'l':
	    if (tag == "link") return true;
	    break;
	case 'm':
	    if (tag == "meta") return true;
	    break;
	case 'p':
	    if (tag == "param") return true;
	    break;
    }
    return false;
}

void
MyHtmlParser::opening_tag(const string &tag, const map<string,string> &p)
{
#if 0
    cout << "<" << tag;
    map<string, string>::const_iterator x;
    for (x = p.begin(); x != p.end(); x++) {
	cout << " " << x->first << "=\"" << x->second << "\"";
    }
    cout << ">\n";
#endif
    if (tag.empty()) return;

    HtmlTag htmltag(tag);
    {
	map<string, string>::const_iterator i;
	if ((i = p.find("class")) != p.end()) {
	    htmltag.cls = i->second;
	}
	if ((i = p.find("id")) != p.end()) {
	    htmltag.id = i->second;
	}
    }
    if (!get_autocloses(tag)) {
	tags.push_back(htmltag);
    }
    if (currlink != NULL && tag != "a") {
	currlink->child_tags.push_back(htmltag);
    }
    switch (tag[0]) {
	case 'a':
	    if (tag == "a") {
		close_link();
		HtmlLink * link = new HtmlLink;
		links.push_back(link);
		paralinks.push_back(link);

		map<string, string>::const_iterator i;
		if ((i = p.find("href")) != p.end()) {
		    link->target = i->second;
		}
		link->parent_tags = tags;
		link_text_start = dump.size();
		link->start_pos = link_text_start;
		currlink = link;
	    }
	    if (tag == "address") new_para();
	    break;
	case 'b':
	    if (tag == "body") {
		new_para();
		start_dump();
		break;
	    }
	    if (tag == "blockquote" || tag == "br") new_para();
	    break;
	case 'c':
	    if (tag == "center") new_para();
	    break;
	case 'd':
	    if (tag == "dd" || tag == "dir" || tag == "div" || tag == "dl" ||
		tag == "dt") new_para();
	    break;
	case 'e':
	    if (tag == "embed") new_para();
	    break;
	case 'f':
	    if (tag == "fieldset" || tag == "form") new_para();
	    break;
	case 'h':
	    // hr, and h1, ..., h6
	    if (tag.length() == 2 && strchr("r123456", tag[1]))
		new_para();
	    break;
	case 'i':
	    if (tag == "iframe" || tag == "img" || tag == "isindex" ||
		tag == "input") new_para();
	    break;
	case 'k':
	    if (tag == "keygen") new_para();
	    break;
	case 'l':
	    if (tag == "legend" || tag == "li" || tag == "listing")
		new_para();
	    break;
	case 'm':
	    if (tag == "meta") {
		map<string, string>::const_iterator i, j;
		if ((i = p.find("content")) != p.end()) {
		    if ((j = p.find("name")) != p.end()) {
			string name = j->second;
			lowercase_string(name);
			if (name == "description") {
			    if (sample.empty()) {
				sample = i->second;
				decode_entities(sample);
			    }
			} else if (name == "keywords") {
			    if (!keywords.empty()) keywords += ' ';
			    string tmp = i->second;
			    decode_entities(tmp);
			    keywords += tmp;
			} else if (name == "robots") {
			    string val = i->second;
			    decode_entities(val);
			    lowercase_string(val);
			    if (val.find("none") != string::npos ||
				val.find("noindex") != string::npos) {
				indexing_allowed = false;
				throw true;
			    }
			}
		    }
		    if ((j = p.find("http-equiv")) != p.end()) {
			string hdr = j->second;
			lowercase_string(hdr);
			if (hdr == "content-type") {
			    if (!fixed_charset) {
				string value = i->second;
				lowercase_string(value);
				size_t start = value.find("charset=");
				if (start == string::npos) break;
				start += 8;
				if (start == value.size()) break;
				size_t end = start;
				if (value[start] != '"') {
				    while (end < value.size()) {
					unsigned char ch = value[end];
					if (ch <= 32 || ch >= 127 ||
					    strchr(";()<>@,:\\\"/[]?={}", ch))
					    break;
					++end;
				    }
				} else {
				    ++start;
				    ++end;
				    while (end < value.size()) {
					unsigned char ch = value[end];
					if (ch == '"') break;
					if (ch == '\\') value.erase(end, 1);
					++end;
				    }
				}
				charset = value.substr(start, end - start);
			    }
			}
		    }
		}
		break;
	    }
	    if (tag == "marquee" || tag == "menu" || tag == "multicol")
		new_para();
	    break;
	case 'o':
	    if (tag == "ol" || tag == "option") new_para();
	    break;
	case 'p':
	    if (tag == "p" || tag == "pre" || tag == "plaintext")
		new_para();
	    break;
	case 'q':
	    if (tag == "q") new_para();
	    break;
	case 's':
	    if (tag == "style") {
		in_style_tag = true;
		break;
	    }
	    if (tag == "script") {
		in_script_tag = true;
		break;
	    }
	    if (tag == "select") new_para();
	    break;
	case 't':
	    if (tag == "table" || tag == "td" || tag == "textarea" ||
		tag == "th") new_para();
	    break;
	case 'u':
	    if (tag == "ul") new_para();
	    break;
	case 'x':
	    if (tag == "xmp") new_para();
	    break;
    }
}

void
MyHtmlParser::close_link()
{
    if (currlink == NULL) return;
    map<string, string>::const_iterator i;
    if (links.size() == 0)
	return;
    HtmlLink * link = links[links.size() - 1];
    if (dump.size() > link_text_start) {
	link->text = dump.substr(link_text_start);
    }
    currlink = NULL;
}

void
MyHtmlParser::closing_tag(const string &tag)
{
    if (tag.empty()) return;
    int i;
    for (i = tags.size() - 1; i >= 0; --i) {
	if (tags[i].name == tag) {
	    if (currlink != NULL) {
		for (int j = tags.size() - 1; j >= i; --j) {
		    if (tags[j].name == "a") close_link();
		}
	    }
	    tags.resize(i);
	    break;
	}
    }
    switch (tag[0]) {
	case 'a':
	    if (tag == "a") close_link();
	    if (tag == "address") new_para();
	    break;
	case 'b':
	    if (tag == "body") {
		throw true;
	    }
	    if (tag == "blockquote" || tag == "br") new_para();
	    break;
	case 'c':
	    if (tag == "center") new_para();
	    break;
	case 'd':
	    if (tag == "dd" || tag == "dir" || tag == "div" || tag == "dl" ||
		tag == "dt") new_para();
	    break;
	case 'f':
	    if (tag == "fieldset" || tag == "form") new_para();
	    break;
	case 'h':
	    // hr, and h1, ..., h6
	    if (tag.length() == 2 && strchr("r123456", tag[1]))
		new_para();
	    break;
	case 'i':
	    if (tag == "iframe") new_para();
	    break;
	case 'l':
	    if (tag == "legend" || tag == "li" || tag == "listing")
		new_para();
	    break;
	case 'm':
	    if (tag == "marquee" || tag == "menu") new_para();
	    break;
	case 'o':
	    if (tag == "ol" || tag == "option") new_para();
	    break;
	case 'p':
	    if (tag == "p" || tag == "pre") new_para();
	    break;
	case 'q':
	    if (tag == "q") new_para();
	    break;
	case 's':
	    if (tag == "style") {
		in_style_tag = false;
		break;
	    }
	    if (tag == "script") {
		in_script_tag = false;
		break;
	    }
	    if (tag == "select") new_para();
	    break;
	case 't':
	    if (tag == "title") {
		if (title.empty()) {
		    title = dump;
		    start_dump();
		}
		break;
	    }
	    if (tag == "table" || tag == "td" || tag == "textarea" ||
		tag == "th") new_para();
	    break;
	case 'u':
	    if (tag == "ul") new_para();
	    break;
	case 'x':
	    if (tag == "xmp") new_para();
	    break;
    }
}
