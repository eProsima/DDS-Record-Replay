.. include:: ../exports/alias.include
.. include:: ../exports/roles.include

.. _docker:

############
Docker Image
############

eProsima no longer distributes a standalone Docker image of |ddsrecord|. However, the *Fast DDS Suite* Docker image 
is provided for those who want a Docker image with a set of eProsima's tools and libraries running on an Ubuntu platform.
It can be downloaded from `eProsima's Downloads Page <https://www.eprosima.com/index.php/downloads-all>`__.

This image contains the full *Fast DDS Suite*, and an instance of |ddsrecord| can be configured using a *YAML* configuration
file provided by the user and shared with the Docker container.
The steps to run |ddsrecord| in a Docker container are explained below.

#.  Download the compressed Docker image in ``.tar`` format from the
    `eProsima's Downloads Page <https://www.eprosima.com/index.php/downloads-all>`__ and load it into your local Docker running the
    following command in a terminal:

    .. code-block:: bash

        docker load -i "ubuntu-fastdds-suite_<fastdds-version>.tar"

    where ``fastdds-version`` is the downloaded version of |efastdds|.

    |br|

#.  Build a |ddsrecord| configuration YAML file on the local machine.
    This will be the |ddsrecord| configuration file that runs inside the Docker container.
    To continue this installation manual, let's use the configuration file provided in :ref:`this tutorial <tutorials_dds_monitor>`.
    Open your preferred text editor and copy the configuration example from :ref:`here <tutorials_dds_monitor_configuring_recorder>`
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
            ubuntu-fastdds-suite:<fastdds-version> ddsrecorder

    It is important to mention that both the path to the configuration file hosted in the local machine and the one
    created in the Docker container must be absolute paths in order to share just one single file as a shared volume.

    After executing the previous command you should be able to see the initialization traces from the |ddsrecord|
    running in the Docker container.
    If you want to terminate the application gracefully, just press ``Ctrl+C`` to stop the execution of |ddsrecord|.
