<!-- TODO: revisit -->
# eProsima Foxglove Websocket Yaml Module

This library implements the required functions to translate a Foxglove Websocket configuration written in *yaml*
format into C++ source code.
It is powered by `yaml-cpp` library.

It provides methods:

- to create every type of object from a *yaml* node,
- to read a *yaml* file, and
- to interact with a *yaml* object using `YAML::Node` class from `yaml-cpp`.

---

## Example of usage

```cpp
// LOAD DDS ROUTER CONFIGURATION FROM FILE

core::configuration::DDSRouterConfiguration router_configuration =
                yaml::YamlReaderConfiguration::load_ddsrouter_configuration_from_file("configuration.yaml");
```

---

## Dependencies

* `yaml-cpp`
* `cpp_utils`
* `ddsrouter_core`
* `ddsrouter_yaml`
* `foxgloveweb_participants`


---

## How to use it in your project

Just import library `foxgloveweb_yaml` into your CMake project.

```cmake
find_package(foxgloveweb_yaml)
target_link_libraries(${LIBRARY_TARGET_NAME} foxgloveweb_yaml)
```
