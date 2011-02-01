/* pyhtmltotext.cc: Python interface for htmltotext.
 *
 * Copyright (C) 2007,2008 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>
#include <Python.h>
#include "structmember.h"
#include "myhtmlparse.h"

/* Python object used to represent a link. */
typedef struct {
    PyObject_HEAD
    PyObject *name;
    PyObject *cls;
    PyObject *id;
} PyHtmlTag;

static void
PyHtmlTag_dealloc(PyHtmlTag * self)
{
    Py_XDECREF(self->name);
    Py_XDECREF(self->cls);
    Py_XDECREF(self->id);
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
PyHtmlTag_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyHtmlTag *self;

    self = (PyHtmlTag *)type->tp_alloc(type, 0);
    if (self != NULL) {
	/* Initialise any fields to default values here. */
    }

    return (PyObject *)self;
}

static PyMemberDef PyHtmlTag_members[] = {
    {"name", T_OBJECT_EX,
	offsetof(PyHtmlTag, name), 0,
	"Name of the tag."},
    {"cls", T_OBJECT_EX,
	offsetof(PyHtmlTag, cls), 0,
	"Class of the tag."},
    {"id", T_OBJECT_EX,
	offsetof(PyHtmlTag, id), 0,
	"id of the tag."},
    {NULL}  /* Sentinel */
};

static PyObject *
PyHtmlTag_str(PyHtmlTag * link)
{
    PyObject * result = NULL;
    PyObject * args = NULL;
    PyObject * format = NULL;
    PyObject * empty = NULL;
    const char * formatstr = "PyHtmlTag(name=%r, cls=%r, id=%r)";
    
    format = PyUnicode_Decode(formatstr, strlen(formatstr), "utf-8", NULL);
    if (format == NULL) goto fail;

    args = PyTuple_New(3);
    if (args == NULL) goto fail;

    // Don't use a zero size array here, because MSVC complains about it.
    Py_UNICODE emptystring[1];
    empty = PyUnicode_FromUnicode(emptystring, 0);
    if (empty == NULL) goto fail;

    if (link->name == NULL) {
	Py_INCREF(empty);
	PyTuple_SET_ITEM(args, 0, empty);
    } else {
	Py_INCREF(link->name);
	PyTuple_SET_ITEM(args, 0, link->name);
    }

    if (link->cls == NULL) {
	Py_INCREF(empty);
	PyTuple_SET_ITEM(args, 1, empty);
    } else {
	Py_INCREF(link->cls);
	PyTuple_SET_ITEM(args, 1, link->cls);
    }

    if (link->id == NULL) {
	Py_INCREF(empty);
	PyTuple_SET_ITEM(args, 2, empty);
    } else {
	Py_INCREF(link->id);
	PyTuple_SET_ITEM(args, 2, link->id);
    }

    result = PyUnicode_Format(format, args);

fail:
    Py_XDECREF(format);
    Py_XDECREF(args);
    Py_XDECREF(empty);
    return result;
}

static PyTypeObject PyHtmlTagType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "htmltotext.PyHtmlTag",   /*tp_name*/
    sizeof(PyHtmlTag), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyHtmlTag_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    (reprfunc)PyHtmlTag_str,  /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "A link in a parsed page", /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    0,                         /* tp_methods */
    PyHtmlTag_members,        /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                         /* tp_init */
    0,                         /* tp_alloc */
    PyHtmlTag_new,            /* tp_new */
};

static void
PyHtmlTag_ready()
{
    if (PyType_Ready(&PyHtmlTagType) < 0)
	return;
}

static void
PyHtmlTag_register(PyObject * m)
{
    Py_INCREF(&PyHtmlTagType);
    PyModule_AddObject(m, "PyHtmlTag", (PyObject *)&PyHtmlTagType);
}


/* Python object used to represent a link. */
typedef struct {
    PyObject_HEAD
    PyObject *target;
    PyObject *text;
    PyObject *para;
    PyObject *start_pos;
    PyObject *parent_tags;
    PyObject *child_tags;
} PyHtmlLink;

static void
PyHtmlLink_dealloc(PyHtmlLink * self)
{
    Py_XDECREF(self->target);
    Py_XDECREF(self->text);
    Py_XDECREF(self->para);
    Py_XDECREF(self->start_pos);
    Py_XDECREF(self->parent_tags);
    Py_XDECREF(self->child_tags);
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
PyHtmlLink_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyHtmlLink *self;

    self = (PyHtmlLink *)type->tp_alloc(type, 0);
    if (self != NULL) {
	/* Initialise any fields to default values here. */
    }

    return (PyObject *)self;
}

