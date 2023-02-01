import os
from sphinx.util.osutil import relative_uri


def get_imagedir(app, docname):
    if hasattr(app.builder, 'imagedir'):  # Sphinx (>= 1.3.x)
        dirname = app.builder.imagedir
    elif hasattr(app.builder, 'imgpath') or app.builder.format == 'html':  # Sphinx (<= 1.2.x) and HTML writer
        dirname = '_images'
    else:
        dirname = ''

    if dirname:
        relpath = relative_uri(app.builder.get_target_uri(docname), dirname)
    else:
        relpath = ''

    abspath = os.path.join(app.builder.outdir, dirname)
    return (relpath, abspath)
