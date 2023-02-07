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

/**
 * @file schema.cpp
 */

#include <ostream>
#include <map>

#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicTypeMember.h>
#include <fastrtps/types/DynamicTypePtr.h>
#include <fastrtps/types/TypeDescriptor.h>

#include <cpp_utils/exception/InconsistencyException.hpp>
#include <cpp_utils/exception/UnsupportedException.hpp>
#include <cpp_utils/types/Tree.hpp>
#include <cpp_utils/utils.hpp>

#include <auxiliar/dynamic_types/schema.hpp>

namespace eprosima {
namespace ddsrecorder {
namespace participants {
namespace detail {

constexpr const char* TYPE_SEPARATOR =
    "================================================================================\n";

struct TreeNodeType
{
    TreeNodeType(
            std::string member_name,
            std::string type_kind_name,
            bool is_struct = false)
        : member_name(member_name)
        , type_kind_name(type_kind_name)
        , is_struct(is_struct)
    {
    }

    std::string member_name;
    std::string type_kind_name;
    bool is_struct;
};

// Forward declaration
std::string type_kind_to_str(const fastrtps::types::DynamicType_ptr& type);

fastrtps::types::DynamicType_ptr container_internal_type(
        const fastrtps::types::DynamicType_ptr& dyn_type)
{
    return dyn_type->get_descriptor()->get_element_type();
}

std::vector<uint32_t> array_size(
        const fastrtps::types::DynamicType_ptr& dyn_type,
        bool unidimensional=true)
{
    if (unidimensional)
    {
        return {dyn_type->get_descriptor()->get_total_bounds()};
    }
    else
    {
        std::vector<uint32_t> result;
        for (int i=0; i<dyn_type->get_descriptor()->get_bounds_size(); i++)
        {
            result.push_back(dyn_type->get_descriptor()->get_bounds(i));
        }
        return result;
    }
}

std::vector<std::pair<std::string, fastrtps::types::DynamicType_ptr>> get_members_sorted(
        const fastrtps::types::DynamicType_ptr& dyn_type)
{
    std::vector<std::pair<std::string, fastrtps::types::DynamicType_ptr>> result;

    std::map<fastrtps::types::MemberId, fastrtps::types::DynamicTypeMember*> members;
    dyn_type->get_all_members(members);

    for (const auto& member : members)
    {
        result.emplace_back(
            std::make_pair<std::string, fastrtps::types::DynamicType_ptr>(
                member.second->get_name(),
                member.second->get_descriptor()->get_type()));
    }
    return result;
}

std::string container_kind_to_str(
        const fastrtps::types::DynamicType_ptr& dyn_type,
        bool allow_bounded = false)
{
    auto internal_type = container_internal_type(dyn_type);
    auto this_array_size = array_size(dyn_type);

    std::stringstream ss;
    ss << type_kind_to_str(internal_type);

    for (const auto& bound : this_array_size)
    {
        if (bound != fastrtps::types::BOUND_UNLIMITED)
        {
            if (allow_bounded)
            {
                ss << "[<=" << bound << "]";
            }
            else
            {
                ss << "[" << bound << "]";
            }
        }
        else
        {
            ss << "[]";
        }
    }

    return ss.str();
}

std::string type_kind_to_str(
        const fastrtps::types::DynamicType_ptr& dyn_type)
{
    switch(dyn_type->get_kind())
    {
        case fastrtps::types::TK_BOOLEAN:
            return "boolean";

        case fastrtps::types::TK_BYTE:
            return "byte";

        case fastrtps::types::TK_INT16:
            return "int16";

        case fastrtps::types::TK_INT32:
            return "int32";

        case fastrtps::types::TK_INT64:
            return "int64";

        case fastrtps::types::TK_UINT16:
            return "uint16";

        case fastrtps::types::TK_UINT32:
            return "uint32";

        case fastrtps::types::TK_UINT64:
            return "uint64";

        case fastrtps::types::TK_FLOAT32:
            return "float32";

        case fastrtps::types::TK_FLOAT64:
            return "float64";

        case fastrtps::types::TK_CHAR8:
            return "char";

        case fastrtps::types::TK_STRING8:
            return "string";

        case fastrtps::types::TK_STRING16:
            return "wstring";

        case fastrtps::types::TK_ARRAY:
            return container_kind_to_str(dyn_type);

        case fastrtps::types::TK_SEQUENCE:
            return container_kind_to_str(dyn_type, true);

        case fastrtps::types::TK_STRUCTURE:
            return dyn_type->get_name();

        case fastrtps::types::TK_FLOAT128:
        case fastrtps::types::TK_CHAR16:
        case fastrtps::types::TK_ENUM:
        case fastrtps::types::TK_BITSET:
        case fastrtps::types::TK_MAP:
        case fastrtps::types::TK_UNION:
        case fastrtps::types::TK_NONE:
            throw utils::UnsupportedException(
                STR_ENTRY << "Type " << dyn_type->get_name() << " is not supported in ROS2 msg.");

        default:
            throw utils::InconsistencyException(
                STR_ENTRY << "Type " << dyn_type->get_name() << " has not correct kind.");
    }
}

bool struct_kind(const fastrtps::types::TypeKind& kind)
{
    return kind == fastrtps::types::TK_STRUCTURE;
}

bool container_kind(const fastrtps::types::TypeKind& kind)
{
    return kind == fastrtps::types::TK_ARRAY || kind == fastrtps::types::TK_SEQUENCE ;
}

utils::TreeNode<TreeNodeType> generate_dyn_type_tree(
        const fastrtps::types::DynamicType_ptr& type,
        const std::string& member_name = "PARENT")
{
    // Get kind
    fastrtps::types::TypeKind kind = type->get_kind();

    if (container_kind(kind))
    {
        // If container (array or struct) has exactly one branch
        // Calculate child branch
        auto internal_type = container_internal_type(type);

        // Create this node
        utils::TreeNode<TreeNodeType> container(member_name, type_kind_to_str(type));
        // Add branch
        container.add_branch(generate_dyn_type_tree(internal_type, "CONTAINER_MEMBER"));

        return container;
    }
    else if (struct_kind(kind))
    {
        // If is struct, the call is recursive.
        // Create new tree node
        utils::TreeNode<TreeNodeType> parent(member_name, type->get_name(), true);

        // Get all members of this struct
        std::vector<std::pair<std::string, fastrtps::types::DynamicType_ptr>> members_by_name = get_members_sorted(type);

        for (const auto& member : members_by_name)
        {
            // Add each member with its name as a new node in a branch (recursion)
            parent.add_branch(
                generate_dyn_type_tree(member.second, member.first));
        }
        return parent;
    }
    else
    {
        // It is primitive type, thus add type name and return
        return utils::TreeNode<TreeNodeType>(member_name, type_kind_to_str(type));
    }
}

std::ostream& node_to_str(
        std::ostream& os,
        const TreeNodeType& node)
{
    os << node.type_kind_name << " " << node.member_name;
    return os;
}

std::ostream& generate_schema_from_node(
        std::ostream& os,
        const utils::TreeNode<TreeNodeType>& node)
{
    // We know for sure this is called from structs
    for (auto const& child : node.branches())
    {
        node_to_str(os, child.info);
        os <<  "\n";
    }
    return os;
}

std::string generate_dyn_type_schema_from_tree(
        const utils::TreeNode<TreeNodeType>& parent_node)
{
    std::set<std::string> types_written;

    std::stringstream ss;

    // Write down main node
    generate_schema_from_node(ss, parent_node);
    types_written.insert(parent_node.info.type_kind_name);

    // For every Node, check if it is a struct.
    // If it is, check if it is not yet written
    // If it is not, write it down
    for (const auto& node : parent_node.all_nodes())
    {
        if (node.info.is_struct && types_written.find(node.info.type_kind_name) == types_written.end())
        {
            // Add types separator
            ss << TYPE_SEPARATOR;

            // Add types name
            ss << "MSG: fastdds/" << node.info.type_kind_name << "\n";

            // Add next type
            generate_schema_from_node(ss, node);
            types_written.insert(node.info.type_kind_name);
        }
    }

    return ss.str();
}

std::string generate_dyn_type_schema(
        const fastrtps::types::DynamicType_ptr& dynamic_type)
{
    // Generate type tree
    utils::TreeNode<TreeNodeType> parent_type = generate_dyn_type_tree(dynamic_type);

    // From tree, generate string
    return generate_dyn_type_schema_from_tree(parent_type);
}

} /* namespace detail */
} /* namespace participants */
} /* namespace ddsrecorder */
} /* namespace eprosima */
