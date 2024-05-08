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

using namespace eprosima::utils;

namespace test {

int value_to_check{0};

struct TestTypeOrder_LastIn;
struct TestTypeOrder_FirstIn;
struct TestTypeOrder_FirstIn_LastOut;
struct TestTypeOrder_LastIn_FirstOut;

/*
 * These classes are meant to be used as singleton and check the order that they are created and destroyed.
 * They fail in ctor or dtor asserting if they are not created before or after others.
 */

/**
 * This class is supposed to be created after TestTypeOrder_FirstIn_LastOut, so check the value in ctor and modify it.
 */
struct TestTypeOrder_LastIn
{
    TestTypeOrder_LastIn()
    {
        check_correct();
        value_to_check = 200;
    }

    void check_correct()
    {
        ASSERT_EQ(value_to_check, 100);
    }

};

/**
 * This class is supposed to be created before TestTypeOrder_LastIn.
 * It checks in construction that TestTypeOrder_LastIn still does not exist, and set value.
 */
struct TestTypeOrder_FirstIn
{
    TestTypeOrder_FirstIn()
    {
        check_correct_ctor();
        value_to_check = 100;
    }

    void check_correct_ctor()
    {
        ASSERT_EQ(value_to_check, 0);
    }

};


/**
 * This class is supposed to be created before TestTypeOrder_LastIn_FirstOut.
 * It sets value when it starts and check that when leaving the value has already changed.
 */
struct TestTypeOrder_FirstIn_LastOut
{
    TestTypeOrder_FirstIn_LastOut()
    {
        check_correct_ctor();
        value_to_check = 100;
    }

    ~TestTypeOrder_FirstIn_LastOut()
    {
        check_correct_dtor();
    }

    void check_correct_ctor()
    {
        ASSERT_EQ(value_to_check, 0);
    }

    void check_correct_dtor()
    {
        ASSERT_EQ(value_to_check, 200);
    }

};

/**
 * This class is supposed to be created after TestTypeOrder_LastIn or TestTypeOrder_LastIn_FirstOut.
 * Checks value in creation and set it again in destruction.
 */
struct TestTypeOrder_LastIn_FirstOut
{
    TestTypeOrder_LastIn_FirstOut()
        : lock_reference_so_it_cannot_be_destroyed_before_this_(
            Singleton<TestTypeOrder_FirstIn_LastOut>::get_shared_instance())
    {
        check_correct();
    }

    ~TestTypeOrder_LastIn_FirstOut()
    {
        value_to_check = 200;
    }

    void check_correct()
    {
        ASSERT_EQ(value_to_check, 100);
    }

    std::shared_ptr<TestTypeOrder_FirstIn_LastOut> lock_reference_so_it_cannot_be_destroyed_before_this_;
};

} /* namespace test */

TEST(singletonOrderTest, correct_construction_order)
{
    // First call get_instance of the supposed to be created first
    Singleton<test::TestTypeOrder_FirstIn>::get_instance();

    // call get_isntance of the one supposed to be constructed afterwards
    Singleton<test::TestTypeOrder_LastIn>::get_instance();

    // Let singleton destroy by themselves
}

TEST(singletonOrderTest, correct_destruction_order)
{
    // First call get_instance of the supposed to be created first
    Singleton<test::TestTypeOrder_FirstIn_LastOut>::get_instance();

    // call get_isntance of the one supposed to be constructed afterwards
    Singleton<test::TestTypeOrder_LastIn_FirstOut>::get_instance();

    // Let singleton destroy by themselves and fail if they should be destroyed in different order
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
