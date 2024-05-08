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
 * @file InternalPtrData.ipp
 */

#pragma once

#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/exception/ValueAccessException.hpp>

namespace eprosima {
namespace utils {

///////////////////////
// CONSTRUCTORS
///////////////////////

template<typename T>
InternalPtrData<T>::InternalPtrData() noexcept
    : reference_(nullptr)
{
}

template<typename T>
InternalPtrData<T>::InternalPtrData(
        InternalPtrData&& other) noexcept
    : reference_(std::move(other.reference_))
    , shared_mutex_(std::move(other.shared_mutex_))
    , deleter_(std::move(other.deleter_))
{
}

template<typename T>
InternalPtrData<T>::~InternalPtrData() noexcept
{
    // Release data in case it still exists
    release_reference_();
}

///////////////////////
// INTERACTION METHODS
///////////////////////

template<typename T>
void InternalPtrData<T>::lock_shared() noexcept
{
    shared_mutex_.lock_shared();
}

template<typename T>
void InternalPtrData<T>::unlock_shared() noexcept
{
    shared_mutex_.unlock_shared();
}

///////////////////////
// ACCESS DATA METHODS
///////////////////////

template<typename T>
T* InternalPtrData<T>::operator ->() const noexcept
{
    return reference_;
}

template<typename T>
T& InternalPtrData<T>::operator *() const noexcept
{
    return *reference_;
}

template<typename T>
T* InternalPtrData<T>::get() const noexcept
{
    return reference_;
}

template<typename T>
InternalPtrData<T>::operator bool() const noexcept
{
    return reference_ != nullptr;
}

//////////////////////////////////
// PROTECTED METHODS FOR OWNERPTR
//////////////////////////////////

template<typename T>
InternalPtrData<T>::InternalPtrData(
        T* reference,
        const std::function<void(T*)>& deleter) noexcept
    : reference_(reference)
    , deleter_(deleter)
{
}

template<typename T>
void InternalPtrData<T>::release_reference_()
{
    shared_mutex_.lock();
    if (reference_ != nullptr)
    {
        deleter_(reference_);
        reference_ = nullptr;
    }
    shared_mutex_.unlock();
}

} /* namespace utils */
} /* namespace eprosima */
