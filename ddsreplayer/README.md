<!-- TODO: update -->
# eProsima DDS Replayer Tool Module
This module create an executable that runs a DDS Replayer configured via *yaml* configuration file.

---

## Example of usage

```sh
# Source installation first. In colcon workspace: :$ source install/setup.bash

ddsrecorder --help

# Usage: DDS Replayer
# Playback traffic recorded by eProsima DDS Recorder.
# To stop the execution gracefully use SIGINT (C^) or SIGTERM (kill) signals.
# General options:

# Application help and information.
#   -h --help           Print this help message.
#   -v --version        Print version, branch and commit hash.

# Application parameters
#   -i --input-file     Path to the input MCAP File.
#   -c --config-path    Path to the Configuration File (yaml format) [Default: ./DDS_REPLAYER_CONFIGURATION.yaml].
#   -r --reload-time    Time period in seconds to reload configuration file. This is needed when FileWatcher functionality is not available (e.g. config file is a symbolic link). Value 0 does not reload file. [Default: 0].

# Debug parameters
#   -d --debug          Set log verbosity to Info
#                                              (Using this option with --log-filter and/or --log-verbosity will head to undefined behaviour).
#      --log-filter     Set a Regex Filter to filter by category the info and warning log entries. [Default = "(DDSPIPE|DDSREPLAYER)"].
#      --log-verbosity  Set a Log Verbosity Level higher or equal the one given. (Values accepted: "info","warning","error" no Case Sensitive) [Default = "warning"].
```

---

## Dependencies

* `cpp_utils`
* `ddspipe_core`
* `ddspipe_participants`
* `ddspipe_yaml`
* `ddsrecorder_participants`
* `ddsrecorder_yaml`

Only for test:

* `python`

---
