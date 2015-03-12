#!/usr/bin/env python

def checkBMP(f):
	#} else if (substr($first_line, 0, 2) == "BM" && unpack_I4(substr($first_line, 2, 4)) == filesize($path)) {
	return(False)

def getRegularFileType(f):
	result = "application/octet-stream"
	header = f.read(270)
	f.seek(0)
	if header.startswith("<?php"):
		return("application/x-httpd-php") # text/html")
	elif header.startswith("<?xml"):
		if header.find("<svg") > -1:
			return("image/svg+xml")
		return("application/xml")
	elif header.startswith("<!DOCTYPE html"):
		return("text/html")
	elif header.startswith("\037\213\010\0") or header.startswith("\037\213\010\010"):
		return("application/x-gtar")
	elif header[257:257 + 6] == "ustar\0" or header[257:257 + 8] == "ustar  \0": # POSIX, GNU
		return("application/x-tar")
	elif header.startswith("\211PNG\r\n"):
		return("image/png")
	elif header.startswith("\377\330\377\340") or header.startswith("\377\330\377\341"):
		return("image/jpeg")
	elif header.startswith("BM") and checkBMP(f):
		return("image/x-ms-bmp") # not exactly standard.
	elif header.startswith("GIF89a"): # TODO more junk
		return("image/gif")
	elif header.startswith("RIFF") and header[8:8+4] == "AVI ":
		return("video/x-msvideo")
	elif header.startswith("!<arch>\ndebian-binary"):
		return("application/x-debian-package")
	elif header.startswith("%!PS-Adobe-2."):
		return("application/postscript")
	elif header.startswith("%PDF-"):
		return("application/pdf")
	elif header.startswith("PK\3\4"): 
		return("application/zip")
	elif header.startswith("\312\376\272\276"):
		return("application/x-java-class")
	elif header.startswith("# ") or header.startswith("#!"):
		return("text/plain")
	elif header.startswith("Index: "):
		return("text/plain") # patch
	elif header.startswith("ID3\3"):
		return("audio/mpeg")
	elif header.startswith("\060\046\262\165\216\146\317\021\246"):
		return("audio/x-ms-wma")
	elif header[10:10+4] == "\377\377\377\377": # HACK HACK
		return("image/svg+xml")
	elif len([c for c in header if ord(c) < 8 or (ord(c) >= 14 and ord(c) < 32)]) == 0:
		return("text/plain")
	elif header.startswith("\177ELF\1"):
		return("application/x-executable")
	elif header.startswith("MZ\220\0"):
		return("application/x-executable")
	elif header.startswith("\000\001\000\000\000"): # FIXME $tag = substr($first_line, 12, 4); // FFTM
		return("application/vnd.font-opentype")
