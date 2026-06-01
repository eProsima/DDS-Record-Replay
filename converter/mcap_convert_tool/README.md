# eProsima MCAP Convert Tool Module
This module creates the standalone `mcap-convert` executable used to convert MCAP recordings into the SQLite `.db` format used by DDS Record & Replay.

---

## Example of usage

```sh
# Source installation first. In colcon workspace: :$ source install/setup.bash

mcap-convert --help

# Convert using the default output path <input>.db
mcap-convert -i /path/to/recording.mcap

# Convert using an explicit output path
mcap-convert -i /path/to/recording.mcap --sql-output /path/to/recording.db
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
