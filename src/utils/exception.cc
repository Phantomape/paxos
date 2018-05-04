#include "exception.h"

namespace paxos {

SysCallException::SysCallException(int err_code, const std::string& err_msg, bool detail) 
    : err_code_(err_code), err_msg_(err_msg) {
    if (detail) {
        err_msg_.append(", ").append(::strerror(err_code_));
    }
}

int SysCallException::GetErrorCode() const throw () {
    return err_code_;
}

const char* SysCallException::What() const throw () {
    return err_msg_.c_str();
}

}