#   Raft Algorithm and API Design
This article is written to take a deep dive in etcd's raft implementation and make a cross comparison between etcd and PhxPaxos.

The primary object in raft is a node, since the concept of node also exists in paxos, we'll try to tell them apart by calling them raft node and paxos node. From the official example of etcd, we can see that a raft node is started by either reading config to create a new node or reading snapshot to reboot an existing node.