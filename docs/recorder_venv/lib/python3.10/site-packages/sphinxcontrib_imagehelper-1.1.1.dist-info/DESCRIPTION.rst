sphinxcontrib-imagehelper
==========================

.. image:: https://travis-ci.org/tk0miya/sphinxcontrib-imagehelper.svg?branch=master
   :target: https://travis-ci.org/tk0miya/sphinxcontrib-imagehelper

.. image:: https://coveralls.io/repos/tk0miya/sphinxcontrib-imagehelper/badge.png?branch=master
   :target: https://coveralls.io/r/tk0miya/sphinxcontrib-imagehelper?branch=master

.. image:: https://codeclimate.com/github/tk0miya/sphinxcontrib-imagehelper/badges/gpa.svg
   :target: https://codeclimate.com/github/tk0miya/sphinxcontrib-imagehelper

`sphinxcontrib-imagehelper` is helpers for creating image Sphinx extensions.

Adding new image format support to Sphinx is too boring.
This helper helps you to create image sphinx extensions.

It provides these features:

* Caching converted images
* Conseal sphinx directory structure; determine path of images automatically
* Support common options for image directive (cf. `:height:`, `:scale:`, `:align:`, and so on)
* Enhance standard imaging directives; `image` and `figure` get capability to embed new image format

With `sphinxcontrib-imagehelper`, all you have to do is only convert new image format to
well known image formats.

Install
=======

::

   $ pip install sphinxcontrib-imagehelper

Example
=======

::

    from sphinxcontrib.imagehelper import (
        add_image_type, add_image_directive, add_figure_directive, ImageConverter
    )

    # Declare converter class inherits ImageConverter
    class MyImageConverter(ImageConverter):
        # Override `get_filename_for()` to determine filename
        def get_filename_for(self, node):
            # filename is came from its URI and configuration
            hashed = sha1((node['uri'] + self.app.config.some_convert_settings).encode('utf-8')).hexdigest()
            return 'myimage-%s.png' % hashed

        # Override `convert()` to convert new image format to well known image formats (PNG, JPG and so on)
        def convert(self, node, filename, to):
            # Hint: you can refer self.app.builder.format to switch conversion behavior
            succeeded = convert_myimage_to_png(filename, to,
                                               option1=node['option'],
                                               option2=self.app.config.some_convert_settings)
            if succeeded:
                return True  # return True if conversion succeeded
            else:
                return False

    def setup(app)
        # Register new image type: myimage
        add_image_type(app, 'my', 'img', MyImageConverter)

        # Register my-image directive
        add_image_directive(app, 'my')

        # Register my-figure directive
        add_figure_directive(app, 'my')

Helpers
=======

`sphinxcontrib.imagehelper.add_image_type(app, name, ext, handler)`
    Register a new image type which is identified with file extension `ext`.
    The `handler` is used to convert image formats.

`sphinxcontrib.imagehelper.ImageConverter`
    A handler class for converting image formats. It is used at `add_image_type()`.
    The developers of sphinx-extensions should create a handler class which inherits `ImageConverter`,
    and should override two following methods:

    `ImageConverter.option_spec`
        A definition of additional options.
        By default, it is empty dict.

    `ImageConverter.get_last_modified_for(self, node)`
        Determine last modified time of target image.
        By default, this method returns the timestamp of the image file.

    `ImageConverter.get_filename_for(self, node)`
        Determine a filename of converted image.
        By default, this method returns the filename replaced its extension with '.png'::

            def get_filename_for(self, node):
                return os.path.splitext(node['uri'])[0] + '.png'

    `ImageConverter.convert(self, node, filename, to)`
        Convert image to embedable format.
        By default, this method does nothing.

`sphinxcontrib.imagehelper.add_image_directive(app, name, option_spec={})`
    Add a custom image directive to Sphinx.
    The directive is named as `name`-image (cf. astah-image).

    If `option_spec` is given, the new directive accepts custom options.

`sphinxcontrib.imagehelper.add_figure_directive(app, name, option_spec={})`
    Add a custom figure directive to Sphinx.
    The directive is named as `name`-figure (cf. astah-figure).

    If `option_spec` is given, the new directive accepts custom options.

`sphinxcontrib.imagehelper.generate_image_directive(name, option_spec={})`
    Generate a custom image directive class. The class is not registered to Sphinx.
    You can enhance the directive class with subclassing.

`sphinxcontrib.imagehelper.generate_figure_directive(name, option_spec={})`
    Generate a custom figure directive class. The class is not registered to Sphinx.
    You can enhance the directive class with subclassing.


