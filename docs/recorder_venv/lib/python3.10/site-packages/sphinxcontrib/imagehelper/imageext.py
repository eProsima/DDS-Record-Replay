import os
import re
import cgi
import posixpath
from math import ceil
from docutils import nodes
from sphinx.util.osutil import ensuredir
from docutils.parsers.rst import directives
from docutils.parsers.rst.directives.images import Image, Figure
from sphinxcontrib.imagehelper.utils import get_imagedir

URI_PATTERN = re.compile('^\w+://')


class image_node(nodes.General, nodes.Element):
    pass


def get_imageext_handler(app, uri):
    ext = os.path.splitext(uri.lower())[1][1:]
    if ext in app.imageext_types:
        _, handler = app.imageext_types[ext]
        return handler
    else:
        for pattern in app.imageext_url_patterns:
            if uri.startswith(pattern):
                _, handler = app.imageext_url_patterns[pattern]
                return handler
        else:
            return None


def get_imageext_handler_by_name(app, imageext_type):
    for name, handler in app.imageext_types.values():
        if name == imageext_type:
            return handler
    else:
        for name, handler in app.imageext_url_patterns.values():
            if name == imageext_type:
                return handler

        return None


def on_builder_inited(app):
    Image.option_spec['option'] = directives.unchanged
    Figure.option_spec['option'] = directives.unchanged


def on_doctree_read(app, doctree):
    for image in doctree.traverse(nodes.image):
        handler = get_imageext_handler(app, image['uri'])
        option_spec = getattr(handler, 'option_spec', {})

        options = cgi.parse_qs(image.get('option', ''))
        for name in options:
            if name not in option_spec:
                app.warn('Unsupported option `%s` found at %s' % (name, image['uri']))
            else:
                try:
                    for value in options.get(name):
                        image[name] = option_spec[name](value)
                except (ValueError, TypeError) as exc:
                    app.warn('Fail to apply `%s` option to %s:\n%s' %
                             (name, image['uri'], ' '.join(exc.args)))


def on_doctree_resolved(app, doctree, docname):
    for image in doctree.traverse(nodes.image):
        handler = get_imageext_handler(app, image['uri'])
        if handler:
            handler(app).visit(docname, image)

    for image in doctree.traverse(image_node):
        handler = get_imageext_handler_by_name(app, image['imageext_type'])
        if handler:
            handler(app).visit(docname, image)


class ImageConverter(object):
    option_spec = {}

    def __init__(self, app):
        self.app = app
        self.warn = app.warn

    def visit(self, docname, image_node):
        rel_imagedir, abs_imagedir = get_imagedir(self.app, docname)
        basename = self.get_filename_for(image_node)
        if URI_PATTERN.match(image_node['uri']):
            srcpath = image_node['uri']
        else:
            srcpath = os.path.join(self.app.srcdir, image_node['uri'])
        abs_imgpath = os.path.join(abs_imagedir, basename)

        last_modified = self.get_last_modified_for(image_node)
        if last_modified is None:
            ret = False
        elif not os.path.exists(abs_imgpath) or os.stat(abs_imgpath).st_mtime < last_modified:
            ensuredir(os.path.dirname(abs_imgpath))
            ret = self.convert(image_node,
                               os.path.normpath(srcpath),
                               os.path.normpath(abs_imgpath))
        else:
            ret = True

        if ret:
            if last_modified is not None and os.path.exists(abs_imgpath):
                os.utime(abs_imgpath, (last_modified, last_modified))

            rel_imgpath = posixpath.join(rel_imagedir, basename)
            newnode = nodes.image(**image_node.attributes)
            newnode['candidates'] = {'*': rel_imgpath}
            newnode['uri'] = rel_imgpath
            image_node.replace_self(newnode)
        else:
            image_node.parent.remove(image_node)

    def get_last_modified_for(self, node):
        path = os.path.join(self.app.srcdir, node['uri'])
        if os.path.exists(path):
            return ceil(os.stat(path).st_mtime)
        else:
            return None

    def get_filename_for(self, node):
        return os.path.splitext(node['uri'])[0] + '.png'

    def convert(self, node, filename, to):
        pass


def add_image_type(app, name, ext, handler):
    if not hasattr(app, 'imageext_types'):
        app.add_node(image_node)
        app.connect('builder-inited', on_builder_inited)
        app.connect('doctree-read', on_doctree_read)
        app.connect('doctree-resolved', on_doctree_resolved)
        app.imageext_types = {}
        app.imageext_url_patterns = {}

    if isinstance(ext, (list, tuple)):
        for e in ext:
            add_image_type(app, name, e, handler)
    elif URI_PATTERN.match(ext):
        app.imageext_url_patterns[ext] = (name, handler)
    else:
        if ext.startswith('.'):
            ext = ext[1:]

        app.imageext_types[ext] = (name, handler)
