# eProsima DDS Recorder docker Image

This image is installed with a DDS Recorder Prototype that is able to run a `DDS Recorder` application
Fast DDS, DDS Recorder and all dependencies are already installed in the image.
In order to run the image use the following command.

---

## DDS Recorder Tool

This tool is a CLI tool with several arguments and configured by a `.yaml` file.
This tool subscribes to allowed DDS Topics and record the data received in a `.mcap` file.
The schemas to deserialize this data in FoxGlove are also written in the `.mcap` file in `.msg` ROS 2 format.
Those schemas are generated when the tool has access to the Type Object or Type Identifier of the Data Type used.

### How to retrieve Data Type to the tool

Fast DDS does not send the Data Type information by default, it must be configured to do so.
First of all, when generating the Types using Fast DDS Gen, the option `-typeobject` must be added in order to generate the needed code to fill the TypeObject data.

For native types (Data Types that does not rely in other Data Types) this is enough, as Fast DDS will send the TypeObject by default.
However, for more complex types, it is required to use `TypeInformation` mechanism.
In the Fast DDS `DomainParticipant` set the following QoS in order to send this information:

```cpp
DomainParticipantQos pqos;
pqos.wire_protocol().builtin.typelookup_config.use_server = true;
```

---

## Run DDS Recorder

There are some configurations already available in the container under directory `/ddsrecorder/resources/`

- `simple_configuration.yaml` Configuration with just the basics to run the executable.
- `complete_configuration.yaml` Configuration with all the possible configurations available.

In order to execute the `DDS Recorder` use the following command:
```bash
ddsrecorder --config-path /ddsrecorder/resources/<configuration>.yaml
```

In order to know all the possible arguments supported by this tool, use the command:
```bash
ddsrecorder --help` or `ddsrecorder -h
```

In order to see further information and debugging info about what the tool is executing, use the argument `--debug`:
```bash
ddsrecorder --config-path /ddsrecorder/resources/<configuration>.yaml --debug
```

### Use Custom Configurations

There are 2 ways to write a custom configuration:

1. Modify a `.yaml` file within the container instance.
2. Using shared volumes to share the `.yaml` configuration file.

### Run with shared volume

In order to automatically retrieve every `.mcap` file generated inside the container, use a docker volume.
Run the following command to share your local `<shared/folder> and container `<shared/folder>`.

```bash
docker run -it --net=host --ipc=host --privileged --volume $(pwd)/<shared/folder>/:<shared/folder>  ddsrecorder:v0.1.0
```


### Connectivity issues

- `--net=host` allow the DDS Recorder to connect with external (different device) participants.
- `--ipc=host` allow the DDS Recorder to use Shared Memory Transport with the participants in the same host.

If local Participants (same host) are unable to connect with the DDS Recorder inside a Docker, it may be because they try to use Shared Memory, but the docker has no access to the same shared segment.
To avoid this, run the other participants as `root` or change the docker image user name to be the same as the external participants one.
Other option may be to not using Shared Memory by disabling it by Fast DDS configuration (by XML or QoS in code) or by CMake option when compiling `-DSHM_TRANSPORT_DEFAULT=ON`.

---

## Configuration

This first version does support the following configurations:

|               | Description                                                 | Type           | Default   |
|---------------|-------------------------------------------------------------|----------------|-----------|
| allowlist     | List of topics that are going to be recorded                | List of topics | Empty     |
| blocklist     | List of topics that are **not** going to be recorded        | List of topics | Empty     |
| domain        | DDS Domain to discover and subscribe to topics allowed      | integer        | 0         |
| extension     | File extension for the result file                          | string         | .mcap     |
| filename      | File name for the result file                               | string         | MANDATORY |
| path          | Path to result file                                         | string         | ./        |
| use-timestamp | Whether to add or not the timestamp to the result file name | bool           | true      |

The topics in `allowlist` and `blocklist` are filled with elements with field `name` referring to the Topic name.
Optionally each element can have the element `type` referring to the Topic Data Type name.
Both name and allow wildcards (`*`).
