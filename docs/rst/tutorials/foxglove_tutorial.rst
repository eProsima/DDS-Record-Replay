.. include:: ../exports/alias.include

.. _tutorials_foxglove:

############################
Visualize MCAP with Foxglove
############################

This tutorial records schema-backed ShapesDemo data and opens the finalized
MCAP in Foxglove. Foxglove's account and local-file requirements can change;
consult its current
`local data documentation <https://docs.foxglove.dev/docs/visualization/connecting/local-data>`__.

Prerequisites
=============

* DDS Record & Replay installed on :ref:`Linux <installation_manual_linux>` or
  :ref:`Windows <installation_manual_windows>`.
* `eProsima ShapesDemo <https://eprosima-shapes-demo.readthedocs.io/en/stable/>`__.
* Access to the Foxglove web or desktop application.

.. _tutorials_foxglove_configuring_recorder:

Configure Recorder
==================

Save this validated configuration as ``foxglove-recorder.yaml``:

.. literalinclude:: ../examples/recorder_foxglove.yaml
   :language: yaml
   :linenos:

``only-with-type`` prevents schema-less channels, while ``record-types`` keeps
the dynamic type data used by Replayer and inspection tools. This example uses
raw DDS/OMG IDL conventions; set ``ros2-types: true`` when the source topics and
consumer expect ROS 2 MCAP naming and schemas.

Record ShapesDemo
=================

#. Start ShapesDemo on domain ``0``.
#. Create publishers for ``Square``, ``Triangle``, or ``Circle``.
#. Start Recorder:

   .. code-block:: console

      $ ddsrecorder --config-path foxglove-recorder.yaml

#. Let the publishers run, then press :kbd:`Ctrl+C`.
#. Select the finalized file named similarly to
   ``2026-08-24_14-30-00_CEST_shapesdemo_data.mcap``. Do not select the
   ``.mcap.tmp~`` file.

Open and inspect the recording
==============================

Open the local MCAP by dragging it into Foxglove, pressing :kbd:`Ctrl+O`, or
choosing **Open local file(s)**. The channel list should contain the ShapesDemo
topics. Add a raw-message or plot panel and select one of those channels.

If a topic appears but its value cannot be decoded, inspect whether the channel
has a non-empty schema and confirm ``record-types``/``ros2-types`` match the
source system. Continue with :ref:`troubleshooting` for schema and temporary-file
checks.
