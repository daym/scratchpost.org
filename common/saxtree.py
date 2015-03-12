#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import with_statement
import xml.sax.saxutils as saxutils # escape (simple XML escape), unescape, quoteattr, XMLGenerator(out, encoding = "UTF-8")
import xml.sax.xmlreader as xmlreader
import xml.sax
attr0 = xmlreader.AttributesImpl({})
out = None
doc = None
#nodes = []
class XMLGenerator(saxutils.XMLGenerator):
	def __init__(self, out, *args, **kwargs):
		saxutils.XMLGenerator.__init__(self, out, encoding = "UTF-8") # *args, **kwargs)
		self._out = out # do NOT mess with the damn encoding there
def init(out1):
	global out
	global doc
	out = out1
	doc = XMLGenerator(out)
	doc.processingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"")
	#<html><body><p>hello</p></body></html>

class Document(object):
	@classmethod
	def __enter__(klass):
		doc.startDocument()
	@classmethod
	def __exit__(klass, ty, val, tb):
		doc.endDocument()
class Node(object):
	@classmethod
	def __enter__(klass):
		pass
	@classmethod
	def __exit__(klass, ty, val, tb):
		pass

def enc(v):
	if isinstance(v, unicode):
		return(v.encode("utf-8"))
	else:
		return(v)
class Element(Node):
	def __init__(self, **attrs):
		self.attrs = attrs # dict([(enc(k),enc(v)) for k, v in attrs.items()])
		#self.children = children
	def __enter__(self):
		attr1 = xmlreader.AttributesImpl(self.attrs)
		doc.startElement(self.__class__.__name__.lower(), attr1)
	def __exit__(self, ty, val, tb):
		doc.endElement(self.__class__.__name__.lower())
class Text(Node):
	def __init__(self, value):
		self.value = value # enc(value)
	def __enter__(self):
		doc.characters(self.value) # .decode("utf-8"))
	def __exit__(self, ty, val, tb):
		pass
	
class HTML(Element):
	pass
class HEAD(Element):
	pass
class BODY(Element):
	pass
class P(Element):
	pass
class DIV(Element):
	pass
class NAV(Element):
	pass
class UL(Element):
	pass
class LI(Element):
	pass
class A(Element):
	pass
class TABLE(Element):
	pass
class TR(Element):
	pass
class TD(Element):
	pass
class TH(Element):
	pass
class META(Element):
	pass
class LINK(Element):
	pass
class SCRIPT(Element):
	pass
class IMG(Element):
	pass
class BR(Element): # clear
	pass
class OBJECT(Element):
	pass
class EM(Element):
	pass
class Deunicodizer(object):
	def __init__(self, doc):
		self.doc = doc
	def processingInstruction(self, name, value):
		return self.doc.processingInstruction(enc(name), enc(value))
	def characters(self, text):
		return self.doc.characters(enc(text))
	def setDocumentLocator(self, *args, **kwargs):
		self.doc.setDocumentLocator(*args, **kwargs)
	def startDocument(self):
		self.doc.startDocument()
	def startElement(self, name, attrs):
		attrs = dict([(enc(k),enc(v)) for k, v in attrs.items()])
		name = enc(name)
		self.doc.startElement(name, attrs)
	def endElement(self, name):
		self.doc.endElement(enc(name))
	def endDocument(self):
		self.doc.endDocument()

def copyFrom(f):
	xml.sax.parse(f, Deunicodizer(doc))
if __name__ == "__main__":
	import StringIO
	io = StringIO.StringIO()
	init(io)
	with HTML():
		with BODY():
			with P():
				with Text("hällo"):
					pass
	print(io.getvalue())
	assert(io.getvalue() == """<?xml version="1.0" encoding="utf-8"?><html><body><p>hällo</p></body></html>""")
	io.seek(0)
	io2 = StringIO.StringIO()
	doc = XMLGenerator(io2)
	copyFrom(io)
	assert(io2.getvalue() == """<?xml version="1.0" encoding="UTF-8"?>
<html><body><p>hällo</p></body></html>""")
