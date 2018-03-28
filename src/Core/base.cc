#include "base.h"
#include <iostream>
#include <string>

namespace paxos {
    
Base::Base() {
    std::cout << "Base::Base()" << std::endl;
}

Base::~Base() {
    std::cout << "Base::~Base()" << std::endl;
}

}

