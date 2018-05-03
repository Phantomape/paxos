#pragma once

#include <vector>

namespace paxos {

class Network;

class Node {
public:
    Node() {}
    virtual ~Node() {}

    static int Run(Node*& node);
};

}