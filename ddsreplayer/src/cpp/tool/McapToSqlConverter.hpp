// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file McapToSqlConverter.hpp
 */

#pragma once

#include <string>

#include <ddsrecorder_yaml/replayer/YamlReaderConfiguration.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace replayer {

/**
 * @brief Convert an MCAP recording into the SQLite schema used by DDS Record & Replay
 *
 * The converter is intentionally built as a thin orchestration layer over existing code:
 * - it reuses \c McapReaderParticipant to read MCAP summaries, metadata, topics, partitions,
 *   and message streams
 * - it reuses the dynamic-type registration/deserialization path extracted from
 *   \c DdsReplayer
 * - it reuses \c SqlWriter and \c SqlMessage to persist the same SQL schema already used by
 *   recorder-side SQL output
 *
 * The high-level workflow is:
 * - load the MCAP summary and dynamic-type attachment
 * - register/build Fast DDS dynamic types when available
 * - iterate MCAP messages in log-time order
 * - resolve topic metadata and writer information from MCAP metadata
 * - write SQL rows with original sequence numbers and timestamps
 * - serialize JSON when type information is available, otherwise keep CDR-only fallback rows
 */
class McapToSqlConverter
{
public:

    /**
     * @brief Create a converter for a given MCAP input and SQL output
     *
     * @param configuration Replayer configuration used for logging and reader-side filters.
     * @param input_file Path to the input MCAP file.
     * @param output_file Requested output SQL/SQLite file path. If empty, a default path is derived
     *        from \c input_file.
     */
    McapToSqlConverter(
            const yaml::ReplayerConfiguration& configuration,
            const std::string& input_file,
            const std::string& output_file = "");

    /**
     * @brief Execute the MCAP-to-SQL conversion
     *
     * @throw utils::InitializationException If the input file or output location cannot be initialized
     * @throw utils::InconsistencyException If SQL writing fails
     *
     * @note This method performs conversion only; it does not create DDS replay participants
     *       nor publish recorded traffic
     */
    void convert();

    /**
     * @brief Resolve the output SQL/SQLite path to use for a conversion
     *
     * @param input_file Input MCAP path
     * @param output_file Optional explicit output path from the command line
     *
     * @return The effective output path. When \c output_file is empty, the result is
     *         \c <input_file_stem>.db . When \c output_file has no extension, \c .db is appended
     */
    static std::string resolve_output_file(
            const std::string& input_file,
            const std::string& output_file = "");

protected:
    const yaml::ReplayerConfiguration& configuration_;
    const std::string input_file_;
    const std::string output_file_;
};

} /* namespace replayer */
} /* namespace ddsrecorder */
} /* namespace eprosima */
