#!/usr/bin/python
# -*- coding: utf-8 -*-
from twisted.internet import reactor
from twisted.web.server import Site
from twisted.web.resource import Resource
from twisted.web.server import NOT_DONE_YET
from twisted.web import http

import json
import time
import base64
import collections
import cgi
import sys
import os
import subprocess


class HyperionServer(Resource):
    isLeaf = True

    def render_OPTIONS(self, request):
        request.setHeader('Access-Control-Allow-Origin', '*')
        request.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTION')
        request.setHeader('Access-Control-Allow-Headers', 'Content-type')
        request.setHeader('Access-Control-Max-Age', 2520)
        request.setHeader('Content-type', 'text/html')
        request.write('')
        request.finish()
        return NOT_DONE_YET

    def render_GET(self, request):
        if request:
			try:
				args = request.args
				if "arg" in args:
					if args["arg"][0] == "start":
						os.system("sudo service hyperion start")
						request.write('started')
					elif args["arg"][0] == "stop":
						os.system("sudo service hyperion stop")
						request.write('stopped')
					elif args["arg"][0] == "status":
						proc = subprocess.Popen(["sudo service hyperion status"], stdout=subprocess.PIPE, shell=True)
						(out, err) = proc.communicate()
						out = str(out).split("Active: ")[1].split('(')[0]
						request.write(str(out))
			except Exception, ex:
				request.write('error')
			request.finish()
        return NOT_DONE_YET

root = HyperionServer()
factory = Site(root)
reactor.listenTCP(19447, factory)
reactor.run()
exit()
