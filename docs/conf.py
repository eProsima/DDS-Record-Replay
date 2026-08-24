# Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0

"""Sphinx configuration for DDS Record & Replay.

The documentation build is intentionally self-contained.  In particular, it
does not download theme configuration, CSS, templates, or images at import
time.  This keeps local, CI, and Read the Docs builds reproducible.
"""

import os
import re


PROJECT_NAME = 'DDS Record & Replay'
COMPRESS_PROJECT_NAME = 'ddsrecordreplay'


def get_version(version_file):
    """Read VERSION_MAJOR/MINOR/PATCH from a project VERSION file."""
    version_parts = {}
    try:
        with open(version_file, 'r', encoding='utf-8') as version_handle:
            for line in version_handle:
                match = re.match(r'VERSION_(MAJOR|MINOR|PATCH)\s+(\S+)', line)
                if match:
                    version_parts[match.group(1).lower()] = match.group(2)
    except OSError:
        return None

    if {'major', 'minor', 'patch'} <= version_parts.keys():
        return version_parts
    return None


script_path = os.path.abspath(os.path.dirname(__file__))
versions = get_version(os.path.join(script_path, 'VERSION'))
if versions is None:
    versions = get_version(os.path.join(script_path, '..', 'VERSION'))
if versions is None:
    raise RuntimeError('Could not determine the DDS Record & Replay version.')

project = PROJECT_NAME
copyright = '2021, eProsima'
author = 'eProsima'
version = '{}.{}'.format(versions['major'], versions['minor'])
release = '{}.{}.{}'.format(
    versions['major'], versions['minor'], versions['patch'])
language = 'en'
rst_prolog = f'''\
.. |release-url| replace:: https://raw.githubusercontent.com/eProsima/DDS-Record-Replay/v{release}/ddsrecordreplay.repos
.. |release-tag| replace:: v{release}
'''

extensions = [
    'sphinx.ext.todo',
    'sphinx_design',
]

try:
    import sphinxcontrib.spelling  # noqa: F401

    extensions.append('sphinxcontrib.spelling')
    spelling_word_list_filename = ['rst/spelling_wordlist.txt']

    from sphinxcontrib.spelling.filters import ContractionFilter

    spelling_filters = [ContractionFilter]
    spelling_ignore_contributor_names = False
except ImportError:
    pass

templates_path = ['rst/_templates']
source_suffix = '.rst'
master_doc = 'index'
exclude_patterns = [
    '*/includes/*.rst',
    '*/*/includes/*.rst',
    '*/*/*/includes/*.rst',
    '*/*/*/*/includes/*.rst',
]
todo_include_todos = False
numfig = True
suppress_warnings = ['config.cache']

html_theme = 'furo'
html_theme_options = {
    'navigation_with_keys': True,
    'top_of_page_button': 'edit',
}
html_title = f'<center><i>{release}</i></center>'
html_logo = 'rst/_static/dds-record-replay-logo.png'
html_favicon = 'rst/_static/css/imgs/eProsima.ico'
html_static_path = ['rst/_static']
html_css_files = ['css/eprosima-furo.css']
html_show_sphinx = False

htmlhelp_basename = f'{PROJECT_NAME} Manual'
latex_documents = [
    (
        master_doc,
        f'{COMPRESS_PROJECT_NAME}.tex',
        f'{PROJECT_NAME} Documentation',
        'eProsima',
        'manual',
    ),
]
man_pages = [
    (master_doc, PROJECT_NAME, f'{PROJECT_NAME} Documentation', [author], 1)
]
texinfo_documents = [
    (
        master_doc,
        PROJECT_NAME,
        f'{PROJECT_NAME} Documentation',
        author,
        PROJECT_NAME,
        f'Documentation of eProsima {PROJECT_NAME}',
        'Miscellaneous',
    ),
]
