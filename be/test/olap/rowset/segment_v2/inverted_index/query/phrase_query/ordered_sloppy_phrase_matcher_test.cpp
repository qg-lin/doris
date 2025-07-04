// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "olap/rowset/segment_v2/inverted_index/query/phrase_query/ordered_sloppy_phrase_matcher.h"

#include <gtest/gtest.h>

#include "olap/rowset/segment_v2/inverted_index/util/mock_iterator.h"

using namespace testing;

namespace doris::segment_v2 {

class OrderedSloppyPhraseMatcherTest : public ::testing::Test {
protected:
    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(OrderedSloppyPhraseMatcherTest, BasicOrderedMatchWithinSlop) {
    auto mockIter1 = std::make_shared<MockIterator>();
    mockIter1->set_postings({{1, {2}}});
    auto mockIter2 = std::make_shared<MockIterator>();
    mockIter2->set_postings({{1, {4}}});

    std::vector<PostingsAndPosition> postings;
    postings.emplace_back(mockIter1, 0);
    postings.emplace_back(mockIter2, 1);

    OrderedSloppyPhraseMatcher matcher(postings, 1);
    matcher.reset(1);

    EXPECT_TRUE(matcher.next_match());
    EXPECT_FALSE(matcher.next_match());
}

TEST_F(OrderedSloppyPhraseMatcherTest, ExceedSlopThreshold) {
    auto mockIter1 = std::make_shared<MockIterator>();
    mockIter1->set_postings({{1, {2}}});
    auto mockIter2 = std::make_shared<MockIterator>();
    mockIter2->set_postings({{1, {5}}});

    std::vector<PostingsAndPosition> postings;
    postings.emplace_back(mockIter1, 0);
    postings.emplace_back(mockIter2, 1);

    OrderedSloppyPhraseMatcher matcher(postings, 1);
    matcher.reset(1);

    EXPECT_FALSE(matcher.next_match());
}

TEST_F(OrderedSloppyPhraseMatcherTest, OrderViolation) {
    auto mockIter1 = std::make_shared<MockIterator>();
    mockIter1->set_postings({{1, {3}}});
    auto mockIter2 = std::make_shared<MockIterator>();
    mockIter2->set_postings({{1, {2}}});

    std::vector<PostingsAndPosition> postings;
    postings.emplace_back(mockIter1, 0);
    postings.emplace_back(mockIter2, 1);

    OrderedSloppyPhraseMatcher matcher(postings, 2);
    matcher.reset(1);

    EXPECT_FALSE(matcher.next_match());
}

TEST_F(OrderedSloppyPhraseMatcherTest, ThreeTermsWithSlopAccumulation) {
    auto mockIter1 = std::make_shared<MockIterator>();
    mockIter1->set_postings({{1, {1}}});
    auto mockIter2 = std::make_shared<MockIterator>();
    mockIter2->set_postings({{1, {3}}});
    auto mockIter3 = std::make_shared<MockIterator>();
    mockIter3->set_postings({{1, {5}}});

    std::vector<PostingsAndPosition> postings;
    postings.emplace_back(mockIter1, 0);
    postings.emplace_back(mockIter2, 1);
    postings.emplace_back(mockIter3, 1);

    OrderedSloppyPhraseMatcher matcher(postings, 3);
    matcher.reset(1);

    EXPECT_TRUE(matcher.next_match());
}

TEST_F(OrderedSloppyPhraseMatcherTest, DocIDMismatchThrows) {
    auto mockIter = std::make_shared<MockIterator>();
    mockIter->set_postings({{1, {2}}});

    std::vector<PostingsAndPosition> postings;
    postings.emplace_back(mockIter, 0);

    OrderedSloppyPhraseMatcher matcher(postings, 0);

    EXPECT_THROW({ matcher.reset(2); }, Exception);
}

TEST_F(OrderedSloppyPhraseMatcherTest, MultiplePositionCandidates) {
    auto mockIter1 = std::make_shared<MockIterator>();
    mockIter1->set_postings({{1, {2, 5}}});
    auto mockIter2 = std::make_shared<MockIterator>();
    mockIter2->set_postings({{1, {4, 7}}});

    std::vector<PostingsAndPosition> postings;
    postings.emplace_back(mockIter1, 0);
    postings.emplace_back(mockIter2, 1);

    OrderedSloppyPhraseMatcher matcher(postings, 1);
    matcher.reset(1);

    EXPECT_TRUE(matcher.next_match());
    EXPECT_TRUE(matcher.next_match());
    EXPECT_FALSE(matcher.next_match());
}

TEST_F(OrderedSloppyPhraseMatcherTest, ZeroSlopRequiresExact) {
    auto mockIter1 = std::make_shared<MockIterator>();
    mockIter1->set_postings({{1, {2}}});
    auto mockIter2 = std::make_shared<MockIterator>();
    mockIter2->set_postings({{1, {3}}});

    std::vector<PostingsAndPosition> postings;
    postings.emplace_back(mockIter1, 0);
    postings.emplace_back(mockIter2, 1);

    OrderedSloppyPhraseMatcher matcher(postings, 0);
    matcher.reset(1);

    EXPECT_TRUE(matcher.next_match());
}

TEST_F(OrderedSloppyPhraseMatcherTest, GapWithSlopCoverage) {
    auto mockIter1 = std::make_shared<MockIterator>();
    mockIter1->set_postings({{1, {2}}});
    auto mockIter2 = std::make_shared<MockIterator>();
    mockIter2->set_postings({{1, {5}}});

    std::vector<PostingsAndPosition> postings;
    postings.emplace_back(mockIter1, 0);
    postings.emplace_back(mockIter2, 1);

    OrderedSloppyPhraseMatcher matcher(postings, 2);
    matcher.reset(1);

    EXPECT_TRUE(matcher.next_match());
}

} // namespace doris::segment_v2