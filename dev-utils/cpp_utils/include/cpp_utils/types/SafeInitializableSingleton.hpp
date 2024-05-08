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
 * @file Singleton.hpp
 *
 * This file contains class Singleton definition.
 */

#pragma once

#include <memory>
#include <shared_mutex>

namespace eprosima {
namespace utils {

/**
 * @brief This auxiliary class allows to create a Singleton class that can be initialize in run time.
 *
 * In order to create a Singleton of a class T that could be initialized along the process,
 * implement the class T as a normal class and then use it as a Singleton by using the type SafeInitializableSingleton<T>.
 *
 * @note for more information about the Singleton class, refer to \c InitializableSingleton .
 *
 * @tparam T type of the value that will be converted to Singleton.
 * @tparam Index identifier of a specific Singleton element.
 *
 * @example
 *   class Object;  // A class that represents a generic object, but has no default constructor.
 *   using InitializedObject = SafeInitializableSingleton<Object>;
 *
 *   // From now on, we can access an instance of Database shared within the whole process (not initialized)
 *   SafeInitializableSingleton::initialize<Object ctor args>(Initialization args);
 *   InitializedObject::get_instance()->do_something_with_object(args);
 *
 * @attention this class can have an internal reference that points to \c nullptr .
 * Initialize it before using it, and check whenever used if the internal reference is valid.
 *
 * @attention this class is thread-safe, but does not guarantee access to the internal data neither.
 */
template <typename T, int Index = 0>
class SafeInitializableSingleton
{
public:

    /**
     * @brief Initialize the internal ptr of the Singleton.
     *
     * @tparam Args arguments for the \c T object ctor.
     * @param args arguments for the \c T object ctor.
     * @return true always.
     *
     * Check class \c InitializableSingleton for more information.
     */
    template <typename ... Args>
    static bool initialize(
            Args... args);

    //! Get a reference to the instance of this Singleton
    static T* get_instance(
            bool create = true) noexcept;

    /**
     * @brief Get a shared reference to the instance of this Singleton
     *
     * This method is useful to manage the order of destruction between singletons, as holding the shared_ptr
     * of one of them force it to not be destroyed until after the holder is destroyed.
     *
     * @warning Do not create a double loop between shared references in Singletons, or it will force a memory leak.
     */
    static std::shared_ptr<T> get_shared_instance(
            bool create = true) noexcept;

protected:

    /**
     * @brief Initialization function that does not take the mutex.
     *
     * @warning This function is supposed to be called with \c mtx_ guarded.
     */
    template <typename ... Args>
    static bool initialize_nts_(
            Args... args);

    /**
     * @brief The actual internal ptr of the singleton.
     *
     * @note declaring it here makes its first access non-thread-safe, and thus it must be initialize statically.
     * It is declared here because the initialize function must be parametrized, and that would forbid to use
     * the same static variable for the different get functions.
     */
    static std::shared_ptr<T> the_ptr_;

    //! Guard the acces to \c the_ptr_ that can be initialized and read anywhere.
    static std::mutex mtx_;

private:

    /**
     * @brief Protected default constructor specifies that none can create an instance of this class.
     *
     * @note this constructor must exist (cannot be deleted), otherwise this class could not be used.
     * However, this ctor will never be called anywhere.
     */
    SafeInitializableSingleton() = default;
};

} /* namespace utils */
} /* namespace eprosima */

// Include implementation template file
#include <cpp_utils/types/impl/SafeInitializableSingleton.ipp>
