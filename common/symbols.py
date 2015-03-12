#!/usr/bin/env python

class Symbol(object):
	def __init__(self, text):
		self.text = text
	def __str__(self):
		return(self.text)
	def __repr__(self):
		return(str(self))
symbols = {}
def intern(name):
	if name in symbols:
		return symbols[name]
	symbols[name] = Symbol(name)
	return symbols[name]

if __name__ == "__main__":
	assert(intern("foo") is intern("foo"))
	assert(intern("foo") is not intern("bar"))
