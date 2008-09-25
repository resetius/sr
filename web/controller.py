#!/usr/bin/env python
# -*- coding: utf-8 -*-


import operator, os, pickle, sys
import cherrypy

from genshi.template import TemplateLoader
from formencode import Invalid

from stem_result import StemResult

loader = TemplateLoader(
    os.path.join(os.path.dirname(__file__), 'templates'),
    auto_reload=True
)

class Root(object):

    def __init__(self, root):
        self.root = root

    @cherrypy.expose
    def index(self):
        tmpl = loader.load('index.html')
        return tmpl.generate(root=self.root, title=u'Main Page').render('html', doctype='html')

    @cherrypy.expose
    def conv2(self, cancel=False, **data):
        tmpl = loader.load('conv2.html')
        stream = tmpl.generate(root=self.root, title=u'Conv2')
        return stream.render('html', doctype='html')

    @cherrypy.expose
    def stemmer2(self, cancel=False, **data):
        tmpl = loader.load('stemmer2.html')
        result = StemResult(data)
        stream = tmpl.generate(root=self.root, stem = result, title=u'Stemmer2')
        return stream.render('html', doctype='html')

def main():
    data = {} # We'll replace this later

    # Some global configuration; note that this could be moved into a
    # configuration file
    cherrypy.config.update({
		'server.socket_port': 8084,
		'server.socket_host': '0.0.0.0',
        'tools.encode.on': True, 'tools.encode.encoding': 'utf-8',
        'tools.decode.on': True,
        'tools.trailing_slash.on': True,
        'tools.staticdir.root': os.path.abspath(os.path.dirname(__file__)),
    })

    cherrypy.quickstart(Root('/'), '/', {
        '/media': {
            'tools.staticdir.on': True,
            'tools.staticdir.dir': 'static'
        }
    })

def serverless():

    # Some global configuration; note that this could be moved into a
    # configuration file
    cherrypy.config.update({
        'log.screen': False,
	'log.error_file': '/tmp/site.log',
        'environment': 'production',
        'tools.encode.on': True, 'tools.encode.encoding': 'utf-8',
        'tools.decode.on': True,
        'tools.trailing_slash.on': True,
        'tools.staticdir.root': os.path.abspath(os.path.dirname(__file__)),
        'request.show_tracebacks': False,
        # Turn off signal handlers when CP does not control the OS process
        'engine.SIGTERM': None,
        'engine.SIGHUP': None,
    })
    
    cherrypy.tree.mount(Root('/cherry'), '/cherry', {
        '/media': {
            'tools.staticdir.on': True,
            'tools.staticdir.dir': 'static'
        }
    })

if __name__ == '__main__':
    main()

