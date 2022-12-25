//
// Created by andrew01 on 12/15/22.
//

#ifndef HOST_RESPONSE_H
#define HOST_RESPONSE_H
#include "Defines.h"
class Response {
public:
 Result result;
 int sd;
};

class NeighbourSuperNodeResponse : public Response{
public:
    in_addr_t ip;
    in_port_t port;
};
#endif //HOST_RESPONSE_H
