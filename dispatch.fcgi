#!/usr/bin/env python2.7

from __future__ import with_statement
import locale
locale.setlocale(locale.LC_ALL, "en_US.UTF-8")
import sys
import time
#import codecs
import StringIO
#sys.stdout = codecs.getwriter('utf8')(sys.stdout)
import common.fcgi as fcgi
import imp
from common.site import Template, serveSimpleResponse, servePage, urlFromFilename, formatLastModified, startResponse, printPathBar, navigationB, getEntries, getEntries2, serveDefaultPage, pathjoin, renderBinaryFile
import sys
import urllib

# TODO (maybe) ETag, Expires header.
# <http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html>
# TODO serve style sheets as text/css, not text/plain.

def runGlobal(callback):
    counter=0
    try:
        while fcgi.isFCGI():
            req = fcgi.Accept()
            req.fieldStorage = req.getFieldStorage() # ugh. Anyway, calling it twice wouldn't work, so it's still less bad than the alternative.
            counter=counter+1
            status = callback(req)
            #if result is not None:
            #    for item in result:
            #        req.out.write(str(item))
            req.Finish(status or 0)
    except:
        import traceback
        f = open("/home/danny_milo/traceback", "w")
        traceback.print_exc(file=f)
        f.close()
        req.Finish(500)

class WSGIServer(object):
    def __init__(self, appCallback):
        self.appCallback = appCallback
    def run(self):
        runGlobal(self.appCallback)

import common.saxtree
import common.frame
import common.magic
import stat
import os

# TODO
#<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
#"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
from common.saxtree import HTML, HEAD, META, BODY, P, Text, TABLE, TR, TD, LINK, SCRIPT, DIV, A, IMG, EM, BR

def getBodyAttributeAdds(rawData):
    i = rawData.find("$body_attribute_adds")
    if i == -1:
        return {}
    rawData = rawData[i:]
    i = rawData.find("\n")
    if i != -1:
        rawData = rawData[:i]
    rawData = rawData[len("$body_attribute_adds") : ].strip()
    if not rawData.startswith('='):
        return {}
    rawData = rawData[len("="):].strip().rstrip(";").strip()
    r = eval(rawData, {}, {}) # => " onload=\"init();\" "
    r = eval("dict(%s)" % r, {}, {})
    return r
def servePseudoPHPPage(name, req, outHeaders):
    """ serves a pseudo-PHP page. This is for backwards compatibility with the existing site (which was in PHP previously). Note that most PHP code is not executed, it's mostly data recycling. """
    with open(name, "rb") as f:
        rawData = f.read()
        onload = getBodyAttributeAdds(rawData).get("onload")
        with Template(serveDefaultPage(name, req, outHeaders, onload=onload)):
            data = StringIO.StringIO("<div>%s</div>" % (rawData, ))
            common.saxtree.copyFrom(data)
def inClientCacheP(stat1, req):
    mtime = stat1.st_mtime
    If_Modified_Since = req.env.get("If_Modified_Since")
    try:
        # TODO test this. TODO non-GMT times.
        xmtime = time.strptime(If_Modified_Since, HTTP_DATE_FMT)
        return(xmtime >= mtime)
    except:
        return(False)
def serveFile(name, req, stat1):
    regFlag = stat.S_ISREG(stat1.st_mode)
    if (stat1.st_mode & stat.S_IXUSR) != 0:
        # executable
        mod = imp.load_source(name.replace("/", "_").replace(".", "_"), name) # first is mod name
        mod.main(name, req)
        return
    if not regFlag: # wtf... TODO cache indefinitely.
        with Template(servePage(req, 200, "OK", [])):
            pass
    else: # regular file, ok.
        REQUEST_METHOD = req.env.get("REQUEST_METHOD", "UNKNOWN")
        headers = [("Last-Modified", formatLastModified(stat1.st_mtime))]
        if inClientCacheP(stat1, req):
            serveSimpleResponse(req, 304, "Not Modified", headers)
            return
        with open(name, "rb") as f:
            if not renderBinaryFile(f, req.out, REQUEST_METHOD != "HEAD"):
                if REQUEST_METHOD == "HEAD":
                    serveSimpleResponse(req, 200, "OK", headers)
                else:
                    with Template(servePseudoPHPPage(name, req, headers)):
                        pass
def serveFileOrDir(name, req):
    """ also handles 404 """
    try:
        stat1 = os.stat(name)
    except OSError:
        with Template(servePage(req, 404, "Not Found", [])):
            with Text("Resource not found: "):
                pass
            with EM():
                with Text(name):
                    pass
            return
            pass
    # if it is a dir, redirect. TODO HTTP redirect instead? Probably overkill.
    if stat.S_ISDIR(stat1.st_mode):
        if not name.endswith("/"):
            name = "%s/" % (name, )
        name = pathjoin(name, "index")
        try:
            stat1 = os.stat(name)
        except OSError:
            #if not os.path.exists(name):
            headers = [("Last-Modified", formatLastModified(stat1.st_mtime))]
            with Template(serveDefaultPage(name, req, headers)):
                pass
            return
    serveFile(name, req, stat1)

def app(req):
    DOCUMENT_ROOT = req.env.get("DOCUMENT_ROOT", "R") # FIXME
    PATH_INFO = req.env.get("PATH_INFO", "/.").split("?")[0] # PATH_INFO"]
    #PATH_INFO = urllib.unquote(PATH_INFO)
    name = pathjoin(DOCUMENT_ROOT, PATH_INFO)
    serveFileOrDir(name, req)

"""class REQ(object):
	env = {
		"get": lambda a,b=None: b
	}
	out = sys.stdout
req = REQ()
serveFileOrDir("/home/danny_milo/scratchpost.org/index", req)
"""
WSGIServer(app).run() # , bindAddress = "/tmp/fcgi.sock").run()"""

"""
TODO
$page_extra_files = array(); // user: set this to have the mtime checker check these files (also directories), too
if (file_exists("image")) { /* magic magic mushroom */
        array_push($page_extra_files, "image"); /* gallery mode is automatic if there exists a directory "image" */
        // preserve bandwidth by caching:
        header('Expires: ' . gmdate('D, d M Y H:i:s', time() + 24 * 60 * 60) . ' GMT');
}

gallery:
	selected image
	special-case image/svg+xml
	group images (see image/.webgrouping-formats (%s etc) and image/.webgrouping (regexp))
	thumbnails
	limit to the last 80 entries

"""
