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
 * @file OwnerPtr.ipp
 */

#pragma once

#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/exception/ValueAccessException.hpp>

namespace eprosima {
namespace utils {

//////////////////////////////
// STATIC AUXILIARY VARIABLES
//////////////////////////////

template<typename T>
const std::function<void(T*)> OwnerPtr<T>::DEFAULT_DELETER_ = [](T* value)
        {
            delete value;
        };

///////////////////////
// CONSTRUCTORS
///////////////////////

template<typename T>
OwnerPtr<T>::OwnerPtr(
        T* reference,
        std::function<void(T*)> deleter /* = default_deleter() */)
{
    reset(reference, deleter);
}

template<typename T>
OwnerPtr<T>::~OwnerPtr()
{
    reset();
}

template<typename T>
OwnerPtr<T>& OwnerPtr<T>::operator =(
        OwnerPtr<T>&& other) noexcept
{
    // In case this Ptr had data inside, it must be deleted
    reset();

    this->data_reference_ = std::move(other.data_reference_);
    // Move a shared ptr reset the internal ptr, so other points to null

    return *this;
}

///////////////////////
// INTERACTION METHODS
///////////////////////

template<typename T>
LesseePtr<T> OwnerPtr<T>::lease() const noexcept
{
    return LesseePtr<T>(
        data_reference_);
}

template<typename T>
void OwnerPtr<T>::reset()
{
    if (this->operator bool())
    {
        // In case the data is valid, it releases reference and erase the shared ptr
        data_reference_->release_reference_();
        data_reference_.reset();
    }
}

template<typename T>
void OwnerPtr<T>::reset(
        T* reference,
        std::function<void(T*)> deleter /* = default_deleter() */)
{
    reset();

    if (nullptr == reference)
    {
        throw InitializationException(
                  "Trying to set an OwnerPtr with a nullptr.");
    }
    else
    {
        data_reference_.reset(new InternalPtrData<T>(reference, deleter));
    }
}

///////////////////////
// ACCESS DATA METHODS
///////////////////////

template<typename T>
T* OwnerPtr<T>::operator ->() const noexcept
{
    return data_reference_->operator ->();
}

template<typename T>
T& OwnerPtr<T>::operator *() const noexcept
{
    return data_reference_->operator *();
}

template<typename T>
T* OwnerPtr<T>::get() const noexcept
{
    return data_reference_->get();
}

template<typename T>
OwnerPtr<T>::operator bool() const noexcept
{
    return
        data_reference_ &&
        data_reference_->operator bool();
}

////////////////////////////
// STATIC AUXILIARY METHODS
////////////////////////////

template<typename T>
std::function<void(T*)> OwnerPtr<T>::default_deleter()
{
    return OwnerPtr<T>::DEFAULT_DELETER_;
}

////////////////////////////
// EXTERNAL OPERATORS
////////////////////////////

template<class T>
bool operator ==(
        const OwnerPtr<T>& lhs,
        std::nullptr_t) noexcept
{
    return !lhs;
}

template<class T>
bool operator ==(
        std::nullptr_t,
        const OwnerPtr<T>& lhs) noexcept
{
    return !lhs;
}

} /* namespace utils */
} /* namespace eprosima */
