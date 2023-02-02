.. include:: ../exports/alias.include

.. _user_manual_user_interface:

##############
User Interface
##############

|eddsrecorder| is a user application executed from command line.

.. contents::
    :local:
    :backlinks: none
    :depth: 1

Source Dependency Libraries
---------------------------

|eddsrecorder| depends on |fastdds| ``fastrtps``, ``fastcdr`` and ``ddsrouter`` libraries.
In order to correctly execute the Recorder, make sure that ``fastrtps``, ``fastcdr`` and ``ddsrouter`` are properly sourced.

.. code-block:: bash

    source <path-to-fastdds-installation>/install/setup.bash
    source <path-to-ddsrouter-installation>/install/setup.bash

.. note::

    If Fast DDS and DDS Router have been installed in the system, these libraries would be sourced by default.


.. note::
    This page is under maintenance and will be updated soon.
