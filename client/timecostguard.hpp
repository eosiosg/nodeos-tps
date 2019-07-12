//
// Created by zwg on 19-7-11.
//

#pragma once

#include <string>
#include <ostream>
#include <fc/time.hpp>
using namespace std;

class TimeCostGuard {
    string thing;
    fc::time_point start;
    ostream & out;
    TimeCostGuard() = delete;
public:
    TimeCostGuard(ostream& out, string thing):out(out),thing(thing) {
        start = fc::time_point::now();
    }
    virtual ~TimeCostGuard() {
        auto cost = (fc::time_point::now() - start).count()/1000.0;
        if(cost > 1) { // > 1ms
            out << "End do \"" << thing << "\"," << "time cost " << cost << " ms" << endl;
        }
    }
};
