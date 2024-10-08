[![DDS Record and Replay](resources/images/github_banner_ddsrecordreplay.png)](https://eprosima.com/middleware/tools/eprosima-dds-record-replay)

<br>

<div class="menu" align="center">
    <strong>
        <a href="https://eprosima.com/index.php/downloads-all">Download</a>
        <span>&nbsp;&nbsp;•&nbsp;&nbsp;</span>
        <a href="https://dds-recorder.readthedocs.io/en/latest/">Docs</a>
        <span>&nbsp;&nbsp;•&nbsp;&nbsp;</span>
        <a href="https://eprosima.com/index.php/company-all/news">News</a>
        <span>&nbsp;&nbsp;•&nbsp;&nbsp;</span>
        <a href="https://x.com/EProsima">X</a>
        <span>&nbsp;&nbsp;•&nbsp;&nbsp;</span>
        <a href="mailto:info@eprosima.com">Contact Us</a>
    </strong>
</div>

<br><br>

<div class="badges" align="center">
    <a href="https://opensource.org/licenses/Apache-2.0"><img alt="License" src="https://img.shields.io/github/license/eProsima/DDS-Record-Replay.svg"/></a>
    <a href="https://github.com/eProsima/DDS-Record-Replay/releases"><img alt="Releases" src="https://img.shields.io/github/v/release/eProsima/DDS-Record-Replay?sort=semver"/></a>
    <a href="https://github.com/eProsima/DDS-Record-Replay/issues"><img alt="Issues" src="https://img.shields.io/github/issues/eProsima/DDS-Record-Replay.svg"/></a>
    <a href="https://github.com/eProsima/DDS-Record-Replay/network/members"><img alt="Forks" src="https://img.shields.io/github/forks/eProsima/DDS-Record-Replay.svg"/></a>
    <a href="https://github.com/eProsima/DDS-Record-Replay/stargazers"><img alt="Stars" src="https://img.shields.io/github/stars/eProsima/DDS-Record-Replay.svg"/></a>
    <br>
    <a href="https://dds-recorder.readthedocs.io"><img alt="Documentation badge" src="https://img.shields.io/readthedocs/dds-recorder.svg"/></a>
    <a href="https://github.com/eProsima/DDS-Record-Replay/actions/workflows/nightly-windows-ci.yml"><img alt="Windows CI" src="https://img.shields.io/github/actions/workflow/status/eProsima/DDS-Record-Replay/nightly-windows-ci.yml?label=Windows%20CI"></a>
    <a href="https://github.com/eProsima/DDS-Record-Replay/actions/workflows/nightly-ubuntu-ci.yml"><img alt="Ubuntu CI" src="https://img.shields.io/github/actions/workflow/status/eProsima/DDS-Record-Replay/nightly-ubuntu-ci.yml?label=Ubuntu%20CI"></a>
</div>

<br><br>

*eProsima DDS Record & Replay* is an end-user software application that efficiently saves DDS data published in a DDS environment into a MCAP format database.
Thus, the exact playback of the recorded network events is possible as the data is linked to the timestamp at which the original data was published.

*eProsima DDS Record & Replay* is easily configurable and installed with a default setup, so that DDS topics, data types and entities are automatically discovered without the need to specify the types of data recorded.
This is because the recording tool exploits the DynamicTypes functionality of [eProsima Fast DDS](https://fast-dds.docs.eprosima.com), the C++ implementation of the [DDS (Data Distribution Service) Specification](https://www.omg.org/spec/DDS/About-DDS/) defined by the [Object Management Group (OMG)](https://www.omg.org/).


## Documentation

You can access the documentation online, which is hosted on [Read the Docs](https://dds-recorder.readthedocs.io/).

* [Introduction](https://dds-recorder.readthedocs.io/en/latest/)

**Recording application**

* [Getting Started](https://dds-recorder.readthedocs.io/en/latest/rst/recording/getting_started/getting_started.html)
* [Usage](https://dds-recorder.readthedocs.io/en/latest/rst/recording/usage/usage.html)
* [Configuration](https://dds-recorder.readthedocs.io/en/latest/rst/recording/usage/configuration.html)

**Replay application**

* [Getting Started](https://dds-recorder.readthedocs.io/en/latest/rst/replaying/getting_started/getting_started.html)
* [Usage](https://dds-recorder.readthedocs.io/en/latest/rst/replaying/usage/usage.html)
* [Configuration](https://dds-recorder.readthedocs.io/en/latest/rst/replaying/usage/configuration.html)

## Getting Help

If you need support you can reach us by mail at `support@eProsima.com` or by phone at `+34 91 804 34 48`.
