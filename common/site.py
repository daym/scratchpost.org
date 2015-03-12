#!/usr/bin/env python

from __future__ import with_statement
import sys
import hashlib
from saxtree import HTML, HEAD, META, BODY, P, Text, TABLE, TR, TD, LINK, SCRIPT, DIV, A, IMG, EM, BR, NAV
import magic
import time
import saxtree
import StringIO
import urllib
import os
import stat
import urlparse

def pathjoin(a, b):
    return a + b
def copy(source, destination):
	while True:
		b = source.read(8192)
		if b == "":
			break
		destination.write(b)
def loadStrings(name):
    with open(name, "r") as f:
        for line in f.xreadlines():
            yield line.rstrip("\n")

class Template(object):
    def __init__(self, generator):
        self.generator = generator
    def __enter__(self):
        if self.generator is not None:
          self.generator.next()
    def __exit__(self, ty, exc, tb):
        try:
            if self.generator is not None:
              self.generator.next()        
        except StopIteration:
            pass
def serveSimpleResponse(req, statusCode, statusText, headers):
    """ serveSimpleResponse("200", "OK", [("Content-Type", "text/html; charset=utf-8")]) """
    startResponse(req.out, statusCode, statusText, headers)
    req.out.flush()

def servePage(req, statusCode, statusText, outHeaders, onload=None):
    """ serves a HTML page without content. If you want to add content, be sure to call using 'with Template(servePage(...)):'"""
    headers = [(k, v) for k, v in outHeaders if k.lower() != "Content-Type"]
    headers.append(("Content-Type", "text/html; charset=utf-8"))
    startResponse(req.out, statusCode, statusText, headers)
    saxtree.init(req.out)
    with HTML(xmlns="http://www.w3.org/1999/xhtml"):
        with HEAD():
            with META(**{"http-equiv": "Content-Type", "content": "application/xhtml+xml; charset=UTF-8"}): # IE
                pass
            with META(name="MSSmartTagsPreventParsing", content="TRUE"):
                pass
            with LINK(rel="openid.server", href="http://www.livejournal.com/"):
                pass
            with LINK(rel="openid.delegate", href="http://dannymi.livejournal.com/"):
                pass
            with LINK(rel="stylesheet", href="/common/scratchpost.css", type="text/css"):
                pass
            with LINK(rel="shortcut icon", href="/common/icon/scratchpost.PNG", type="image/png"):
                pass
            with LINK(rel="icon", href="/common/icon/scratchpost.PNG", type="image/png"):
                pass
            with LINK(rel="up", href="../"):
                pass
            with SCRIPT(type="text/javascript", src="/common/scratchpost.js"):
                with Text(" "):  # Opera workaround
                    pass
        with BODY(**({"onload": onload} if onload is not None else {})):
            with Template(navigationB(req)) as realBody:
                yield realBody
def urlFromFilename(name):
    return urllib.quote(name)
    
HTTP_DATE_FMT = "%a, %d %b %Y %H:%M:%S GMT"
def formatLastModified(mtime):
    humanTime = time.gmtime(mtime)
    return(time.strftime(HTTP_DATE_FMT, humanTime))
def startResponse(out, statusCode, statusText, headers):
    statusCode = int(statusCode)
    headers = headers[:]
    headers.append(("Date", formatLastModified(time.time())))
    headers.append(("Status", "%s %s" % (statusCode, statusText)))
    for k, v in headers:
       out.write("%s: %s\r\n" % (k, v)) # TODO escape
    out.write("\r\n")
def renderBinaryFile(f, out, bWriteBody):
    """ renders the file f, if it is binary. 
        returns: whether the file f was rendered or not. """
    stat1 = os.fstat(f.fileno())
    regularFileFlag = stat.S_ISREG(stat1.st_mode)
    if regularFileFlag:
        mimeType = magic.getRegularFileType(f)
        if mimeType != "application/x-httpd-php": # or True:
                f.seek(0, 2)
                l = f.tell()
                f.seek(0)
                startResponse(out, 200, "OK", [("Content-Type", mimeType), ("Content-Length", l), ("Last-Modified", formatLastModified(stat1.st_mtime))])
                if bWriteBody:
                    copy(f, out)
                return(True)
    return(False)
# see <http://developer.yahoo.com/performance/rules.html#etags>

def printPathBar(dirName):
    io = StringIO.StringIO()
    parts = dirName.split("/")
    with P(): # path
        for part in parts:
            if part == "":
                continue
            with Text("/"):
                pass
            io.write("/")
            io.write(part)
            with A(href=urlFromFilename(io.getvalue())):
                with Text(part):
                    pass
def getEntries(containerName):
    try:
        webhidden = set(loadStrings("%s/.webhidden" % (containerName, )))
    except:
        webhidden = set()
    webhidden.add("index") # TODO except when we aren't it?
    for name in os.listdir(containerName):
        if name.startswith(".") or name in webhidden:
            continue
        title = name
        bmtime = 0
        try:
            bname = os.path.join(containerName, name)
            st = os.stat(bname)
            bmtime = st.st_mtime
            if stat.S_ISDIR(st.st_mode):
                title = "%s/" % (name, )
        except OSError:
            pass
        yield (title, name, bmtime)
