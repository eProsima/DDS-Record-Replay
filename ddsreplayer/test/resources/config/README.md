# RECORDING

## Run Configuration example [publisher]

```
./configuration publisher --domain 0 --samples 10 --interval 200 --wait 1 -r
```

## Run DDS Recorder

```
ddsrecorder
```

# INFORMATION

## TOPICS (CHANNELS)

configuration_topic             10

Configuration

    index 1
    message "Configuration"
    stdout: "Sample: 'Configuration' with index: '1' (10 Bytes) SENT"

    index 2
    message "Configuration"
    stdout: "Sample: 'Configuration' with index: '2' (10 Bytes) SENT"

    index 3
    message "Configuration"
    stdout: "Sample: 'Configuration' with index: '3' (10 Bytes) SENT"

    index 4
    message "Configuration"
    stdout: "Sample: 'Configuration' with index: '4' (10 Bytes) SENT"

    index 5
    message "Configuration"
    stdout: "Sample: 'Configuration' with index: '5' (10 Bytes) SENT"

    index 6
    message "Configuration"
    stdout: "Sample: 'Configuration' with index: '6' (10 Bytes) SENT"

    index 7
    message "Configuration"
    stdout: "Sample: 'Configuration' with index: '7' (10 Bytes) SENT"

    index 8
    message "Configuration"
    stdout: "Sample: 'Configuration' with index: '8' (10 Bytes) SENT"

    index 9
    message "Configuration"
    stdout: "Sample: 'Configuration' with index: '9' (10 Bytes) SENT"

    index 10
    message "Configuration"
    stdout: "Sample: 'Configuration' with index: '10' (10 Bytes) SENT"

/ddsrecorder/status             1

DdsRecorderStatus

    previous "CLOSED"
    current "RUNNING"
    info ""

## WARNING
Due to race conditions in writer-reader match, the first message may not be correctly recorded.

## START TIME
2024-08-01T11:24:35.338269976+02:00 (1722504275.338269976)

## END TIME
2024-08-01T11:24:40.357583786+02:00 (1722504280.357583786)

## DURATION
0:00:05.01931381

## COMPRESSION
zstd: [1/1 chunks] [1.35 KiB/578.00 B (58.12%)] [115.00 B/sec]
