#!/usr/bin/env python

from __future__ import with_statement
import saxtree
from saxtree import UL, LI, Text, A

def printList(attrs, entries):
	with UL(**attrs):
		for entry in entries:
                        title, href = entry[:2]
			with LI():
				with A(href=href):
					with Text(title):
						pass
