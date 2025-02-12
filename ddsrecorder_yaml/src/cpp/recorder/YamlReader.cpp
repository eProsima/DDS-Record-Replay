// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <set>

#include <mcap/mcap.hpp>

#include <ddspipe_yaml/YamlReader.hpp>
#include <ddspipe_yaml/YamlValidator.hpp>

#include <ddsrecorder_yaml/recorder/yaml_configuration_tags.hpp>

namespace eprosima {
namespace ddspipe {
namespace yaml {

using namespace eprosima::ddsrecorder::yaml;

template <>
mcap::McapWriterOptions
YamlReader::get<mcap::McapWriterOptions>(
        const Yaml& yml,
        const YamlReaderVersion version)
{
    static const std::set<TagType> tags{
        RECORDER_COMPRESSION_SETTINGS_ALGORITHM_TAG,
        RECORDER_COMPRESSION_SETTINGS_LEVEL_TAG,
        RECORDER_COMPRESSION_SETTINGS_FORCE_TAG,
    };

    YamlValidator::validate_tags(yml, tags);

    mcap::McapWriterOptions mcap_writer_options{"ros2"};

    // Parse optional compression algorithm
    if (YamlReader::is_tag_present(yml, RECORDER_COMPRESSION_SETTINGS_ALGORITHM_TAG))
    {
        auto algorithm_yml = YamlReader::get_value_in_tag(yml, RECORDER_COMPRESSION_SETTINGS_ALGORITHM_TAG);
        mcap_writer_options.compression = YamlReader::get_enumeration<mcap::Compression>(algorithm_yml,
                    {
                        {RECORDER_COMPRESSION_SETTINGS_ALGORITHM_NONE_TAG, mcap::Compression::None},
                        {RECORDER_COMPRESSION_SETTINGS_ALGORITHM_LZ4_TAG, mcap::Compression::Lz4},
                        {RECORDER_COMPRESSION_SETTINGS_ALGORITHM_ZSTD_TAG, mcap::Compression::Zstd},
                    });
    }

    // Parse optional compression level
    if (YamlReader::is_tag_present(yml, RECORDER_COMPRESSION_SETTINGS_LEVEL_TAG))
    {
        auto level_yml = YamlReader::get_value_in_tag(yml, RECORDER_COMPRESSION_SETTINGS_LEVEL_TAG);
        mcap_writer_options.compressionLevel = YamlReader::get_enumeration<mcap::CompressionLevel>(level_yml,
                    {
                        {RECORDER_COMPRESSION_SETTINGS_LEVEL_FASTEST_TAG, mcap::CompressionLevel::Fastest},
                        {RECORDER_COMPRESSION_SETTINGS_LEVEL_FAST_TAG, mcap::CompressionLevel::Fast},
                        {RECORDER_COMPRESSION_SETTINGS_LEVEL_DEFAULT_TAG, mcap::CompressionLevel::Default},
                        {RECORDER_COMPRESSION_SETTINGS_LEVEL_SLOW_TAG, mcap::CompressionLevel::Slow},
                        {RECORDER_COMPRESSION_SETTINGS_LEVEL_SLOWEST_TAG, mcap::CompressionLevel::Slowest},
                    });
    }

    // Parse optional compression force
    if (YamlReader::is_tag_present(yml, RECORDER_COMPRESSION_SETTINGS_FORCE_TAG))
    {
        mcap_writer_options.forceCompression = YamlReader::get<bool>(yml, RECORDER_COMPRESSION_SETTINGS_FORCE_TAG,
                        version);
    }

    return mcap_writer_options;
}

} /* namespace yaml */
} /* namespace ddspipe */
} /* namespace eprosima */
