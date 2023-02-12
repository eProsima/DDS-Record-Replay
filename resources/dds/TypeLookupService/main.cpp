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

enum EntityType
{
    PUBLISHER,
    SUBSCRIBER
};

int main(
        int argc,
        char** argv)
{
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

    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Kind::Warning);

    EntityType type = PUBLISHER;
    std::string topic_name = "DDSTopic";
    DataTypeKind data_type = DataTypeKind::HELLO_WORLD;
    int count = 0;
    int domain = 0;

    long sleep = 1000; // This is not set by configuration

    if (argc > 1)
    {
        if (!strcmp(argv[1], "publisher"))
        {
            type = PUBLISHER;
        }
        else if (!strcmp(argv[1], "subscriber"))
        {
            type = SUBSCRIBER;
        }
        // check if first argument is help, needed because we skip it when parsing
        else if (!(strcmp(argv[1], "-h") && strcmp(argv[1], "--help")))
        {
            option::printUsage(fwrite, stdout, usage, columns);
            return 0;
        }
        else
        {
            std::cerr << "ERROR: first argument can only be <publisher|subscriber>" << std::endl;
            option::printUsage(fwrite, stdout, usage, columns);
            return 1;
        }

        argc -= (argc > 0);
        argv += (argc > 0); // skip program name argv[0] if present
        --argc; ++argv; // skip pub/sub argument
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
                        std::cerr << "ERROR: incorrect Data Type." << std::endl;
                        return 1;
                    }

                    break;

                case optionIndex::DOMAIN_ID:
                    domain = strtol(opt.arg, nullptr, 10);
                    break;

                case optionIndex::SAMPLES:
                    count = strtol(opt.arg, nullptr, 10);
                    break;

                case optionIndex::UNKNOWN_OPT:
                    std::cerr << "ERROR: " << opt.name << " is not a valid argument." << std::endl;
                    option::printUsage(fwrite, stdout, usage, columns);
                    return 1;
                    break;
            }
        }
    }
    else
    {
        std::cerr << "ERROR: <publisher|subscriber> argument is required." << std::endl;
        option::printUsage(fwrite, stdout, usage, columns);
        return 1;
    }

    try
    {
        switch (type)
        {
            case PUBLISHER:
            {
                // Create Publisher
                TypeLookupServicePublisher mypub(
                    topic_name,
                    static_cast<uint32_t>(domain),
                    data_type);

                // Run Participant
                mypub.run(static_cast<uint32_t>(count), static_cast<uint32_t>(sleep));
                break;
            }

            // case SUBSCRIBER:
            // {
            //     // Create Subscriber
            //     TypeLookupServiceSubscriber mysub(
            //         topic_name,
            //         static_cast<uint32_t>(domain));

            //     // Run Participant
            //     mysub.run(static_cast<uint32_t>(count));

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
