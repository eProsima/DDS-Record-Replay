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

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <cpp_utils/memory/Heritable.hpp>

using namespace eprosima::utils;

namespace test {

struct Parent
{
    Parent(
            int x)
        : int_value(x)
    {
    }

    Parent()
        : Parent(0)
    {
    }

    Parent(
            const Parent& other)
        : Parent(other.int_value)
    {
    }

    Parent& operator =(
            const Parent& other)
    {
        int_value = other.int_value;
        return *this;
    }

    bool operator ==(
            const Parent& other) const
    {
        return other.get_int() == this->get_int();
    }

    bool operator <(
            const Parent& other) const
    {
        return this->get_int() < other.get_int();
    }

    virtual int get_int() const
    {
        return int_value;
    }

    bool greater_than_10() const
    {
        return get_int() > 10;
    }

    int int_value;
};

std::ostream& operator <<(
        std::ostream& os,
        const Parent& p)
{
    os << "P{" << p.get_int() << "}";
    return os;
}

struct Child_A : public Parent
{
    Child_A(
            int y)
        : Parent(10)
        , other_value(y)
    {
    }

    Child_A()
        : Child_A(1)
    {
    }

    virtual int get_int() const override
    {
        return Parent::get_int() + other_value;
    }

    int other_value;
};

struct Child_B : public Parent
{
    Child_B(
            int y)
        : Parent(10)
        , other_value(y)
    {
    }

    Child_B()
        : Child_B(1)
    {
    }

    virtual int get_int() const override
    {
        return Parent::get_int() * other_value;
    }

    int other_value;
};

} /* namespace test */

/**
 * TODO
 */
TEST(HeritableTest, constructor_int)
{
    // From ptr
    {
        Heritable<int> h(new int(1));
        ASSERT_EQ(h.get_reference(), 1);
    }

    // From make_heriable
    {
        Heritable<int> h = Heritable<int>::make_heritable(2);
        ASSERT_EQ(h.get_reference(), 2);
    }
}

/**
 * TODO
 */
TEST(HeritableTest, constructor_parent)
{
    // from ptr
    {
        Heritable<test::Parent> h(new test::Parent(1));
        ASSERT_EQ(h->get_int(), 1);
    }

    // from child ptr
    {
        Heritable<test::Parent> h(new test::Child_A(1));
        ASSERT_EQ(h->get_int(), 11);
    }

    // from make_heritable default
    {
        Heritable<test::Parent> h(Heritable<test::Parent>::make_heritable());
        ASSERT_EQ(h->get_int(), 0);
    }

    // from make_heritable with arg
    {
        Heritable<test::Parent> h(Heritable<test::Parent>::make_heritable(3));
        ASSERT_EQ(h->get_int(), 3);
    }

    // from make_heritable copy
    {
        test::Parent p(4);
        Heritable<test::Parent> h(Heritable<test::Parent>::make_heritable(p));
        ASSERT_EQ(h->get_int(), 4);
    }

    // by copy
    {
        Heritable<test::Parent> p(new test::Parent(5));
        Heritable<test::Parent> h(p);
        ASSERT_EQ(h->get_int(), 5);
        ASSERT_EQ(h, p);
    }

    // by movement
    {
        Heritable<test::Parent> p(new test::Parent(6));
        Heritable<test::Parent> h(std::move(p));
        ASSERT_EQ(h->get_int(), 6);
    }

    // by copy from child
    {
        Heritable<test::Child_A> p(new test::Child_A(7));
        Heritable<test::Parent> h(p);
        ASSERT_EQ(h->get_int(), 17);
        ASSERT_EQ(h, p);
    }

    // by movement from child
    {
        Heritable<test::Child_A> p(new test::Child_A(8));
        Heritable<test::Parent> h(std::move(p));
        ASSERT_EQ(h->get_int(), 18);
    }

    // copy operator
    {
        Heritable<test::Parent> p(new test::Parent(9));
        Heritable<test::Parent> h(new test::Parent(10));
        h = p;
        ASSERT_EQ(h->get_int(), 9);
        ASSERT_EQ(h, p);
    }

    // move operator
    {
        Heritable<test::Parent> p(new test::Parent(11));
        Heritable<test::Parent> h(new test::Parent(12));
        h = std::move(p);
        ASSERT_EQ(h->get_int(), 11);
    }
}

