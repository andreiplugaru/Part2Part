//
// Created by andrew01 on 12/15/22.
//

#ifndef HOST_REQUEST_H
#define HOST_REQUEST_H

class Request {

};
class NeighbourSuperNodeRequest: public Request{
public:
    bool type;//0 - prev; 1 - next
    in_addr_t ip;
    in_port_t port;
};
class ConnectToSuperNodeRequest: public Request{
};
class ChangeSuperNodeNeighbourRequest: public Request{
};
class FileRequest: public Request{
public:
    int id;
    in_addr_t ipOfTheNodeRequesting;
    in_addr_t ipOfTheSuperNodeRequesting;
    in_addr_t ipOfTheNodeWithFile;
    char fileName[255];
    };
#endif //HOST_REQUEST_H
