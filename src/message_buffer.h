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
#ifndef MESSAGE_BUFFER_H
#define MESSAGE_BUFFER_H

#include <stdint.h>
#include <memory>

namespace sprawlnet {

class MessageBuffer {
public:
    MessageBuffer() : buffer(NULL), length(0) {}
    MessageBuffer(const char* buffer, uint16_t length);
    virtual ~MessageBuffer() {};
    void copy_to(MessageBuffer *dest);
    const char* get_buffer() { return buffer.get(); }
    uint16_t get_length() { return length; }

protected:
    virtual void set(const char* buffer, uint16_t length);

private:
    MessageBuffer(const MessageBuffer&);
    MessageBuffer &operator=(const MessageBuffer&);

    std::auto_ptr<char> buffer;
    uint16_t length;
};

} // namespace sprawlnet

#endif

