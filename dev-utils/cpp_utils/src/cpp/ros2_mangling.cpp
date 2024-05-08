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

#include <algorithm>

#include <cpp_utils/Log.hpp>
#include <cpp_utils/macros/macros.hpp>
#include <cpp_utils/utils.hpp>

#include <cpp_utils/ros2_mangling.hpp>

namespace eprosima {
namespace utils {

const char* const ros_topic_prefix = "rt";
const char* const ros_service_requester_prefix = "rq";
const char* const ros_service_response_prefix = "rr";

const std::vector<std::string> ros_prefixes_ =
{ros_topic_prefix, ros_service_requester_prefix, ros_service_response_prefix};

const char* const ros2_msgs_format = "/msg/";
const char* const ros2_srv_format = "/srv/";

std::string remove_prefix(
        const std::string& name,
        const std::string& prefix)
{
    if (name.rfind(prefix + "/", 0) == 0)
    {
        return name.substr(prefix.length());
    }
    return "";
}

std::string add_prefix(
        const std::string& name,
        const std::string& prefix)
{
    return prefix + name;
}

std::string add_suffix(
        const std::string& name,
        const std::string& suffix)
{
    return name + suffix;
}

std::string get_ros_prefix_if_exists(
        const std::string& topic_name)
{
    for (const auto& prefix : ros_prefixes_)
    {
        if (topic_name.rfind(prefix + "/", 0) == 0)
        {
            return prefix;
        }
    }
    return "";
}

std::string remove_ros_prefix_if_exists(
        const std::string& topic_name)
{
    for (const auto& prefix : ros_prefixes_)
    {
        if (topic_name.rfind(prefix + "/", 0) == 0)
        {
            return topic_name.substr(prefix.length());
        }
    }
    return topic_name;
}

std::string add_ros_topic_prefix(
        const std::string& topic_name)
{
    return add_prefix(topic_name, ros_topic_prefix);
}

std::string add_ros_service_requester_prefix(
        const std::string& topic_name)
{
    return add_prefix(topic_name, ros_service_requester_prefix);

}

std::string add_ros_service_response_prefix(
        const std::string& topic_name)
{
    return add_prefix(topic_name, ros_service_response_prefix);
}

const std::vector<std::string>& get_all_ros_prefixes()
{
    return ros_prefixes_;
}

std::string demangle_if_ros_topic(
        const std::string& topic_name)
{
    return remove_ros_prefix_if_exists(topic_name);
}

std::string mangle_if_ros_topic(
        const std::string& topic_name)
{
    if (topic_name.rfind("/", 0) == 0)
    {
        return add_ros_topic_prefix(topic_name);
    }
    return topic_name;
}

std::string demangle_if_ros_type(
        const std::string& dds_type_string)
{
    if (dds_type_string[dds_type_string.size() - 1] != '_')
    {
        // not a ROS type
        return dds_type_string;
    }

    std::string substring = "dds_::";
    size_t substring_position = dds_type_string.find(substring);
    if (substring_position == std::string::npos)
    {
        // not a ROS type
        return dds_type_string;
    }

    std::string type_namespace = dds_type_string.substr(0, substring_position);

    replace_all(type_namespace, "::", "/");

    size_t start = substring_position + substring.size();
    std::string type_name = dds_type_string.substr(start, dds_type_string.length() - 1 - start);

    return type_namespace + type_name;
}

std::string mangle_if_ros_type(
        const std::string& ros2_type_string)
{
    std::string dds_type_string = ros2_type_string;

    size_t substring_position = dds_type_string.find(ros2_msgs_format);
    if (substring_position == std::string::npos)
    {
        return dds_type_string;
    }

    std::string substring = "dds_::";

    std::string type_namespace = dds_type_string.substr(0, substring_position + strlen(ros2_msgs_format));
    std::string type_name = dds_type_string.substr(substring_position + strlen(ros2_msgs_format),
                    dds_type_string.length() - substring_position - strlen(ros2_msgs_format));

    if (type_name.length() == 0)
    {
        return dds_type_string;
    }

    replace_all(type_namespace, "/", "::");

    return type_namespace + "dds_::" + type_name + "_";
}

std::string demangle_ros_topic_prefix_from_topic(
        const std::string& topic_name)
{
    return remove_prefix(topic_name, ros_topic_prefix);
}

std::string demangle_ros_service_prefix_from_topic(
        const std::string& prefix,
        const std::string& topic_name,
        std::string suffix)
{
    std::string service_name = remove_prefix(topic_name, prefix);
    if ("" == service_name)
    {
        return "";
    }

    size_t suffix_position = service_name.rfind(suffix);
    if (suffix_position != std::string::npos)
    {
        if (service_name.length() - suffix_position - suffix.length() != 0)
        {
            logWarning(DDSPIPE_DEMANGLE,
                    "service topic has service prefix and a suffix, but not at the end"
                    ", report this: " << topic_name.c_str());
            return "";
        }
    }
    else
    {
        logWarning(DDSPIPE_DEMANGLE,
                "service topic has prefix but no suffix"
                ", report this: " << topic_name.c_str());
        return "";
    }
    return service_name.substr(0, suffix_position);
}

std::string mangle_ros_service_in_topic(
        const std::string& prefix,
        const std::string& topic_name,
        const std::string suffix)
{
    size_t topic_name_position = topic_name.rfind("/");
    if (topic_name_position != 0)
    {
        return "";
    }

    std::string service_name = add_prefix(topic_name, prefix);
    service_name = add_suffix(service_name, suffix);

    return service_name;
}

std::string demangle_ros_service_prefix_from_topic(
        const std::string& topic_name)
{
    const std::string demangled_topic = demangle_ros_service_reply_prefix_from_topic(topic_name);
    if ("" != demangled_topic)
    {
        return demangled_topic;
    }
    return demangle_ros_service_request_prefix_from_topic(topic_name);
}

std::string demangle_ros_service_request_prefix_from_topic(
        const std::string& topic_name)
{
    return demangle_ros_service_prefix_from_topic(ros_service_requester_prefix, topic_name, "Request");
}

std::string mangle_ros_service_request_prefix_in_topic(
        const std::string& topic_name)
{
    return mangle_ros_service_in_topic(ros_service_requester_prefix, topic_name, "Request");
}

std::string demangle_ros_service_reply_prefix_from_topic(
        const std::string& topic_name)
{
    return demangle_ros_service_prefix_from_topic(ros_service_response_prefix, topic_name, "Reply");
}

std::string mangle_ros_service_reply_prefix_in_topic(
        const std::string& topic_name)
{
    return mangle_ros_service_in_topic(ros_service_response_prefix, topic_name, "Reply");
}

std::string demangle_service_type_only(
        const std::string& dds_type_name)
{
    std::string ns_substring = "dds_::";
    size_t ns_substring_position = dds_type_name.find(ns_substring);
    if (std::string::npos == ns_substring_position)
    {
        // not a ROS service type
        return "";
    }
    auto suffixes = {
        std::string("_Response_"),
        std::string("_Request_"),
    };
    std::string found_suffix = "";
    size_t suffix_position = 0;
    for (auto suffix : suffixes)
    {
        suffix_position = dds_type_name.rfind(suffix);
        if (suffix_position != std::string::npos)
        {
            if (dds_type_name.length() - suffix_position - suffix.length() != 0)
            {
                logWarning(DDSPIPE_DEMANGLE,
                        "service type contains 'dds_::' and a suffix, but not at the end"
                        ", report this: " << dds_type_name.c_str());
                continue;
            }
            found_suffix = suffix;
            break;
        }
    }
    if (std::string::npos == suffix_position)
    {
        logWarning(DDSPIPE_DEMANGLE,
                "service type contains 'dds_::' but does not have a suffix"
                ", report this: " << dds_type_name.c_str());
        return "";
    }
    // everything checks out, reformat it from '[type_namespace::]dds_::<type><suffix>'
    // to '[type_namespace/]<type>'
    std::string type_namespace = dds_type_name.substr(0, ns_substring_position);
    replace_all(type_namespace, "::", "/");
    size_t start = ns_substring_position + ns_substring.length();
    std::string type_name = dds_type_name.substr(start, suffix_position - start);
    return type_namespace + type_name;
}

std::string mangle_service_type_only(
        const std::string& ros2_type_name)
{
    size_t ns_substring_srv_position = ros2_type_name.find(ros2_srv_format);
    if (std::string::npos == ns_substring_srv_position)
    {
        return "";
    }

    std::string dds_type_name =
            ros2_type_name.substr(0, ns_substring_srv_position + strlen(ros2_srv_format)) + "dds_::";
    dds_type_name = dds_type_name + ros2_type_name.substr(ns_substring_srv_position + strlen(
                        ros2_srv_format),
                    ros2_type_name.length() - ns_substring_srv_position - strlen(ros2_srv_format));

    size_t ns_substring_rq_position = ros2_type_name.find("rq/");
    size_t ns_substring_rr_position = ros2_type_name.find("rr/");

    if (0 == ns_substring_rq_position)
    {
        dds_type_name = dds_type_name + "_Request_";
    }
    else if (0 == ns_substring_rr_position)
    {
        dds_type_name = dds_type_name + "_Response_";
    }
    else
    {
        return "";
    }

    replace_all(dds_type_name, "/", "::");

    return dds_type_name;
}

} /* namespace utils */
} /* namespace eprosima */
