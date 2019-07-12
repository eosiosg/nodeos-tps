//
// Created by zwg on 19-7-11.
//

#pragma once

#include "client.hpp"

class TpsClient: public Client{
private:
    block_id_type* pId{nullptr};
public:
    using Client::Client;
    void handleMessage(const signed_block_ptr& msg) {
        if(pId !=nullptr) (*pId) = msg->previous;
    }
    void setUpdateVar(block_id_type* f) {
        pId = f;
    }
};