static PyMemberDef PyHtmlLink_members[] = {
    {"target", T_OBJECT_EX,
	offsetof(PyHtmlLink, target), 0,
	"Target of the link."},
    {"text", T_OBJECT_EX,
	offsetof(PyHtmlLink, text), 0,
	"Text of the link."},
    {"para", T_OBJECT_EX,
	offsetof(PyHtmlLink, para), 0,
	"Text of paragraph containing the link."},
    {"start_pos", T_OBJECT_EX,
	offsetof(PyHtmlLink, start_pos), 0,
	"Text of start_pos containing the link."},
    {"parent_tags", T_OBJECT_EX,
	offsetof(PyHtmlLink, parent_tags), 0,
	"List of parent tags of the link, oldest ancestor first."},
    {"child_tags", T_OBJECT_EX,
	offsetof(PyHtmlLink, child_tags), 0,
	"List of child tags of the link, oldest ancestor first."},
    {NULL}  /* Sentinel */
};

static PyObject *
PyHtmlLink_str(PyHtmlLink * link)
{
    PyObject * result = NULL;
    PyObject * args = NULL;
    PyObject * format = NULL;
    PyObject * empty = NULL;
    const char * formatstr = "PyHtmlLink(target=%r, text=%r, para=%r, start_pos=%s)";
    
    format = PyUnicode_Decode(formatstr, strlen(formatstr), "utf-8", NULL);
    if (format == NULL) goto fail;

    args = PyTuple_New(4);
    if (args == NULL) goto fail;

    // Don't use a zero size array here, because MSVC complains about it.
    Py_UNICODE emptystring[1];
    empty = PyUnicode_FromUnicode(emptystring, 0);
    if (empty == NULL) goto fail;

    if (link->target == NULL) {
	Py_INCREF(empty);
	PyTuple_SET_ITEM(args, 0, empty);
    } else {
	Py_INCREF(link->target);
	PyTuple_SET_ITEM(args, 0, link->target);
    }

    if (link->text == NULL) {
	Py_INCREF(empty);
	PyTuple_SET_ITEM(args, 1, empty);
    } else {
	Py_INCREF(link->text);
	PyTuple_SET_ITEM(args, 1, link->text);
    }

    if (link->para == NULL) {
	Py_INCREF(empty);
	PyTuple_SET_ITEM(args, 2, empty);
    } else {
	Py_INCREF(link->para);
	PyTuple_SET_ITEM(args, 2, link->para);
    }

    if (link->start_pos == NULL) {
	Py_INCREF(empty);
	PyTuple_SET_ITEM(args, 3, empty);
    } else {
	Py_INCREF(link->para);
	PyTuple_SET_ITEM(args, 3, link->start_pos);
    }

    result = PyUnicode_Format(format, args);

fail:
    Py_XDECREF(format);
    Py_XDECREF(args);
    Py_XDECREF(empty);
    return result;
}

static PyTypeObject PyHtmlLinkType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "htmltotext.PyHtmlLink",   /*tp_name*/
    sizeof(PyHtmlLink), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyHtmlLink_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    (reprfunc)PyHtmlLink_str,  /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "A link in a parsed page", /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    0,                         /* tp_methods */
    PyHtmlLink_members,        /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                         /* tp_init */
    0,                         /* tp_alloc */
    PyHtmlLink_new,            /* tp_new */
};

static void
PyHtmlLink_ready()
{
    if (PyType_Ready(&PyHtmlLinkType) < 0)
	return;
}

static void
PyHtmlLink_register(PyObject * m)
{
    Py_INCREF(&PyHtmlLinkType);
    PyModule_AddObject(m, "PyHtmlLink", (PyObject *)&PyHtmlLinkType);
}


/* Python object used to represent the results of parsing a page. */
typedef struct {
    PyObject_HEAD
    PyObject *indexing_allowed;
    PyObject *badly_encoded;
    PyObject *title;
    PyObject *content;
    PyObject *description;
    PyObject *keywords;
    PyObject *links;
    PyObject *parastarts;
} ParsedPage;

