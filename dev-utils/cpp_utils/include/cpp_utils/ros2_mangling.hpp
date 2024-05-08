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

#pragma once

#include <string>
#include <vector>

#include <cpp_utils/library/library_dll.h>

namespace eprosima {
namespace utils {

/**
 * @brief Remove a specified prefix from a string.
 *
 * This function takes a string and a prefix as input. It checks if the string starts with the specified prefix
 * followed by a forward slash ('/'). If the prefix exists at the beginning of the string, it removes the prefix
 * and the following slash and returns the modified string. If the prefix is not found at the beginning of the
 * string, an empty string is returned.
 *
 * @param[in] name The input string from which the prefix will be removed.
 * @param[in] prefix The prefix to be removed from the input string.
 *
 * @return The modified string with the specified prefix removed, or an empty string if the prefix is not present
 * at the beginning of the input string.
 */
CPP_UTILS_DllAPI
std::string remove_prefix(
        const std::string& name,
        const std::string& prefix);

/**
 * @brief Add a specified prefix to a string.
 *
 * This function takes a string and a prefix as input. It concatenates the prefix with the input string
 * and returns the resulting string with the prefix added.
 *
 * @param[in] name The input string to which the prefix will be added.
 * @param[in] prefix The prefix to be added to the input string.
 *
 * @return The modified string with the specified prefix added.
 */
CPP_UTILS_DllAPI
std::string add_prefix(
        const std::string& name,
        const std::string& prefix);

/**
 * @brief Add a specified suffix to a string.
 *
 * This function takes a string and a suffix as input. It concatenates the suffix with the input string
 * and returns the resulting string with the suffix added.
 *
 * @param[in] name The input string to which the suffix will be added.
 * @param[in] suffix The suffix to be added to the input string.
 *
 * @return The modified string with the specified suffix added.
 */
CPP_UTILS_DllAPI
std::string add_suffix(
        const std::string& name,
        const std::string& suffix);

/**
 * @brief Get a ROS prefix if it exists at the beginning of a topic name.
 *
 * This function takes a topic name as input and checks if it starts with any of the ROS prefixes.
 * If a matching ROS prefix is found at the beginning of the topic name, the prefix is returned.
 * If no matching prefix is found, an empty string is returned.
 *
 * @param[in] topic_name The input topic name to be checked for a ROS prefix.
 *
 * @return The ROS prefix found at the beginning of the topic name, or an empty string if no prefix exists.
 */
CPP_UTILS_DllAPI
std::string get_ros_prefix_if_exists(
        const std::string& topic_name);

/**
 * @brief Remove a ROS prefix if it exists at the beginning of a topic name.
 *
 * This function takes a topic name as input and checks if it starts with any of the ROS prefixes.
 * If a matching ROS prefix is found at the beginning of the topic name, the prefix is removed,
 * and the modified topic name is returned. If no matching prefix is found, the original topic name is returned unchanged.
 *
 * @param[in] topic_name The name of the topic to be processed.
 *
 * @return The topic name with the ROS prefix removed, or the original topic name if no prefix exists.
 */
CPP_UTILS_DllAPI
std::string remove_ros_prefix_if_exists(
        const std::string& topic_name);

/**
 * @brief Add the ROS topic prefix to a given topic name.
 *
 * This function takes a topic name as input and adds the ROS topic prefix to it.
 * The modified topic name with the prefix added is returned.
 *
 * @param[in] topic_name The input topic name to which the ROS topic prefix will be added.
 *
 * @return The modified topic name with the ROS topic prefix added.
 */
CPP_UTILS_DllAPI
std::string add_ros_topic_prefix(
        const std::string& topic_name);

/**
 * @brief Add the ROS service requester prefix to a given topic name.
 *
 * This function takes a topic name as input and adds the ROS service requester prefix to it.
 * The modified topic name with the prefix added is returned.
 *
 * @param[in] topic_name The input topic name to which the ROS service requester prefix will be added.
 *
 * @return The modified topic name with the ROS service requester prefix added.
 */
CPP_UTILS_DllAPI
std::string add_ros_service_requester_prefix(
        const std::string& topic_name);

/**
 * @brief Add the ROS service response prefix to a given topic name.
 *
 * This function takes a topic name as input and adds the ROS service response prefix to it.
 * The modified topic name with the prefix added is returned.
 *
 * @param[in] topic_name The input topic name to which the ROS service response prefix will be added.
 *
 * @return The modified topic name with the ROS service response prefix added.
 */
CPP_UTILS_DllAPI
std::string add_ros_service_response_prefix(
        const std::string& topic_name);

/**
 * @brief Get a reference to the collection of all ROS prefixes.
 *
 * This function returns a reference to the collection of ROS prefixes stored in the `ros_prefixes_` variable.
 *
 * @return A reference to the collection of all ROS prefixes.
 */
CPP_UTILS_DllAPI
const std::vector<std::string>& get_all_ros_prefixes();

/**
 * @brief Demangle a ROS topic name by removing the ROS prefix if it exists.
 *
 * This function takes a ROS topic name as input and attempts to demangle it by removing the ROS prefix.
 * If the input topic name contains a ROS prefix, it is removed. If there is no ROS prefix, the topic name
 * is returned unchanged.
 *
 * @param[in] topic_name The name of the topic to be processed.
 *
 * @return Returns the demangle ROS topic or the original if not a ROS topic.
 */
CPP_UTILS_DllAPI
std::string demangle_if_ros_topic(
        const std::string& topic_name);

/**
 * @brief Mangle a given \c topic_name if it starts with "/".
 *
 * If the topic name begins with "/", it adds the ROS topic prefix to the \c topic_name and then returns it.
 * If the topic name does not start with "/", it is returned unchanged.
 *
 * @param[in] topic_name The name of the topic to be processed.
 *
 * @return Returns the mangle ROS topic or the original if not a ROS topic (start with "/").
 */
CPP_UTILS_DllAPI
std::string mangle_if_ros_topic(
        const std::string& topic_name);

/**
 * @brief Demangle a DDS type string if it is a ROS type.
 *
 * This function takes a DDS type string as input and checks if it represents a ROS type.
 * If the input type string is not a ROS type, it is returned unchanged.
 * If the input type string is a ROS type, it demangles the type string by converting DDS-specific namespace
 * separators ("::") to "/", removing "dds" and returns the demangled result.
 *
 * @param[in] dds_type_string DDS type string to be processed.
 *
 * @return The demangled ROS type string or the original input if it is not a ROS type.
 */
CPP_UTILS_DllAPI
std::string demangle_if_ros_type(
        const std::string& dds_type_string);

/**
 * @brief Mangle a ROS 2 type string into a DDS type string.
 *
 * This function takes a ROS 2 type string as input and mangles it into a DDS type string.
 * If the input string does not contain a ROS 2-specific substring, it is returned unchanged.
 * If the ROS 2 substring is found, this function extracts the type namespace and type name,
 * converts namespace separators from "/" to "::", and adds the "dds_::" prefix and a trailing
 * underscore to the type name.
 *
 * @param[in] ros2_type_string The input ROS 2 type string to be mangled.
 *
 * @return The mangled DDS type string or the original input if it does not contain the ROS 2 substring.
 */
CPP_UTILS_DllAPI
std::string mangle_if_ros_type(
        const std::string& ros2_type_string);

/**
 * @brief Demangle a ROS topic name by removing the ROS topic prefix.
 *
 * This function takes a ROS topic name as input and removes the ROS topic prefix if it exists.
 *
 * @param[in] topic_name The input ROS topic name to be processed.
 *
 * @return The demangled ROS topic name with the ROS topic prefix removed, or "" if the prefix is not present.
 */
CPP_UTILS_DllAPI
std::string demangle_ros_topic_prefix_from_topic(
        const std::string& topic_name);

/**
 * @brief Demangle a ROS service topic name by identifying its type and extracting the appropriate service name.
 *
 * This function takes a ROS service topic name as input and attempts to demangle it by identifying its type
 * (whether it's a reply or request service topic).
 * If neither function successfully demangles the input, an empty string is returned.
 *
 * @param[in] topic_name The input ROS service topic name to be processed.
 *
 * @return The demangled service name if successfully extracted, or an empty string if demangling fails.
 */
CPP_UTILS_DllAPI
std::string demangle_ros_service_prefix_from_topic(
        const std::string& topic_name);
/**
 * @brief Demangle a ROS service request topic name by removing the requester prefix and "Request" suffix.
 *
 * This function takes a ROS service request topic name as input and removes the specified requester prefix and the common "Request" suffix.
 * If the input topic name does not contain both the requester prefix
 * and the "Request" suffix, an empty string is returned.
 *
 * @param[in] topic_name The input ROS service request topic name to be processed.
 *
 * @return The demangled service name with the requester prefix and "Request" suffix removed, or an empty string
 * if the necessary prefixes and suffixes are not present.
 */
CPP_UTILS_DllAPI
std::string demangle_ros_service_request_prefix_from_topic(
        const std::string& topic_name);

/**
 * @brief Mangle a ROS service request topic name by adding the requester prefix and "Request" suffix.
 *
 * This function takes a ROS service request topic name as input and mangles it by adding the specified
 * requester prefix and the common "Request" suffix. If the input topic name starts with "/", indicating
 * it is part of a service request, the mangled service name is returned. Otherwise, an empty string is returned.
 *
 * @param[in] topic_name The input ROS service request topic name to be processed.
 *
 * @return The mangled service name with the requester prefix and "Request" suffix added if
 * it is part of a service request (start with "/"), otherwise an empty string ("").
 */
CPP_UTILS_DllAPI
std::string mangle_ros_service_request_prefix_in_topic(
        const std::string& topic_name);

/**
 * @brief Demangle a ROS service reply topic name by removing the response prefix and "Reply" suffix.
 *
 * This function takes a ROS service reply topic name as input and demangles it by removing the specified
 * response prefix and the common "Reply" suffix.
 * If the input topic name does not contain both the response prefix and the "Reply" suffix, an empty string is returned.
 *
 * @param[in] topic_name The input ROS service reply topic name to be processed.
 *
 * @return The demangled service name with the response prefix and "Reply" suffix removed, or an empty string
 * if the necessary prefixes and suffixes are not present.
 */
CPP_UTILS_DllAPI
std::string demangle_ros_service_reply_prefix_from_topic(
        const std::string& topic_name);

/**
 * @brief Mangle a ROS service reply topic name by adding the response prefix and "Reply" suffix.
 *
 * This function takes a ROS service reply topic name as input and mangles it by adding the specified
 * response prefix and the common "Reply" suffix. If the input topic name does not start with "/", indicating
 * it is part of a service response, an empty string is returned.
 *
 * @param[in] topic_name The input ROS service reply topic name to be processed.
 *
 * @return The mangled service name with the response prefix and "Reply" suffix added if it is part of
 * a service response (starts with "/"), otherwise an empty string ("").
 */
CPP_UTILS_DllAPI
std::string mangle_ros_service_reply_prefix_in_topic(
        const std::string& topic_name);

/**
 * @brief Demangle a ROS service type name to extract the core type in ROS 2 format.
 *
 * This function takes a ROS service type name in DDS format as input and demangles it to the ROS 2 format,
 * extracting the core type. If the input type name does not match the expected ROS service type format,
 * an empty string is returned.
 *
 * @param[in] dds_type_name The input ROS service type name in DDS format to be processed.
 *
 * @return The demangled core service type name in ROS 2 format, or an empty string if the input
 * does not conform to the expected format.
 */
CPP_UTILS_DllAPI
std::string demangle_service_type_only(
        const std::string& dds_type_name);

/**
 * @brief Mangle a ROS 2 service type name to DDS format.
 *
 * This function takes a ROS 2 service type name as input and mangles it to DDS format by adding necessary
 * prefixes and separators. It identifies whether it's a service request or response type and handles the
 * conversion accordingly. If the input type name is not in the expected ROS 2 service type format, an empty
 * string is returned.
 *
 * @param[in] ros2_type_name The input ROS 2 service type name to be processed.
 *
 * @return The mangled ROS 2 service type name in DDS format, or an empty string if the input does not
 * conform to the expected format.
 */
CPP_UTILS_DllAPI
std::string mangle_service_type_only(
        const std::string& ros2_type_name);

} /* namespace utils */
} /* namespace eprosima */