/**
 * TODO
 */
TEST(HeritableTest, ptr_operator_parent)
{
    // ->
    {
        Heritable<test::Parent> h(Heritable<test::Parent>::make_heritable(4));
        ASSERT_EQ(h->get_int(), 4);
        ASSERT_FALSE(h->greater_than_10());
    }

    // *
    {
        Heritable<test::Parent> h(Heritable<test::Parent>::make_heritable(11));
        ASSERT_EQ((*h).get_int(), 11);
        ASSERT_TRUE((*h).greater_than_10());
    }
}

/**
 * TODO
 */
TEST(HeritableTest, compare_operator_parent)
{
    // == heritable true
    {
        Heritable<test::Parent> h1(new test::Parent(1));
        Heritable<test::Parent> h2(new test::Parent(1));
        ASSERT_TRUE(h1.operator ==(h2));
    }

    // == heritable false
    {
        Heritable<test::Parent> h1(new test::Parent(1));
        Heritable<test::Parent> h2(new test::Parent(2));
        ASSERT_FALSE(h1.operator ==(h2));
    }

    // == object true
    {
        Heritable<test::Parent> h1(new test::Parent(1));
        test::Parent p(1);
        ASSERT_TRUE(h1.operator ==(p));
    }

    // == object false
    {
        Heritable<test::Parent> h1(new test::Parent(1));
        test::Parent p(2);
        ASSERT_FALSE(h1.operator ==(p));
    }

    // < heritable true
    {
        Heritable<test::Parent> h1(new test::Parent(0));
        Heritable<test::Parent> h2(new test::Parent(1));
        ASSERT_TRUE(h1.operator <(h2));
    }

    // < heritable false
    {
        Heritable<test::Parent> h1(new test::Parent(0));
        Heritable<test::Parent> h2(new test::Parent(-1));
        ASSERT_FALSE(h1.operator <(h2));
    }

    // < object true
    {
        Heritable<test::Parent> h1(new test::Parent(1));
        test::Parent p(2);
        ASSERT_TRUE(h1.operator <(p));
    }

    // < object false
    {
        Heritable<test::Parent> h1(new test::Parent(1));
        test::Parent p(0);
        ASSERT_FALSE(h1.operator <(p));
    }
}

/**
 * TODO
 */
TEST(HeritableTest, access_data_methods_parent)
{
    // get_reference
    {
        test::Parent p = test::Parent(11);
        Heritable<test::Parent> h(Heritable<test::Parent>::make_heritable(p));
        test::Parent& p_ref = h.get_reference();
        ASSERT_TRUE(p_ref.operator ==(p));
        ASSERT_NE(&p_ref, &p);
    }
}

/**
 * TODO
 */
TEST(HeritableTest, cast_methods)
{
    // Cast to parent
    {
        Heritable<test::Child_A> h(Heritable<test::Child_A>::make_heritable());
        ASSERT_TRUE(h->greater_than_10());

        ASSERT_TRUE(h.can_cast<test::Parent>());
        test::Parent& p = h.dyn_cast<test::Parent>();
        ASSERT_TRUE(p.greater_than_10());

        // Create a child from child conserves value
        test::Child_A a = dynamic_cast<test::Child_A&>(p);
        ASSERT_TRUE(a.greater_than_10());

        // Create a parent from child copy does not conserve value
        test::Parent p_ = p;
        ASSERT_FALSE(p_.greater_than_10());
    }

    // Cast from parent with child to child
    {
        Heritable<test::Parent> h(Heritable<test::Child_A>::make_heritable());
        ASSERT_TRUE(h->greater_than_10());

        ASSERT_TRUE(h.can_cast<test::Child_A>());
        test::Child_A& p = h.dyn_cast<test::Child_A>();
        ASSERT_TRUE(p.greater_than_10());

        ASSERT_FALSE(h.can_cast<test::Child_B>());
    }
}

/**
 * TODO
 */