static void
ParsedPage_dealloc(ParsedPage * self)
{
    Py_XDECREF(self->indexing_allowed);
    Py_XDECREF(self->badly_encoded);
    Py_XDECREF(self->title);
    Py_XDECREF(self->content);
    Py_XDECREF(self->description);
    Py_XDECREF(self->keywords);
    Py_XDECREF(self->links);
    Py_XDECREF(self->parastarts);
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
ParsedPage_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    ParsedPage *self;

    self = (ParsedPage *)type->tp_alloc(type, 0);
    if (self != NULL) {
	/* Initialise any fields to default values here. */
	Py_INCREF(Py_True);
	self->indexing_allowed = Py_True;

	Py_INCREF(Py_False);
	self->badly_encoded = Py_False;
    }

    return (PyObject *)self;
}

static PyMemberDef ParsedPage_members[] = {
    {"indexing_allowed", T_OBJECT_EX,
	offsetof(ParsedPage, indexing_allowed), 0,
	"Boolean flag, set to true if indexing the document is allowed\n"
	"(based on meta tags)."},
    {"badly_encoded", T_OBJECT_EX,
	offsetof(ParsedPage, badly_encoded), 0,
	"Boolean flag, set to true if badly encoded data was found in the\n"
	"page."},
    {"title", T_OBJECT_EX,
	offsetof(ParsedPage, title), 0,
	"The title of the document."},
    {"content", T_OBJECT_EX,
	offsetof(ParsedPage, content), 0,
	"Text from the document body."},
    {"description", T_OBJECT_EX,
	offsetof(ParsedPage, description), 0,
	"Description for the document (based on meta tags)."},
    {"keywords", T_OBJECT_EX,
	offsetof(ParsedPage, keywords), 0,
	"Keywords for the document (based on meta tags)."},
    {"links", T_OBJECT_EX,
	offsetof(ParsedPage, links), 0,
	"Links found in the document (together with associated info)."},
    {"parastarts", T_OBJECT_EX,
	offsetof(ParsedPage, parastarts), 0,
	"Start positions of paragraphs in the document."},
    {NULL}  /* Sentinel */
};

static PyObject *
ParsedPage_str(ParsedPage * parsedpage)
{
    PyObject * result = NULL;
    PyObject * args = NULL;
    PyObject * format = NULL;
    PyObject * empty = NULL;
    const char * formatstr = "ParsedPage(title=%r, content=%r, description=%r, keywords=%r)";
    
    format = PyUnicode_Decode(formatstr, strlen(formatstr), "utf-8", NULL);
    if (format == NULL) goto fail;

    args = PyTuple_New(4);
    if (args == NULL) goto fail;

    // Don't use a zero size array here, because MSVC complains about it.
    Py_UNICODE emptystring[1];
    empty = PyUnicode_FromUnicode(emptystring, 0);
    if (empty == NULL) goto fail;

    if (parsedpage->title == NULL) {
	Py_INCREF(empty);
	PyTuple_SET_ITEM(args, 0, empty);
    } else {
	Py_INCREF(parsedpage->title);
	PyTuple_SET_ITEM(args, 0, parsedpage->title);
    }

    if (parsedpage->content == NULL) {
	Py_INCREF(empty);
	PyTuple_SET_ITEM(args, 1, empty);
    } else {
	Py_INCREF(parsedpage->content);
	PyTuple_SET_ITEM(args, 1, parsedpage->content);
    }

    if (parsedpage->description == NULL) {
	Py_INCREF(empty);
	PyTuple_SET_ITEM(args, 2, empty);
    } else {
	Py_INCREF(parsedpage->description);
	PyTuple_SET_ITEM(args, 2, parsedpage->description);
    }

    if (parsedpage->keywords == NULL) {
	Py_INCREF(empty);
	PyTuple_SET_ITEM(args, 3, empty);
    } else {
	Py_INCREF(parsedpage->keywords);
	PyTuple_SET_ITEM(args, 3, parsedpage->keywords);
    }

    result = PyUnicode_Format(format, args);

fail:
    Py_XDECREF(format);
    Py_XDECREF(args);
    Py_XDECREF(empty);
    return result;
}

static PyTypeObject ParsedPageType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "htmltotext.ParsedPage",   /*tp_name*/
    sizeof(ParsedPage), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)ParsedPage_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    (reprfunc)ParsedPage_str,  /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "A parsed page",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    0,                         /* tp_methods */
    ParsedPage_members,        /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                         /* tp_init */
    0,                         /* tp_alloc */
    ParsedPage_new,            /* tp_new */
};

static void
ParsedPage_ready()
{
    if (PyType_Ready(&ParsedPageType) < 0)
	return;
}

