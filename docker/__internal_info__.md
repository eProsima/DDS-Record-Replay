# Instructions to build the Docker image

## Branches

- Fast DDS (``)
  - Modify `recorder.repos` file
  - IntrospectionExample + DynamicTypes fixes : `experimental/filthy/complex-dynamic-data`
- dev-utils
  - Modify `recorder.repos` file
  - Tree + File + TimeToString : `feature/time_to_string`
- DDS Recorder
  - Checkout in `ddsrecorder` directory
  - Requires access to private repository
  - Tool : `feature/tool`

## Commands

Before building the Dockerfile, the DDS Recorder directory must be in directory `./ddsrecorder`

```sh
# Build docker image
docker build --rm -t ddsrecorder:figure -f Dockerfile .
```
