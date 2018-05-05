#pragma once

namespace paxos {

class BytesBuffer {
public:
    BytesBuffer();
    ~BytesBuffer();

    char* GetPtr();

    int GetLen();

    void Ready(const int buffer_len);

private:
    char* buffer_;
    int len_;
};
    
}