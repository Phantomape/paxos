#include "bytes_buffer.h"
#include <stdio.h>
#include <assert.h>

namespace paxos
{

#define DEFAULT_BUFFER_LEN 1048576

BytesBuffer::BytesBuffer()
    : buffer_(nullptr), len_(DEFAULT_BUFFER_LEN) {
    buffer_ = new char[len_];
    assert(buffer_ != nullptr);
}
    
BytesBuffer::~BytesBuffer() {
    delete []buffer_;
}

char * BytesBuffer::GetPtr() {
    return buffer_;
}

int BytesBuffer::GetLen() {
    return len_;
}

void BytesBuffer::Ready(const int buffer_len) {
    if (len_ < buffer_len) {
        delete []buffer_;
        buffer_ = nullptr;

        while (len_ < buffer_len) {
            len_ *= 2; 
        }

        buffer_ = new char[len_];
        assert(buffer_ != nullptr);
    }
}

}


