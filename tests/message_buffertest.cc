/**
 * Copyright 2011 Kosepeisen.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. See accompanying LICENSE file.
 */
#include <stdint.h>
#include <string.h>

#include "gtest/gtest.h"

#include "message_buffer.h"

namespace sprawlnet {

const char *message = "Test message";

class MessageBufferTest : public ::testing::Test {
public:
};

TEST_F(MessageBufferTest, test_make_buffer) {
    MessageBuffer buffer(message, strlen(message) + 1);

    EXPECT_STREQ(message, buffer.get_buffer());
    EXPECT_EQ(strlen(message) + 1, buffer.get_length());
}

} // namespace sprawlnet
