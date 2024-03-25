.. include:: ../exports/alias.include
.. include:: ../exports/roles.include

.. _docker:

##########################
Docker Image (recommended)
##########################

.. warning::
    Currently, |ddsrecord| Docker image only contains |ddsrecorder| tool, |ddsreplay| application will be added soon.

eProsima distributes a Docker image of |ddsrecord| with Ubuntu 22.04 as base image.
This image launches an instance of |ddsrecord| that is configured using a *YAML* configuration file provided by the user
and shared with the Docker container.
The steps to run |ddsrecord| in a Docker container are explained below.

#.  Download the compressed Docker image in ``.tar`` format from the
    `eProsima Downloads website <https://www.eprosima.com/index.php/downloads-all>`_.
    It is strongly recommended to download the image corresponding to the latest version of |ddsrecord|.

    |br|

#.  Extract the image by executing the following command:

    .. code-block:: bash

        load ubuntu-ddsrecorder:<version>.tar

    where ``version`` is the downloaded version of |ddsrecord|.

    |br|

#.  Build a |ddsrecord| configuration YAML file on the local machine.
    This will be the |ddsrecord| configuration file that runs inside the Docker container.
    To continue this installation manual, let's use the configuration file provided in :ref:`this tutorial <tutorials_foxglove>`.
    Open your preferred text editor and copy the configuration example from :ref:`here <tutorials_foxglove_configuring_recorder>`
    into the ``/<dds_recorder_ws>/DDS_RECORDER_CONFIGURATION.yaml`` file, where ``dds_recorder_ws`` is the path of the
    configuration file.
    To make this accessible from the Docker container we will create a shared volume containing just
    this file. This is explained in next point.

    |br|

#.  Run the Docker container executing the following command:

    .. code-block:: bash

        docker run -it \
            --net=host \
            --ipc=host \
            --privileged \
            -v /<dds_recorder_ws>/DDS_RECORDER_CONFIGURATION.yaml:/root/DDS_RECORDER_CONFIGURATION.yaml \
            ubuntu-ddsrecorder:v0.4.0

    It is important to mention that both the path to the configuration file hosted in the local machine and the one
    created in the Docker container must be absolute paths in order to share just one single file as a shared volume.

    After executing the previous command you should be able to see the initialization traces from the |ddsrecord|
    running in the Docker container.
    If you want to terminate the application gracefully, just press ``Ctrl+C`` to stop the execution of |ddsrecord|.
