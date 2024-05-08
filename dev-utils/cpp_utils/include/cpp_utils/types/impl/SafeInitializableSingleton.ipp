// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file SafeInitializableSingleton.ipp
 *
 */

#pragma once

namespace eprosima {
namespace utils {

template <typename T, int Index>
std::shared_ptr<T> SafeInitializableSingleton<T, Index>::the_ptr_;

template <typename T, int Index>
std::mutex SafeInitializableSingleton<T, Index>::mtx_;

template <typename T, int Index>
template <typename ... Args>
bool SafeInitializableSingleton<T, Index>::initialize(
        Args... args)
{
    std::lock_guard<std::mutex> _(mtx_);
    return initialize_nts_(std::forward<Args>(args) ...);
}

template <typename T, int Index>
T* SafeInitializableSingleton<T, Index>::get_instance(
        bool create /* = true */) noexcept
{
    return get_shared_instance(create).get();
}

template <typename T, int Index>
std::shared_ptr<T> SafeInitializableSingleton<T, Index>::get_shared_instance(
        bool create /* = true */) noexcept
{
    std::lock_guard<std::mutex> _(mtx_);

    if (the_ptr_ || !create)
    {
        return the_ptr_;
    }
    else
    {
        initialize_nts_();
        return the_ptr_;
    }
}

template <typename T, int Index>
template <typename ... Args>
bool SafeInitializableSingleton<T, Index>::initialize_nts_(
        Args... args)
{
    if (!the_ptr_)
    {
        the_ptr_ = std::shared_ptr<T>(new T(std::forward<Args>(args)...));
        return true;
    }
    else
    {
        return false;
    }
}

} /* namespace utils */
} /* namespace eprosima */
