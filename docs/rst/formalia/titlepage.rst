.. raw:: html

  <h1>
    eProsima DDS Record & Replay Documentation
  </h1>

.. image:: /rst/figures/eprosima_logo.svg
  :height: 100px
  :width: 100px
  :align: left
  :alt: eProsima
  :target: http://www.eprosima.com/

*eProsima DDS Record & Replay* is an end-user software application that efficiently saves DDS data published into a DDS environment in a MCAP format database.
Thus, the exact playback of the recorded network events is possible as the data is linked to the timestamp at which the original data was published.

*eProsima DDS Record & Replay* is easily configurable and installed with a default setup, so that DDS topics, data types and entities are automatically discovered without the need to specify the types of data recorded.
This is because the recording tool exploits the DynamicTypes functionality of `eProsima Fast DDS <https://fast-dds.docs.eprosima.com>`_, the C++ implementation of the `DDS (Data Distribution Service) Specification <https://www.omg.org/spec/DDS/About-DDS/>`_ defined by the `Object Management Group (OMG) <https://www.omg.org/>`_.

########
Overview
########

*eProsima DDS Record & Replay* includes the following tools:

* **DDS Recorder tool**.
  The main functionality of this tool is to save the data in a `MCAP <https://mcap.dev/>`_ database.
  The database contains the records of the publication timestamp of the data, the serialized data, and the definition of the data serialization type and format.
  The output MCAP file can be read with any user tool compatible with MCAP file reading since it contains all the necessary information for reading and reproducing the data.

  .. figure:: /rst/figures/ddsrecord_overview.png
    :align: center

* **DDS Remote Controller tool**.
  This application allows remote control of the recording tool.
  Thus, a user can have the recording tool on a device and from another device send commands to start, stop or pause data recording.

  .. figure:: /rst/figures/controller_interact.png
    :align: center

* **DDS Replay tool**.
  This application allows to reproduce DDS traffic recorded with a *DDS Recorder*.
  A user can specify which messages to replay by either setting a time range (begin/end times) out of which messages will be discarded, or directly by blocking/whitelisting a set of topics of interest.
  It is also possible to choose a different playback rate, as well as to use topic QoS different to the ones recorded.

  .. figure:: /rst/figures/ddsreplay_overview.png
    :align: center

###############################
Contacts and Commercial support
###############################

Find more about us at `eProsima's webpage <https://eprosima.com/>`_.

Support available at:

* Email: support@eprosima.com
* Phone: +34 91 804 34 48

#################################
Contributing to the documentation
#################################

*DDS Record & Replay Documentation* is an open source project, and as such all contributions, both in the form of
feedback and content generation, are most welcomed.
To make such contributions, please refer to the
`Contribution Guidelines <https://github.com/eProsima/all-docs/blob/master/CONTRIBUTING.md>`_ hosted in our GitHub
repository.

##############################
Structure of the documentation
##############################

This documentation is organized into the sections below.

* :ref:`Installation Manual <installation_manual_linux>`
* :ref:`Recording application <recorder_getting_started_getting_started>`
* :ref:`Replay application <replayer_getting_started_getting_started>`
* :ref:`Tutorials <tutorials_foxglove>`
* :ref:`Developer Manual <developer_manual_installation_sources_linux>`
* :ref:`Release Notes <notes>`
