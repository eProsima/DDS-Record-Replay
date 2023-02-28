# eProsima DDS Recorder Yaml Module

This library implements the required functions to translate a DDS Recorder configuration written in *yaml*
format into C++ source code.
It is powered by `yaml-cpp` library.

It provides methods:

- to create every type of object from a *yaml* node,
- to read a *yaml* file, and
- to interact with a *yaml* object using `YAML::Node` class from `yaml-cpp`.

---

## Example of usage

```cpp
// LOAD DDS RECORDER CONFIGURATION FROM FILE
eprosima::ddsrecorder::yaml::Configuration configuration("configuration.yaml");
```

---

## Dependencies

* `yaml-cpp`
* `cpp_utils`
* `ddspipe_core`
* `ddspipe_participants`
* `ddspipe_yaml`
* `ddsrecorder_participants`

---

## How to use it in your project

Just import library `ddsrecorder_yaml` into your CMake project.

```cmake
find_package(ddsrecorder_yaml)
target_link_libraries(${LIBRARY_TARGET_NAME} ddsrecorder_yaml)
```
