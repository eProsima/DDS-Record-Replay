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

#include <thread>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <cpp_utils/types/Singleton.hpp>
#include <cpp_utils/wait/BooleanWaitHandler.hpp>

/******************************
 * WARNING:
 * Theses tests are meant to run independently and in different processes.
 * As they use singletons it is required that singleton are constructed and destroyed within each test
 * to test it correctly.
 */

namespace test {

using TestInternalType = int;

/**
 * @brief
 *
 * @note Thread-safe class. It does not protect the access to the internal object.
 */
class TestType
{
public:

    void set(
            const TestInternalType new_value) noexcept
    {
        std::lock_guard<std::mutex> guard(mutex_);
        internal_value_ = new_value;
    }

    TestInternalType get() const noexcept
    {
        std::lock_guard<std::mutex> guard(mutex_);
        return internal_value_;
    }

protected:

    TestInternalType internal_value_ = 0;
    mutable std::mutex mutex_;

    // Protected ctor so it can only be created from Singleton
    TestType() = default;

    template <typename T, int Index>
    friend class eprosima::utils::Singleton;
};

using SingletonType = eprosima::utils::Singleton<TestType>;
using OtherSingletonType = eprosima::utils::Singleton<TestType, 42>;

} /* namespace test */

using namespace eprosima::utils;

/**
 * Access to \c SingletonType class from different scopes and threads that do
 * not have common variable, so get_instance actually gives access to same object.
 *
 * TESTS:
 * - Test that modifying the instance from a thread does change it in other thread.
 * - Test working with instance from \c get_instance , from ptr and from shared_ptr.
 * - Test that actual internal ptr is always the same.
 */
TEST(singletonTest, trivial_get_instance)
{
    // Create wait handler so a thread does not work before the other
    event::BooleanWaitHandler waiter(false, true);
    test::TestType* ptr_to_instance = nullptr;

    // Check that the value is correct at the beggining
    ASSERT_EQ(test::SingletonType::get_instance()->get(), 0);
    ptr_to_instance = test::SingletonType::get_instance();

    // Start first thread that will set value
    std::thread thread_set (
        [&waiter, ptr_to_instance]
            ()
        {
            // Get shared instance
            auto singleton_instance = test::SingletonType::get_instance();

            // Check that internal ptr is the same as the beginning
            ASSERT_EQ(singleton_instance, ptr_to_instance);

            // Check that first value is 0
            ASSERT_EQ(singleton_instance->get(), 0);

            // Set value and let get_thread run
            singleton_instance->set(42);
            waiter.open();
        }
        );

    // Start second thread that will get value
    std::thread thread_get (
        [&waiter, ptr_to_instance]
            ()
        {
            // Wait for thread_set has finished
            waiter.wait();

            // Get shared instance
            auto singleton_instance = test::SingletonType::get_shared_instance();

            // Check that internal ptr is the same as the beginning
            ASSERT_EQ(singleton_instance.get(), ptr_to_instance);

            // Check that value is 42 and modify it
            ASSERT_EQ(singleton_instance->get(), 42);
            singleton_instance->set(84);
        }
        );

    // Wait for threads to finish
    thread_set.join();
    thread_get.join();

    // Check that last modification has been done correctly
    ASSERT_EQ(test::SingletonType::get_instance()->get(), 84);
}

/**
 * This test is similar to the previous one \c trivial_get_instance
 * but in this case both threads use different Singleton indexes, and thus
 * the value should not be modified from one to the other
 *
 */
TEST(singletonTest, different_index_class)
{
    // Create wait handler so a thread does not work before the other
    event::BooleanWaitHandler waiter(false, true);
    test::TestType* ptr_to_instance = nullptr;

    // Check that the value is correct at the beggining
    ASSERT_EQ(test::SingletonType::get_instance()->get(), 0);
    ptr_to_instance = test::SingletonType::get_instance();

    // Start first thread that will set value
    std::thread thread_set (
        [&waiter, ptr_to_instance]
            ()
        {
            // Get shared instance
            auto singleton_instance = test::SingletonType::get_instance();

            // Check that internal ptr is the same as the beginning
            ASSERT_EQ(singleton_instance, ptr_to_instance);

            // Check that first value is 0
            ASSERT_EQ(singleton_instance->get(), 0);

            // Set value and let get_thread run
            singleton_instance->set(42);
            waiter.open();
        }
        );

    // Start second thread that will get value
    std::thread thread_get (
        [&waiter, ptr_to_instance]
            ()
        {
            // Wait for thread_set has finished
            waiter.wait();

            // Get shared instance
            auto singleton_instance = test::OtherSingletonType::get_shared_instance();

            // Check that internal ptr is the same as the beginning
            ASSERT_NE(singleton_instance.get(), ptr_to_instance);

            // Check that value is the initial one 0 and modify it
            ASSERT_EQ(singleton_instance->get(), 0);
            singleton_instance->set(84);
        }
        );

    // Wait for threads to finish
    thread_set.join();
    thread_get.join();

    // Check that last modification has been done correctly
    ASSERT_EQ(test::SingletonType::get_instance()->get(), 42);
    ASSERT_EQ(test::OtherSingletonType::get_instance()->get(), 84);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
