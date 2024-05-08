// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License")
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
 * @file Fuzzy.ipp
 *
 */

#pragma once

namespace eprosima {
namespace utils {

template <typename T>
Fuzzy<T>::Fuzzy(
        const T& other,
        FuzzyLevelType level /* = FuzzyLevelValues::set */)
    : value_(other)
    , fuzzy_level_(level)
{
}

template <typename T>
Fuzzy<T>::Fuzzy(
        T&& other,
        FuzzyLevelType level /* = FuzzyLevelValues::set */)
    : value_(std::move(other))
    , fuzzy_level_(level)
{
}

/////////////////////////
// OPERATORS
/////////////////////////

template <typename T>
Fuzzy<T>::operator T&() noexcept
{
    return value_;
}

template <typename T>
Fuzzy<T>::operator T() const noexcept
{
    return value_;
}

template <typename T>
bool Fuzzy<T>::operator ==(
        const Fuzzy<T>& other) const noexcept
{
    // If both unset, the object is the same
    if (!this->is_set() && !other.is_set())
    {
        return true;
    }
    else
    {
        return this->fuzzy_level_ == other.fuzzy_level_ && this->value_ == other.get_reference();
    }
}

template <typename T>
bool Fuzzy<T>::operator ==(
        const T& other) const noexcept
{
    if (!this->is_valid())
    {
        return false;
    }
    else
    {
        return this->value_ == other;
    }
}

template <typename T>
bool Fuzzy<T>::operator !=(
        const Fuzzy<T>& other) const noexcept
{
    return !(this->operator ==(other));
}

template <typename T>
bool Fuzzy<T>::operator !=(
        const T& other) const noexcept
{
    return !(this->operator ==(other));
}

template <typename T>
const T* Fuzzy<T>::operator ->() const noexcept
{
    return &value_;
}

template <typename T>
T* Fuzzy<T>::operator ->() noexcept
{
    return &value_;
}

/////////////////////////
// GET METHODS
/////////////////////////

template <typename T>
bool Fuzzy<T>::is_valid() const noexcept
{
    return fuzzy_level_ >= FuzzyLevelValues::fuzzy_level_default;
}

template <typename T>
bool Fuzzy<T>::is_set() const noexcept
{
    return fuzzy_level_ >= FuzzyLevelValues::fuzzy_level_fuzzy;
}

template <typename T>
T& Fuzzy<T>::get_reference() noexcept
{
    return value_;
}

template <typename T>
const T& Fuzzy<T>::get_reference() const noexcept
{
    return value_;
}

template <typename T>
T Fuzzy<T>::get_value() const noexcept
{
    return value_;
}

template <typename T>
FuzzyLevelType Fuzzy<T>::get_level() const noexcept
{
    return fuzzy_level_;
}

template <typename T>
std::string Fuzzy<T>::get_level_as_str() const noexcept
{
    // Ideally, we would overload the << operator for FuzzyLevelType, but we can't since it's a namespace to a short.
    std::string fuzzy_level;

    switch (fuzzy_level_)
    {
        case FuzzyLevelValues::fuzzy_level_unset:
            fuzzy_level = "UNSET";
            break;

        case FuzzyLevelValues::fuzzy_level_default:
            fuzzy_level = "DEFAULT";
            break;

        case FuzzyLevelValues::fuzzy_level_fuzzy:
            fuzzy_level = "FUZZY";
            break;

        case FuzzyLevelValues::fuzzy_level_set:
            fuzzy_level = "SET";
            break;

        case FuzzyLevelValues::fuzzy_level_hard:
            fuzzy_level = "HARD";
            break;

        default:
            // TODO: error?
            break;
    }

    return fuzzy_level;
}

/////////////////////////
// SET METHODS
/////////////////////////

template <typename T>
void Fuzzy<T>::unset()
{
    // It is not needed to change value_, as it would be used as it is not set
    fuzzy_level_ = FuzzyLevelValues::fuzzy_level_unset;
}

template <typename T>
void Fuzzy<T>::set_value(
        const T& new_value,
        FuzzyLevelType level /* = FuzzyLevelValues::SET */)
{
    value_ = new_value;
    fuzzy_level_ = level;
}

template <typename T>
void Fuzzy<T>::set_level(
        FuzzyLevelType level /* = FuzzyLevelValues::fuzzy_level_set */)
{
    fuzzy_level_ = level;
}

/////////////////////////
// SERIALIZATION
/////////////////////////

template <typename T>
std::ostream& operator <<(
        std::ostream& os,
        const Fuzzy<T>& f)
{
    os << "Fuzzy{Level(" << f.get_level_as_str() << ") " << f.get_reference() << "}";
    return os;
}

} /* namespace utils */
} /* namespace eprosima */
