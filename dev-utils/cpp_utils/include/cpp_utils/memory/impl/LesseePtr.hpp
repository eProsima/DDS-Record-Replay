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
 * @file LesseePtr.hpp
 *
 * This file contains class LesseePtr definition.
 */

#pragma once

#include <memory>
#include <mutex>
#include <shared_mutex>

#include <cpp_utils/library/library_dll.h>
#include <cpp_utils/memory/impl/GuardedPtr.hpp>
#include <cpp_utils/memory/impl/InternalPtrData.hpp>

namespace eprosima {
namespace utils {

//! Forward declaration of OwnerPtr to use it as friendly class in LesseePtr
template <class T>
class OwnerPtr;

/**
 * This class contains a reference (ptr) to a value of type T but does not own it.
 * Not owning the object implies that, when being used, this reference could have been invalidated (destroyed).
 * In order to avoid that, method \c lock creates a smart pointer referencing that object, that will not be destroyed
 * while the pointer exists.
 *
 * @tparam T Type of the data referenced by this ptr.
 *
 * @note It could only be created from a \c OwnerPtr object, the owner of the data referenced.
 */
template <class T>
class LesseePtr final
{
public:

    ///////////////////////
    // CONSTRUCTORS
    ///////////////////////

    /**
     * @brief Construct a new empty Lessee Ptr object
     *
     * This constructor is only used to create an empty object.
     * It could not contain any data.
     */
    LesseePtr() noexcept = default;

    /**
     * @brief Destroy the Lessee Ptr object
     *
     * It does not destroy the data (it has not the ownership).
     * Mutex of Internal reference is handled by GuardedPtrs.
     */
    ~LesseePtr() = default;

    //! Copy constructor from \c OwnerPtr
    LesseePtr(
            const OwnerPtr<T>& owner) noexcept;

    //! Copy constructor
    LesseePtr(
            const LesseePtr<T>& other) noexcept = default;

    //! Move constructor
    LesseePtr(
            LesseePtr<T>&& other) noexcept = default;

    /**
     * @brief copy assigment
     *
     * It copies the reference from the other object.
     *
     * @param other object to copy
     * @return this object
     */
    LesseePtr<T>& operator =(
            const LesseePtr<T>& other) noexcept = default;

    /**
     * @brief move assigment
     *
     * It gets the reference from the other object.
     * \c other object loses its internal data and gets invalidated.
     *
     * @param other object to copy
     * @return this object
     */
    LesseePtr<T>& operator =(
            LesseePtr<T>&& other) noexcept = default;

    ///////////////////////
    // ACCESS DATA METHODS
    ///////////////////////

    /**
     * @brief Access operator to internal ptr with exception calls in failure cases.
     *
     * This operator will acces the internal data if this still exist, and lock it while using.
     * If the data do not further exist, it will raise an exception.
     *
     * @return Internal data ptr
     *
     * @throw ValueAccessException in case internal data no longer exists.
     */
    T* operator ->() const;

    /**
     * @brief Access operator to internal ptr with exception calls in failure cases.
     *
     * This operator will acces the internal data if this still exist, and lock it while using.
     * If the data do not further exist, it will raise an exception.
     *
     * @return Internal data reference
     *
     * @throw ValueAccessException in case internal data no longer exists.
     */
    T& operator *() const;

    /**
     * @brief Create a smart reference to the data.
     *
     * While the smart reference (return of this method) exists, the data cannot be destroyed.
     * This smart reference must be destroyed as soon as possible, because it locks the real owner of the data.
     *
     * @note This method is similar to \c std::weak_ptr::lock() .
     *
     * @warning return of this method must be checked before used, the data returned could not be valid anymore.
     * @warning this method does not protect the access to the internal data. It only avoids its destruction.
     *
     * @return std::shared_ptr<T> to the data. nullptr if the reference is not valid anymore.
     */
    GuardedPtr<T> lock() const noexcept;

    /**
     * @brief Create a smart reference to the data or throw an exception if data is not available
     *
     * While the smart reference (return of this method) exists, the data could not be destroyed.
     * This smart reference must be destroyed as soon as possible, because it locks the real owner of the data.
     *
     * Use this method instead of \c lock if the non existence of the data is treated as error.
     *
     * @warning this method does not protect the access to the internal data. It only avoids its destruction.
     *
     * @return std::shared_ptr<T> to the data
     * @throw \c ValueAccessException if the data is not valid anymore.
     */
    GuardedPtr<T> lock_with_exception() const;

    /**
     * @brief Whether the internal data is still valid
     *
     * @warning this method does not protect the access to the internal data.
     * It could happen that data is destroyed right after this method returns.
     *
     * A \c true from this method is volatile, while a \c false is persistent.
     */
    operator bool() const noexcept;

protected:

    ////////////////////////////
    // PROTECTED CONSTRUCTOR
    ////////////////////////////

    //! \c OwnerPtr requires friendship to use the constructor
    friend class OwnerPtr<T>;

    /**
     * @brief Construct a new Lessee Ptr object
     *
     * Protected constructor that must be called from \c OwnerPtr .
     *
     * @param data weak reference to the data.
     * @param shared_mutex shared mutex between owner and this.
     */
    LesseePtr(
            std::shared_ptr<InternalPtrData<T>> data_reference) noexcept;


    ////////////////////////////
    // INTERNAL VARIABLES
    ////////////////////////////

    //! Internal data reference
    std::shared_ptr<InternalPtrData<T>> data_reference_;
};

} /* namespace utils */
} /* namespace eprosima */

// Include implementation template file
#include <cpp_utils/memory/impl/LesseePtr.ipp>
