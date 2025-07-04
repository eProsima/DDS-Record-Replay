// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <memory>

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <ddsrecorder_participants/recorder/handler/HandlerContextCollection.hpp>
#include <ddsrecorder_participants/recorder/handler/mcap/McapHandler.hpp>
#include <ddsrecorder_participants/recorder/handler/sql/SqlHandler.hpp>

using namespace eprosima;
using namespace eprosima::fastdds::dds;
using namespace eprosima::ddsrecorder::participants;

struct MockHandlerContext
    : public ddsrecorder::participants::HandlerContext
{
public:

    MockHandlerContext(
            HandlerKind kind,
            std::shared_ptr<BaseHandler> handler)
        : HandlerContext(kind, handler, nullptr, nullptr)
    {
        // Mock implementation
    }

};

class HandlerContextCollectionTest : public testing::Test
{
public:

    void SetUp() override
    {
        mcap_handler_ = std::make_shared<McapHandler>();
        sql_handler_ = std::make_shared<SqlHandler>();

        handler_contexts_ = std::make_unique<
            ddsrecorder::participants::HandlerContextCollection>();
    }

    void TearDown() override
    {
        handler_contexts_.reset(nullptr);
    }

protected:

    std::unique_ptr<ddsrecorder::participants::HandlerContextCollection> handler_contexts_{nullptr};
    std::shared_ptr<BaseHandler> mcap_handler_{nullptr};
    std::shared_ptr<BaseHandler> sql_handler_{nullptr};
};

/**
 * Test that the collection correctly behaves on initialization.
 * CASES:
 * - Initializes a MCAP handler and a SQL handler contexts before starting.
 */
TEST_F(HandlerContextCollectionTest, initialization_ok)
{
    utils::ReturnCode ret = handler_contexts_->init_handler_context(
        std::make_shared<MockHandlerContext>(MockHandlerContext::HandlerKind::MCAP, mcap_handler_));
    EXPECT_EQ(ret, utils::ReturnCode::RETCODE_OK);
    ret = handler_contexts_->init_handler_context(
        std::make_shared<MockHandlerContext>(MockHandlerContext::HandlerKind::SQL, sql_handler_));
    EXPECT_EQ(ret, utils::ReturnCode::RETCODE_OK);

    handler_contexts_->start_nts();
    handler_contexts_->pause_nts();
    handler_contexts_->stop_nts();
}

/**
 * Test that the collection correctly behaves on initialization.
 * CASES:
 * - Initialize a MCAP handler context twice.
 * - Initialize a SQL handler context after calling the starting operation.
 */
TEST_F(HandlerContextCollectionTest, bad_initialization)
{
    handler_contexts_->init_handler_context(
        std::make_shared<MockHandlerContext>(MockHandlerContext::HandlerKind::MCAP, mcap_handler_));
    utils::ReturnCode ret = handler_contexts_->init_handler_context(
        std::make_shared<MockHandlerContext>(MockHandlerContext::HandlerKind::MCAP, mcap_handler_));
    EXPECT_EQ(ret, utils::ReturnCode::RETCODE_ERROR);

    handler_contexts_->start_nts();

    ret = handler_contexts_->init_handler_context(
        std::make_shared<MockHandlerContext>(MockHandlerContext::HandlerKind::SQL, sql_handler_));
    EXPECT_EQ(ret, utils::ReturnCode::RETCODE_PRECONDITION_NOT_MET);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
