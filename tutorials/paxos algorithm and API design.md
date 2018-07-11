#   Paxos Algorithm and API Design
There are three roles in the consensus algorithm, namely proposer, acceptor and learner. In implementation, a single process may act as as more than one role, which is the allegedly the current disadvantage of PhxPaxos

I'll begin to describe the actions of different roles based on Leslie's article [Paxos Made Simple](https://lamport.azurewebsites.net/pubs/paxos-simple.pdf) and I'll try to illustrate the idea by adding detailed comments in code snippet.

##  Proposer
In phase I, the proposer should choose a new proposal number and sends a request to each acceptors, which is implemented in ```Proposer::Prepare()```.
```
void Proposer::Prepare(const bool need_new_ballot) {
    // Reset relevant states
    ...
    
    // Select a proposal number and this number must follow two basic rules
    SelectProposal()

    // Sends the request/message(whatever you call) to acceptors
    SendMessage(send_paxos_msg);
}
```

In phase II, the proposer will receive response from a majority of the acceptors, which will later trigger an accept request. This is implemented in ```Proposer::OnPrepareReply```
```
void Proposer::OnPrepareReply(const PaxosMsg &recv_paxos_msg) {
    // Update proposer states
    ...

    // When received a response from acceptors, the proposer needs to determine 
    // whether it should fire the accept request, it should
    //     1. fire the accept the accept request when the response from prepare    
    //     request is accepted by a majority of acceptors. Although, the Leslie's 
    //     paper said that when received a response to the prepare request from 
    //     acceptors, it  should fire accept request, I think receiving a 
    //     reject/resolve response is much better than receiving a resolve 
    //     response/nothing. The latter approach would lead to a problem that we 
    //     can't determine what cause the no response msg_counter.
    //     2. not fire the accept request when receiving reject response or timeout
    if (resolved) {
        Accept();
    } else if (rejected) {
        Prepare();
    }
}
```
Some of the articles may describe these two phases as prepare phase and accept phase, which is kind of misleading. I prefer calling them phase I and phase II cause prepare and accept is just actions that belong to the proposer. In each phase, the proposer needs to consider a lot of things before triggering these two actions.

However, the paper didn't describe the following actions of the proposer after firing accept request. I'll talk about that in another section in this article.

##  Acceptor
In phase I, the acceptor will receive a _**prepare request**_ from the proposer, which is implemented in ```Acceptor::OnPrepareRequest()```
```
int Acceptor::OnPrepareRequest(const PaxosMsg &recv_paxos_msg) {
    // Decide whether to resolve or reject the request and revise the response 
    // accordingly
    ...

    // Send response back to proposer
    SendMessage(send_node_id, send_paxos_msg);

    return 0;
}
```

In phase II, the acceptor when received a _**accept request**_, it will normally accept it unless, it has already accepted sth. with greater ballot. This part should be implemented```Acceptro::OnAcceptRequest```
```
int Acceptor::OnAcceptRequest(const PaxosMsg & oPaxosMsg) {
    if (resolve) {
        ...
    } else {
        ...
    }
}
```


