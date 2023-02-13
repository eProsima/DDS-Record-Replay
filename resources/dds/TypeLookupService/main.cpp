// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file main.cpp
 *
 */

#include <string>

#include "arg_configuration.h"
#include "TypeLookupServicePublisher.h"
// #include "TypeLookupServiceSubscriber.h"

//! Enumeration to define the DDS entity type to be executed in the example
enum EntityType
{
    PUBLISHER,
    SUBSCRIBER
};

int main(
        int argc,
        char** argv)
{
    // Help message formatting settings
    int columns;

#if defined(_WIN32)
    char* buf = nullptr;
    size_t sz = 0;
    if (_dupenv_s(&buf, &sz, "COLUMNS") == 0 && buf != nullptr)
    {
        columns = strtol(buf, nullptr, 10);
        free(buf);
    }
    else
    {
        columns = 80;
    }
#else
    columns = getenv("COLUMNS") ? atoi(getenv("COLUMNS")) : 80;
#endif // if defined(_WIN32)

    // Set Fast DDS logging verbosity level to Warning
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Kind::Warning);

    // Examples parameter definition
    EntityType entity_type = EntityType::PUBLISHER;
    std::string topic_name = "DDSTopic";
    DataTypeKind data_type = DataTypeKind::HELLO_WORLD;
    int samples = 0;
    int domain = 0;
    long sleep = 1000;

    // Parse example options
    argc -= (argc > 0);
    argv += (argc > 0); // skip program name argv[0] if present
    option::Stats stats(usage, argc, argv);
    std::vector<option::Option> options(stats.options_max);
    std::vector<option::Option> buffer(stats.buffer_max);
    option::Parser parse(usage, argc, argv, &options[0], &buffer[0]);

    if (parse.error())
    {
        option::printUsage(fwrite, stdout, usage, columns);
        return 1;
    }

    if (options[optionIndex::HELP])
    {
        option::printUsage(fwrite, stdout, usage, columns);
        return 0;
    }

    for (int i = 0; i < parse.optionsCount(); ++i)
    {
        option::Option& opt = buffer[i];
        switch (opt.index())
        {
            case optionIndex::HELP:
                // not possible, because handled further above and exits the program
                break;

            case optionIndex::ENTITY_TYPE:
                if (strcmp(opt.arg, PUBLISHER_ENTITY_KIND_ARG) == 0)
                {
                    entity_type = EntityType::PUBLISHER;
                }
                else if (strcmp(opt.arg, SUBSCRIBER_ENTITY_KIND_ARG) == 0)
                {
                    entity_type = EntityType::SUBSCRIBER;
                }
                else
                {
                    std::cerr << "ERROR: incorrect entity type. Only <publisher|subscriber> accepted." << std::endl;
                    return 1;
                }

            case optionIndex::TOPIC_NAME:
                topic_name = std::string(opt.arg);
                break;

            case optionIndex::DATA_TYPE:
                if (strcmp(opt.arg, HELLO_WORLD_DATA_TYPE_ARG) == 0)
                {
                    data_type = DataTypeKind::HELLO_WORLD;
                }
                else if (strcmp(opt.arg, COMPLETE_DATA_TYPE_ARG) == 0)
                {
                    data_type = DataTypeKind::COMPLETE;
                }
                else
                {
                    std::cerr << "ERROR: incorrect data type. Only <helloworld|complete> accepted." << std::endl;
                    return 1;
                }

                break;

            case optionIndex::DOMAIN_ID:
                domain = strtol(opt.arg, nullptr, 10);
                break;

            case optionIndex::SAMPLES:
                samples = strtol(opt.arg, nullptr, 10);
                break;

            case optionIndex::INTERVAL:
                sleep = strtol(opt.arg, nullptr, 10);
                break;

            case optionIndex::UNKNOWN_OPT:
                std::cerr << "ERROR: " << opt.name << " is not a valid argument." << std::endl;
                option::printUsage(fwrite, stdout, usage, columns);
                return 1;
                break;
        }
    }

    // Build and run the publisher or subscriber examples
    try
    {
        switch (entity_type)
        {
            case PUBLISHER:
            {
                // Create Publisher
                TypeLookupServicePublisher mypub(
                    topic_name,
                    static_cast<uint32_t>(domain),
                    data_type);

                // Run Participant
                mypub.run(static_cast<uint32_t>(samples), static_cast<uint32_t>(sleep));
                break;
            }

            // case SUBSCRIBER:
            // {
            //     // Create Subscriber
            //     TypeLookupServiceSubscriber mysub(
            //         topic_name,
            //         static_cast<uint32_t>(domain));

            //     // Run Participant
            //     mysub.run(static_cast<uint32_t>(samples));

            //     break;
            // }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "Execution failed with error:\n " << e.what() << std::endl;
    }

    return 0;
}