static void
ParsedPage_register(PyObject * m)
{
    Py_INCREF(&ParsedPageType);
    PyModule_AddObject(m, "ParsedPage", (PyObject *)&ParsedPageType);
}

/* Functions */

static PyObject *
decode_utf8_noting_errors(const std::string & data,
			  PyObject ** badly_encoded_ptr)
{
    PyObject * result;
    if (*badly_encoded_ptr == Py_False) {
	result = PyUnicode_Decode(data.data(), data.size(), "UTF-8", "strict");
	if (result == NULL) {
	    PyErr_Clear();
	    Py_XDECREF(*badly_encoded_ptr);
	    Py_INCREF(Py_True);
	    *badly_encoded_ptr = Py_True;
	} else {
	    return result;
	}
    }
    result = PyUnicode_Decode(data.data(), data.size(), "UTF-8", "ignore");
    return result;
}

static PyObject *
extract(PyObject *self, PyObject *args)
{
    ParsedPage * result = NULL;
    PyObject * arg1 = NULL;
    PyHtmlLink * link = NULL;
    PyHtmlTag * tag = NULL;

    MyHtmlParser parser;

    if (!PyArg_UnpackTuple(args, "extract", 1, 1, &arg1))
	return NULL;

    if (PyUnicode_Check(arg1)) {
	/* Convert to a UTF8 string (return value needs to be DECREFed). */
	PyObject * utf8 = PyUnicode_AsUTF8String(arg1);
	if (utf8 == NULL)
	    goto fail;

	try {
	    parser.parse_html(std::string(PyString_AS_STRING(utf8),
					  PyString_GET_SIZE(utf8)),
			      std::string("UTF-8"));
	} catch(bool) {
	} catch(...) {
	    Py_DECREF(utf8);
	    throw;
	}
	Py_DECREF(utf8);
    } else {
	const char * buffer = NULL;
	int buffer_length = 0;
	if (!PyArg_ParseTuple(args, "s#", &buffer, &buffer_length))
	{
	    goto fail;
	}

	try {
	    parser.parse_html(std::string(buffer, buffer_length));
	} catch(bool) {
	}
    }

    result = (ParsedPage*) ParsedPage_new(&ParsedPageType, NULL, NULL);
    if (result == NULL) goto fail;
    Py_XDECREF(result->indexing_allowed);
    if (parser.indexing_allowed) {
	Py_INCREF(Py_True);
	result->indexing_allowed = Py_True;
    } else {
	Py_INCREF(Py_False);
	result->indexing_allowed = Py_False;
    }

    // Set the other members to unicode strings
    Py_XDECREF(result->title);
    result->title = decode_utf8_noting_errors(parser.title,
					      &(result->badly_encoded));
    if (result->title == NULL) goto fail;

    Py_XDECREF(result->content);
    result->content = PyUnicode_Decode(parser.dump.data(),
				       parser.dump.size(),
				       "UTF-8", "replace");
    if (result->content == NULL) goto fail;

    Py_XDECREF(result->description);
    result->description = PyUnicode_Decode(parser.sample.data(),
					   parser.sample.size(),
					   "UTF-8", "replace");
    if (result->description == NULL) goto fail;

    Py_XDECREF(result->keywords);
    result->keywords = PyUnicode_Decode(parser.keywords.data(),
					parser.keywords.size(),
					"UTF-8", "replace");
    if (result->keywords == NULL) goto fail;

    Py_XDECREF(result->links);
    result->links = PyList_New(parser.links.size());
    if (result->links == NULL) goto fail;
    {
	std::vector<HtmlLink*>::const_iterator i;
	Py_ssize_t pos = 0;
	for (i = parser.links.begin(); i != parser.links.end(); ++i, ++pos)
	{
	    link = (PyHtmlLink *) PyHtmlLink_new(
		&PyHtmlLinkType, NULL, NULL);
	    if (link == NULL) goto fail;
	    link->target = PyUnicode_Decode((*i)->target.data(),
					    (*i)->target.size(),
					    "UTF-8", "replace");
	    if (link->target == NULL) goto fail;

	    link->text = PyUnicode_Decode((*i)->text.data(),
					  (*i)->text.size(),
					  "UTF-8", "replace");
	    if (link->text == NULL) goto fail;

	    link->para = PyUnicode_Decode((*i)->para.data(),
					  (*i)->para.size(),
					  "UTF-8", "replace");
	    if (link->para == NULL) goto fail;

	    link->start_pos = PyInt_FromLong((*i)->start_pos);
	    if (link->start_pos == NULL) goto fail;

	    {
		Py_XDECREF(link->parent_tags);
		link->parent_tags = PyList_New((*i)->parent_tags.size());
		std::vector<HtmlTag>::const_iterator j;
		Py_ssize_t tagpos = 0;
		for (j = (*i)->parent_tags.begin(); j != (*i)->parent_tags.end();
		     ++j, ++tagpos) {

		    tag = (PyHtmlTag *) PyHtmlTag_new(
			&PyHtmlTagType, NULL, NULL);
		    if (tag == NULL) goto fail;

		    tag->name = PyUnicode_Decode(j->name.data(),
						 j->name.size(),
						 "UTF-8", "replace");
		    if (tag->name == NULL) goto fail;

		    tag->cls = PyUnicode_Decode(j->cls.data(),
						j->cls.size(),
						"UTF-8", "replace");
		    if (tag->cls == NULL) goto fail;

		    tag->id = PyUnicode_Decode(j->id.data(),
					       j->id.size(),
					       "UTF-8", "replace");
		    if (tag->id == NULL) goto fail;

		    PyList_SET_ITEM(link->parent_tags, tagpos, (PyObject *)tag);
		    tag = NULL;
		}
	    }

	    {
		Py_XDECREF(link->child_tags);
		link->child_tags = PyList_New((*i)->child_tags.size());
		std::vector<HtmlTag>::const_iterator j;
		Py_ssize_t tagpos = 0;
		for (j = (*i)->child_tags.begin(); j != (*i)->child_tags.end();
		     ++j, ++tagpos) {

		    tag = (PyHtmlTag *) PyHtmlTag_new(
			&PyHtmlTagType, NULL, NULL);
		    if (tag == NULL) goto fail;

		    tag->name = PyUnicode_Decode(j->name.data(),
						 j->name.size(),
						 "UTF-8", "replace");
		    if (tag->name == NULL) goto fail;

		    tag->cls = PyUnicode_Decode(j->cls.data(),
						j->cls.size(),
						"UTF-8", "replace");
		    if (tag->cls == NULL) goto fail;

		    tag->id = PyUnicode_Decode(j->id.data(),
					       j->id.size(),
					       "UTF-8", "replace");
		    if (tag->id == NULL) goto fail;

		    PyList_SET_ITEM(link->child_tags, tagpos, (PyObject *)tag);
		    tag = NULL;
		}
	    }

	    PyList_SET_ITEM(result->links, pos, (PyObject *)link);
	    link = NULL;
	}
    }

    Py_XDECREF(result->parastarts);
    result->parastarts = PyList_New(parser.parastarts.size());
    if (result->parastarts == NULL) goto fail;
    {
	std::vector<unsigned int>::const_iterator i;
	Py_ssize_t pos = 0;
	for (i = parser.parastarts.begin(); i != parser.parastarts.end(); ++i, ++pos)
	{
	    PyObject * item = PyInt_FromLong(*i);
	    if (item == NULL) goto fail;
	    PyList_SET_ITEM(result->parastarts, pos, item);
	}
    }

    return (PyObject*) result;
fail:
    Py_XDECREF(result);
    Py_XDECREF(link);
    Py_XDECREF(tag);
    return NULL;
}

static PyMethodDef HtmlToTextMethods[] = {
    {"extract", extract, METH_VARARGS,
     "Extract text from a string containing some HTML.\n\n"
     "This takes a single argument, which should be a string type.\n"
     "The return value is a ParsedPage object.\n\n"
     "If the argument is a Unicode object, any character set information\n"
     "in the HTML string (eg, in <meta http-equiv=...> tags) will be\n"
     "ignored.  If the argument is a string object, such information will\n"
     "be used to determine the character encoding of the HTML, with a\n"
     "default of the Latin-1 (ISO-8859-1) being assumed (since this is the\n"
     "standard (but deprecated) default for HTML).  The badly_encoded\n"
     "member of the resulting ParsedPage object will be set to True if any\n"
     "invalid character encodings are found, but a best effort to ignore\n"
     "such errors and continue will be made."
    },
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
inithtmltotext(void)
{
    PyObject * m;

    PyHtmlTag_ready();
    PyHtmlLink_ready();
    ParsedPage_ready();

    m = Py_InitModule3("htmltotext", HtmlToTextMethods,
		       "Extract text from HTML documents.");

    PyHtmlTag_register(m);
    PyHtmlLink_register(m);
    ParsedPage_register(m);
}
