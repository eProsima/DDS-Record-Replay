# Fast DDS Type Lookup Service (DynamicTypes) example

This examples configures a DDS Publisher using DynamicTypes for the data type definition.
Thus, the user will be able to record the published data using the *eProsima DDS Recorder* tool.
Moreover, the example implements a DDS Subscriber that will receive any kind of data published by the Publisher
as long as the Publisher sends the data type information.

This example covers the following points:

* How to configure DynamicTypes on Fast DDS.
* How to instantiate a DynamicType on the publisher side.
* How to fill the DynamicData on the publisher side with valuable information.
* How to use IDL files to define the type and use Fast-DDS-Gen to generate the C++ source code to work with the type.
* How to create a generic Subscriber able to subscribe to any incoming data type using DynamicTypes.


## Building the example

To build the example, install Fast DDS following one of the [installation methods](https://fast-dds.docs.eprosima.com/en/latest/installation/binaries/binaries_linux.html) described in its documentation, or by [installing DDS Record & Replay](https://dds-recorder.readthedocs.io/en/latest/rst/installation/windows.html) together with eProsima dependencies, i.e. Fast DDS, Fast CDR and DDS Pipe.

Once Fast DDS is installed, source Fast DDS or make the libraries accessible to be used by CMake.

Finally, go to this folder and run:

```bash
mkdir build
cd build
cmake .. && make -j8
```

## Recording samples with DDS Recorder

1.  Open two terminals
1.  In the first terminal, run this example

    ```bash
    ./TypeLookupService
    ```

1.  In the second terminal, run the DDS Recorder.
    Please, go to [DDS Record & Replay documentation](https://dds-recorder.readthedocs.io/en/latest/index.html) to learn how to properly install and run DDS Recorder.

    ```bash
    ddsrecorder
    ```

## Publisher <-> Subscriber example

1.  Open two different terminals:

1.  In the first terminal, run:

    ```bash
    ./TypeLookupService --entity publisher
    ```

1.  In the second terminal, run:

    ```bash
    ./TypeLookupService --entity subscriber
    ```

## Arguments

See the output of running the executable with `--help` argument to check the example valid configurations.

```sh
Usage: TypeIntrospectionExample

General options:
  -h, --help                        Produce help message.
  -e, --entity <dds_entity>         DDS Entity type (Default: publisher).
                                    Allowed options:
                                    • publisher -> Run a DDS Publisher.
                                    • subscriber -> Run a DDS Subscriber.

Publisher options:
  -t, --topic <topic_name>          Topic name (Default: /dds/topic).
  -x, --type <data_type_name>       Topic Data Type name (Default: helloworld).
                                    • helloworld -> HelloWorld data type
                                    (one string and one integer).
                                    • complete -> Complex data type composed of
                                    several of the other types at multiple levels.
  -d, --domain <id>                 DDS domain ID (Default: 0).
  -s, --samples <num>               Number of samples to send (Default: 0 => infinite).
  -i, --interval <num>              Time between samples in milliseconds (Default: 1000).

Subscriber options:
  -t, --topic <topic_name>          Topic name (Default: /dds/topic).
  -d, --domain <id>                 DDS domain ID (Default: 0).
  -s, --samples <num>               Number of samples to wait for (Default: 0 => infinite).

```

### Publisher Data Type

This example allows the user to configure the type of the data published.
At the moment, there are two data types that can be used in this example:

*   `helloworld`

    ```idl
    struct HelloWorld
    {
        unsigned long index;
        string message;
    };
    ```

*   `complete`

    ```bash
    struct Timestamp
    {
        long seconds;
        long milliseconds;
    };

    struct Point
    {
        long x;
        long y;
        long z;
    };

    struct MessageDescriptor
    {
        unsigned long id;
        string topic;
        Timestamp time;
    };

    struct Message
    {
        MessageDescriptor descriptor;
        string message;
    };

    struct CompleteData
    {
        unsigned long index;
        Point main_point;
        sequence<Point> internal_data;
        Message messages[2];
    };
    ```

### Subscriber Data Type

The Subscriber will detect the data type name and will register it using the type information sent by the
publisher. Thus, the subscriber does not need to know the type.
In order to look-up a data type, just launch the Subscriber setting the same topic name as the one configured in the
publisher side.

---

## Useful Commands

### Generate type information code using Fast DDS Gen

```sh
fastddsgen -typeobject MyType.idl
```
Please, find more Fast-DDS-Gen options [here](https://fast-dds.docs.eprosima.com/en/latest/fastddsgen/usage/usage.html).
