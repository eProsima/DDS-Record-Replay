# eProsima DDS Recorder Tool Module

This module creates the `ddsrecorder` executable, which records DDS traffic to
MCAP and/or SQLite from a YAML configuration.

---

## Example of usage

```bash
source install/setup.bash
ddsrecorder --help
ddsrecorder --config-path recorder.yaml
```

See the [Recorder user guide](../docs/rst/recording/usage/usage.rst) and
[configuration reference](../docs/rst/recording/usage/configuration.rst).

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
