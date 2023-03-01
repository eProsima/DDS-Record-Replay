# eProsima DDS Recorder

<a href="http://www.eprosima.com"><img src="https://encrypted-tbn3.gstatic.com/images?q=tbn:ANd9GcSd0PDlVz1U_7MgdTe0FRIWD0Jc9_YH-gGi0ZpLkr-qgCI6ZEoJZ5GBqQ" align="left" hspace="8" vspace="2" width="100" height="100" ></a>

[![License](https://img.shields.io/github/license/eProsima/DDS-Recorder.svg)](https://opensource.org/licenses/Apache-2.0)
[![Issues](https://img.shields.io/github/issues/eProsima/DDS-Recorder.svg)](https://github.com/eProsima/DDS-Recorder/issues)
[![Forks](https://img.shields.io/github/forks/eProsima/DDS-Recorder.svg)](https://github.com/eProsima/DDS-Recorder/network/members)
[![Stars](https://img.shields.io/github/stars/eProsima/DDS-Recorder.svg)](https://github.com/eProsima/DDS-Recorder/stargazers)
[![test](https://github.com/eProsima/DDS-Recorder/actions/workflows/test.yml/badge.svg)](https://github.com/eProsima/DDS-Recorder/actions/workflows/test.yml)

*eProsima DDS Record* is an end-user software application that efficiently saves DDS data published in a DDS environment in a MCAP format database.
Thus, the exact playback of the recorded network events is possible as the data is linked to the timestamp at which the original data was published.
At the moment, it is only possible to replay the data using external tools capable of interpreting the MCAP format, as the *eProsima DDS Record* does not provide a replay tool.

*eProsima DDS Record* is easily configurable and installed with a default setup, so that DDS topics, data types and entities are automatically discovered without the need to specify the types of data recorded.
This is because the recording tool exploits the DynamicTypes functionality of [eProsima Fast DDS](https://fast-dds.docs.eprosima.com), the C++ implementation of the [DDS (Data Distribution Service) Specification](https://www.omg.org/spec/DDS/About-DDS/) defined by the [Object Management Group (OMG)](https://www.omg.org/).


## Documentation

You can access the documentation online, which is hosted on [Read the Docs](https://dds-recorder.readthedocs.io/).

* [Introduction](https://dds-recorder.readthedocs.io/en/latest/)

**Recording application**

* [Getting Started](https://dds-recorder.readthedocs.io/en/latest/rst/recording/getting_started/getting_started.html)
* [Usage](https://dds-recorder.readthedocs.io/en/latest/rst/recording/usage/usage.html)
* [Tutorials](https://dds-recorder.readthedocs.io/en/latest/rst/recording/tutorials/tutorials.html)

## Getting Help

If you need support you can reach us by mail at `support@eProsima.com` or by phone at `+34 91 804 34 48`.
