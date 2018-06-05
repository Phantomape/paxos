#pragma once

#include <cstring>
#include <exception>
#include <string>

namespace paxos {

class SysCallException : public std::exception {
public:
    SysCallException(int err_code, const std::string& err_msg, bool detail = true);

    virtual ~SysCallException() throw () {}

    int GetErrorCode() const throw ();

    const char* What() const throw ();

protected:
    int err_code_;
    std::string err_msg_;
};

}
