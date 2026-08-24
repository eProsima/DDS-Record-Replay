.. include:: ../exports/alias.include

.. _docker:

######################
Docker image: Recorder
######################

The distributed DDS Record & Replay Docker image contains ``ddsrecorder`` only.
Install from source when Replayer, ``mcap-convert``, or the graphical controller
is required.

Load the image archive downloaded from the
`eProsima downloads page <https://www.eprosima.com/product-download>`__:

.. code-block:: console

   $ docker load --input ubuntu-ddsrecorder-<version>.tar
   $ docker image ls

Use the exact image repository and tag printed by ``docker load`` in the run
command below.

Create ``recorder.yaml``:

.. literalinclude:: ../examples/recorder_docker.yaml
   :language: yaml
   :linenos:

Create the host output directory and run the Linux container with host DDS
networking and shared-memory namespace:

.. code-block:: console

   $ mkdir -p output
   $ docker run --rm -it \
       --network host \
       --ipc host \
       -v "$PWD/recorder.yaml:/root/DDS_RECORDER_CONFIGURATION.yaml:ro" \
       -v "$PWD/output:/root/output" \
       <loaded-image:tag>

Press :kbd:`Ctrl+C` to finalize the recording. The resulting MCAP appears in
the host ``output`` directory.

``--network host`` is required by this example's DDS discovery model and has
platform-specific behavior on Docker Desktop. When host networking is not
available, configure explicit UDP discovery and port mappings for the DDS
deployment instead of copying this command unchanged.
