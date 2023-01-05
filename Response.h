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
class NextSuperNodeResponse : public Response{
public:
    in_addr_t Nextip;
    in_addr_t NextRedundantIp;
    in_port_t Nextport;
    bool isAlone;
    double foundRatio;
    bool available;
};
class AcceptSuperNodeResponse : public Response{
public:
   bool shouldBeRedundantSuperNode;
};
#endif //HOST_RESPONSE_H
