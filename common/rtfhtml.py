#!/usr/bin/env python

import symbols
import StringIO as io
import sys

def controlWordP(input):
	return input and input in "abcdefghijklmnopqrstuvwxyz"
class Scanner(object):
	def __init__(self):
		self.inputStream = None
		self.input = None
		self.inputValue = None
	def push(self, inputStream):
		self.inputStream = inputStream
	def pop(self):
		pass
	def consumeLL(self):
		oldInput = self.input
		self.input = self.inputStream.read(1)
		return(oldInput)
	def raiseError(self, expected, got=None):
		if got is None:
			got = self.input
		raise SyntaxError("expected %r but got %r" % (expected, got))
	def parseControlWordToken(self):
		result = io.StringIO()
		count = 0
		while controlWordP(self.input):
			result.write(self.consumeLL())
			count += 1
			if count >= 32:
				break # NOTE: spec doesn't say what should happen...
		return(symbols.intern(result.getvalue()))
	def parseNumeralToken(self):
		result = io.StringIO()
		count = 0
		while self.input and self.input in "-0123456789":
			result.write(self.consumeLL())
			count += 1
		return(int(result.getvalue()))
	def parseControlSymbolToken(self):
		# *~
		return(symbols.intern(self.consumeLL()))
	def parseToken(self):
		if self.input == '\\':
			self.consumeLL()
			arg = None
			if controlWordP(self.input):
				cw = self.parseControlWordToken()
				if self.input == " ":
					self.consumeLL()
				elif self.input and self.input in "0123456789-":
					# numeric argument follows and "becomes part of the control word"
					arg = self.parseNumeralToken()
					if self.input == " ":
						self.consumeLL()
			else:
				cw = self.parseControlSymbolToken()
			return (cw, arg) # FIXME
		elif self.input == '{':
			self.consumeLL()
			return symbols.intern("{")
		elif self.input == '}':
			self.consumeLL()
			return symbols.intern("}")
		else:
			#	self.raiseError("\"\\\"|\"{\"")
			v = self.input
			self.consumeLL()
			return symbols.intern(v)
	def consume(self):
		oldValue = self.inputValue
		self.inputValue = self.parseToken()
		return(oldValue)

# Note: there can be a "group" at the beginning (starting with "{") which contains screenColor etc.
		
class Group(object):
	def __init__(self, next=None):
		self.next = next

class Parser(object):
	def __init__(self, scanner):
		self.scanner = scanner
		self.group = Group()
		self.destination = io.StringIO()
	def popGroup(self):
		self.group = self.group.next
	def pushNewGroup(self):
		self.pushGroup(Group())
	def pushGroup(self, group):
		assert(group.next is None)
		group.next = self.group
		self.group = group
	"""
	control word keywords:
		rtf
		ansi
		deff0
		fonttbl
			f0
			froman
			f1
			fdecor
		colortbl
			red
			green
			blue
		stylesheet
			fs
			snext0
		info
			author
		creatim
			yr
			mo
			dy
			hr
			min
		version
		edmins
		nofpages
		nofwords
		nofchars
		vern
		widoctrl
		ftnbj
		sectd
		linex
		endnhere
		pard
		plain
		fs
		par

		pard
		tab (!)
	"""
	# backslash escaped: \, {, }
	def parse(self):
		scanner = self.scanner
		while scanner.inputValue:
			sys.stdout.write(str(scanner.inputValue))
			scanner.consume()
def prepareFile(f):
	s = Scanner()
	s.push(f)
	s.consumeLL()
	s.consume()
	return(s)
def parseFile(f):
	s = prepareFile(f)
	p = Parser(s)
	return p.parse()

if __name__ == "__main__":
	parseFile(open("../literature/PC-Intern/INTERN.RTF", "r"))
"""
\{\--- Zeichen ausgeben, die sich noch im Tastaturpuffer befinden

"""
# TODO: http://cpansearch.perl.org/src/SARGIE/RTF-Tokenizer-1.13/lib/RTF/Tokenizer.pm
