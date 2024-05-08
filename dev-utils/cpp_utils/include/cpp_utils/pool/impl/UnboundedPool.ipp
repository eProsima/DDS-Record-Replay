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
 * @file UnboundedPool.ipp
 */

#ifndef __DDSROUTERUTILS_POOL_UnboundedPool_IMPL_IPP_
#define __DDSROUTERUTILS_POOL_UnboundedPool_IMPL_IPP_

#include <cpp_utils/Log.hpp>
#include <cpp_utils/utils.hpp>
#include <cpp_utils/exception/InconsistencyException.hpp>
#include <cpp_utils/exception/InitializationException.hpp>

namespace eprosima {
namespace utils {

template <typename T>
UnboundedPool<T>::UnboundedPool(
        PoolConfiguration configuration)
    : reserved_(0)
    , configuration_(configuration)
{
    // Call initialize_vector_ in every child constructor

    // Check Configuration consistency
    // Check batch size
    if (configuration.batch_size < 1)
    {
        throw utils::InitializationException("Batch size must be at least 1.");
    }
}

template <typename T>
UnboundedPool<T>::~UnboundedPool()
{
    // Check that every element has been released
    if (elements_.size() != reserved_)
    {
        logDevError(LIMITLESS_POOL, "More Elements reserved than released.");
    }

    logDebug(LIMITLESS_POOL, "Destroying Pool [" << this << "] with " << reserved_ << " elements.");

    // Delete the values
    for (auto& element : elements_)
    {
        this->delete_element_(element);
    }
}

template <typename T>
bool UnboundedPool<T>::loan(
        T*& element)
{
    logDebug(LIMITLESS_POOL, "Loaning element from Pool [" << this << "].");
    if (elements_.size() == 0)
    {
        // It requires to allocate new values
        augment_free_values_();
    }

    // There are already free values available
    // It uses an existing already allocated value
    element = elements_.back();
    elements_.pop_back();

    return true;
}

template <typename T>
bool UnboundedPool<T>::return_loan(
        T* element)
{
    logDebug(LIMITLESS_POOL, "Returning loan to Pool [" << this << "].");
    // This only could happen if more elements are released than reserved.
    // TODO: this should be a performance test, not in production. It does not affect behaviour.
    if (reserved_ == elements_.size())
    {
        throw InconsistencyException("return_loan: More elements are released than reserved.");
    }

    // Return it to the vector
    this->reset_element_(element);
    elements_.push_back(element);

    return true;
}

template <typename T>
void UnboundedPool<T>::augment_free_values_()
{
    augment_free_values_(this->configuration_.batch_size);
}

template <typename T>
void UnboundedPool<T>::augment_free_values_(
        unsigned int new_values_count)
{
    for (unsigned int i = 0; i < new_values_count; ++i)
    {
        auto new_element = this->new_element_();
        this->elements_.push_back(new_element);
    }
    reserved_ += new_values_count;
    logDebug(
        LIMITLESS_POOL,
        "Pool " << TYPE_NAME(T) << " [" << this << "] augmented in "
                << new_values_count << " to " << reserved_ << " elements.");
}

template <typename T>
void UnboundedPool<T>::initialize_vector_()
{
    augment_free_values_(this->configuration_.initial_size);
}

} /* namespace utils */
} /* namespace eprosima */

#endif /* __DDSROUTERUTILS_POOL_UnboundedPool_IMPL_IPP_ */
