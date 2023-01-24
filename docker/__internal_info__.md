# Instructions to build the Docker image

## Branches

- Fast DDS: `bugfix/complex-dynamic-types`
- DDS Recorder: `prototype`

## Commands

build
```sh
# Build docker image
docker build --rm -t ddsrecorder:figure -f Dockerfile .
```
