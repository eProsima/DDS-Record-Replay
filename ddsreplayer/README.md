# eProsima DDS Replayer Tool Module

This module creates the `ddsreplayer` executable, which replays DDS traffic
from a finalized MCAP or DDS Record & Replay SQLite file.

MCAP-to-SQL conversion is exposed as the standalone `mcap-convert` tool and is no longer part of the `ddsreplayer` CLI.

---

## Example of usage

```bash
source install/setup.bash
ddsreplayer --help
ddsreplayer --input recording.mcap
```

See the [Replayer user guide](../docs/rst/replaying/usage/usage.rst) and
[configuration reference](../docs/rst/replaying/usage/configuration.rst).

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