def getEntries2(containerName):
    #yield from getEntries(containerName)
    for entry in sorted(getEntries(containerName)):
        yield entry
    try:
        webseealso = loadStrings("%s/.webseealso" % (containerName, ))
        webseealso = map(lambda line: line.split(" ", 1), webseealso) # URL, description
    except:
        webseealso = []
    for URL, description in webseealso:
        yield ("See also: %s" % (description, ), URL, None)
def getNewestSubpage(base): # -> (title, path, mtime)
    #st = os.stat(base)
    #mtime = st.st_mtime
    title = None
    name = None
    mtime = 0
    for xtitle, yname, xmtime in getEntries(base):
        xname = os.path.join(base, yname)
        if xtitle.endswith("/") and not xtitle.startswith("."):
            btitle, bname, bmtime = getNewestSubpage(xname)
            if bmtime > xmtime:
                xname = bname
                xmtime = bmtime
                xtitle = btitle
        if xmtime > mtime:
            name = xname
            mtime = xmtime
            title = xtitle
    return title, name, mtime
def md5hexdigestText(text):
    m = hashlib.md5()
    m.update(text)
    return(m.hexdigest())
def gallery(folderName, req):
    DOCUMENT_ROOT = req.env.get("DOCUMENT_ROOT", "R")
    ifolderName = "%s/image" % (folderName, )
    idirName = pathjoin(DOCUMENT_ROOT, ifolderName)
    with P():
       with Text(ifolderName):
          pass
    for title, href, mtime in sorted(getEntries(idirName)):
        #with A(href=href):
        #with DIV(**{"id": "anchor_%s" % (md5hexdigestText(href), ), "class": "image_of_the_day_big_image"}):
        with IMG(**{"alt": title, "src": urlFromFilename("%s/%s" % (ifolderName, href)), "class": "page"}):
            pass
    with BR(clear="both") as br:
        yield br
def navigationB(req):
    from common import saxdirtree
    DOCUMENT_ROOT = req.env.get("DOCUMENT_ROOT", "R")
    #PATH_INFO = req.env.get("REQUEST_URI", ".").split("?")[0] # PATH_INFO"]
    #PATH_INFO = urllib.unquote(PATH_INFO)
    PATH_INFO = req.env.get("PATH_INFO", "/.").split("?")[0] # PATH_INFO"]
    folderName = PATH_INFO # FIXME
    dirName = pathjoin(DOCUMENT_ROOT, folderName)
    with TABLE(**{"border": "0", "class": "main_navigation_area"}):
        with TR():
            with TD():
                with NAV(**{"class": "main_navigation", "id": "main_navigation"}):
                    printPathBar(PATH_INFO)
                    try:
                        saxdirtree.printList({"class": "main_directory"}, getEntries2(dirName))
                    except OSError:
                        pass
            with TD(**{"class": "main"}):
                # TODO H1
                with DIV(**{"class": "main_border", "id": "main_border"}):
                    with DIV(**{"class": "main_content"}) as main_content:
                        if os.path.exists(dirName + "/image") and (not os.path.exists(dirName + "/image/equation") or os.stat(dirName + "/image").st_nlink > 3):
                            with Template(gallery(folderName, req)):
                                pass
                        # everything else.
                        yield main_content
def modificationB(req, name):
    DOCUMENT_ROOT = req.env.get("DOCUMENT_ROOT", "R")
    dname = os.path.dirname(name)
    title, name, mtime = getNewestSubpage(dname)
    if name and name.startswith(DOCUMENT_ROOT):
        name = name[len(DOCUMENT_ROOT):]
        with P() as result:
            with Text("Last Modification in "):
                pass
            with A(href=urlFromFilename(name)):
                with Text(name):
                    pass
            with Text(" on "):
                pass
            humanReadableTime = time.asctime(time.gmtime(mtime))
            with Text(humanReadableTime):
                pass
            with Text("."):
                pass
            yield(result)
def serveDefaultPage(name, req, outHeaders, onload=None):
    with Template(servePage(req, 200, "OK", outHeaders, onload=onload)) as t:
        yield t
        with Template(modificationB(req, name)) as t2:
            pass
        # TODO webauthor


"""
urlparse.urlunparse
>>> urllib.urlencode({"a": "b", "c": "d"})
'a=b&c=d'
>>> urlparse.urlunparse(("http", "a", "x", ("a", "b"), "q", "frag"))
"http://a/x;('a', 'b')?q#frag"
>>> urlparse.urlunparse(("http", "a", "x", None, "X", "frag"))
'http://a/x?X#frag'
"""

def makeURL(scheme, netloc, url, params = None, query = None, fragment = None):
	"""
	>>> makeURL("http", "a", "x", None, {"a": "b", "c": "d"}, "frag")
	'http://a/x?a=b&c=d#frag'
	"""
	squery = urllib.urlencode(query)
	return urlparse.urlunparse((scheme, netloc, url, params, squery, fragment))

if __name__ == "__main__":
	makeURL("https", "api.dreamhost.com", "/", None, {
		"key": "xxx",
		"cmd": "a",
		"format": "json",
		"unique_id": hash("DNS_UPDATE"),
	}, None)
