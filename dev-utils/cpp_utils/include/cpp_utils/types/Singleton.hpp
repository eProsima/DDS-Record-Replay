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
 * @file Singleton.hpp
 *
 * This file contains class Singleton definition.
 */

#pragma once

#include <memory>
#include <mutex>

namespace eprosima {
namespace utils {

/**
 * @brief This auxiliary class allows to easily create a Singleton class from a class that already exists.
 *
 * In order to create a Singleton of a class T, this class helps to define and implement the class T as a normal class
 * and then use it as a Singleton by using the type Singleton<T>.
 *
 * In order to create a Singleton class from T, it must have a default constructor and it is highly recommended
 * that the construction of the object is simple and could not fail.
 *
 * There could be more than one Singleton instance per class.
 * But because the static variables of this class, there could only be one Singleton per class.
 * For this purpose there is an \c Index that allows to create different Singleton instances of the same class
 * just by using a different index for each of them.
 *
 * @tparam T type of the value that will be converted to Singleton.
 * @tparam Index identifier of a specific Singleton element.
 *
 * @example
 *   class SomethingDatabase;  // A class that represents a database of things
 *   using ProcessSharedDatabase = Singleton<SomethingDatabase>;
 *
 *   // From now on, we can access an instance of Database shared within the whole process
 *   ProcessSharedDatabase::get_instance()->do_something_in_database(args);
 *
 * @attention internal class in Singleton should have a protected ctor. Otherwise the static variable could be
 * copied and, or moved. User is responsible of creating a safe class.
 *
 * @attention this class is thread-safe but does not guarantee that internal class is thread safe neither protect
 * its methods and variables.
 *
 * @attention it is advised to not use directly Singleton<T> from code, but define previously a "class" that will
 * be the singleton by \c using and choosing a "random" \c Index so every user knows the name to access it.
 */
template <typename T, int Index = 0>
class Singleton
{
public:

    //! Get a reference to the instance of this Singleton
    static T* get_instance() noexcept;

    /**
     * @brief Get a shared reference to the instance of this Singleton
     *
     * This method is useful to manage the order of destruction between singletons, as holding the shared_ptr
     * of one of them force it to not be destroyed until after the holder is destroyed.
     *
     * @warning Do not create a double loop between shared references in Singletons, or it will force a memory leak.
     */
    static std::shared_ptr<T> get_shared_instance() noexcept;

private:

    /**
     * @brief Protected default constructor specifies that none can create an instance of this class.
     *
     * @note this constructor must exist (cannot be deleted), otherwise this class could not be used.
     * However, this ctor will never be called anywhere.
     */
    Singleton() = default;
};

} /* namespace utils */
} /* namespace eprosima */

// Include implementation template file
#include <cpp_utils/types/impl/Singleton.ipp>