TEST(HeritableTest, cast_methods_negative)
{
    // cast from parent to child
    {
        Heritable<test::Parent> h(Heritable<test::Parent>::make_heritable());
        ASSERT_FALSE(h.can_cast<test::Child_A>());
        ASSERT_THROW(
            h.dyn_cast<test::Child_A>(),
            std::bad_cast);
    }

    // cast from child A to child B
    {
        Heritable<test::Child_A> h(Heritable<test::Child_A>::make_heritable());
        ASSERT_FALSE(h.can_cast<test::Child_B>());
        ASSERT_THROW(
            h.dyn_cast<test::Child_B>(),
            std::bad_cast);
    }
}

/**
 * TODO
 */
TEST(HeritableTest, serialize_operator)
{
    // int
    {
        Heritable<int> h(Heritable<int>::make_heritable(-3));
        std::stringstream ss;
        ss << h;
        ASSERT_EQ(ss.str(), "{-3}");
    }

    // std::string
    {
        Heritable<std::string> h(Heritable<std::string>::make_heritable(5, '='));
        std::stringstream ss;
        ss << h;
        ASSERT_EQ(ss.str(), "{=====}");
    }

    // parent
    {
        Heritable<test::Parent> h(Heritable<test::Parent>::make_heritable(17));
        std::stringstream ss;
        ss << h;
        ASSERT_EQ(ss.str(), "{P{17}}");
    }
}

/**
 * TODO
 */
TEST(HeritableTest, inheritance_set_test)
{
    std::set<Heritable<test::Parent>> p_set;

    // Add a Parent value
    Heritable<test::Parent> p1(Heritable<test::Parent>::make_heritable(1));
    ASSERT_EQ(p_set.find(p1), p_set.end());
    p_set.insert(p1);
    ASSERT_NE(p_set.find(p1), p_set.end());

    // Add another Parent value created in call
    ASSERT_EQ(p_set.find(Heritable<test::Parent>::make_heritable(2)), p_set.end());
    p_set.insert(Heritable<test::Parent>::make_heritable(2));
    ASSERT_NE(p_set.find(Heritable<test::Parent>::make_heritable(2)), p_set.end());

    // Add another Parent value created by ptr
    ASSERT_EQ(p_set.find(Heritable<test::Parent>::make_heritable(3)), p_set.end());
    p_set.insert(new test::Parent(3));
    ASSERT_NE(p_set.find(Heritable<test::Parent>::make_heritable(3)), p_set.end());

    // Add a child A value
    Heritable<test::Child_A> a3(Heritable<test::Child_A>::make_heritable(3));
    ASSERT_EQ(p_set.find(a3), p_set.end());
    p_set.insert(a3);
    ASSERT_NE(p_set.find(a3), p_set.end());

    // Add a child A value created in parent ptr
    Heritable<test::Parent> a4(Heritable<test::Child_A>::make_heritable(4));
    ASSERT_EQ(p_set.find(a4), p_set.end());
    p_set.insert(a4);
    ASSERT_NE(p_set.find(a4), p_set.end());

    // Add another child A value created in call
    ASSERT_EQ(p_set.find(Heritable<test::Child_A>::make_heritable(5)), p_set.end());
    p_set.insert(Heritable<test::Child_A>::make_heritable(5));
    ASSERT_NE(p_set.find(Heritable<test::Child_A>::make_heritable(5)), p_set.end());

    // Add another child A value created from ptr
    Heritable<test::Child_A> a6(Heritable<test::Child_A>::make_heritable(6));
    ASSERT_EQ(p_set.find(a6), p_set.end());
    p_set.insert(new test::Child_A(6));
    ASSERT_NE(p_set.find(a6), p_set.end());

    // Add  child B
    Heritable<test::Child_B> b7(Heritable<test::Child_B>::make_heritable(7));
    ASSERT_EQ(p_set.find(b7), p_set.end());
    p_set.insert(b7);
    ASSERT_NE(p_set.find(b7), p_set.end());

    // Find a child A class calling from a Parent
    auto it_1 = p_set.find(Heritable<test::Parent>::make_heritable(13));
    ASSERT_NE(it_1, p_set.end());
    ASSERT_TRUE(it_1->can_cast<test::Child_A>());

    // Find a parent class calling from a Child
    auto it_2 = p_set.find(Heritable<test::Child_A>::make_heritable(-9));
    ASSERT_NE(it_2, p_set.end());
    ASSERT_FALSE(it_2->can_cast<test::Child_A>());
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
