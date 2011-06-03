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

#include <string.h>

#include "message_buffer.h"

using std::auto_ptr;

namespace sprawlnet {

MessageBuffer::MessageBuffer(const char *buffer, uint16_t length) {
    set(buffer, length);
}

void MessageBuffer::copy_to(MessageBuffer *dest) const {
    dest->set(buffer.get(), length);
}

void MessageBuffer::set(const char* buffer, uint16_t length) {
    this->buffer.reset();
    this->buffer = auto_ptr<char>(new char[length]);
    this->length = length;
    memcpy(this->buffer.get(), buffer, length);
}

} // namespace sprawlnet
