//
// Created by LENOVO on 2023/3/11.
//

#ifndef PDSP_PDSP_H
#define PDSP_PDSP_H

#include "basic.h"
#include <stack>

bool is_observed (int vertex){
    if(neighbor.find(vertex) == neighbor.end())
        return false;
    if(neighbor[vertex].size() == 1 && P.find(neighbor[vertex][0]) != P.end())
        return true;
    else {
        int cnt = 0;
        for(auto &v: neighbor[vertex]){
            if(P.find(v) != P.end())
                cnt++;
        }
        return cnt >= neighbor[vertex].size() - 1;
    }
}



//! PROP property
void check_prop (int vertex){
    bitmap is_in_stack(n);
    std::stack<int> check_stack;
    for(auto &v: neighbor[vertex]){
        for(auto & u: neighbor[v]){
            if(!is_in_stack.find(u)){
                check_stack.push(u);
                is_in_stack.set(u);
            }
        }
    }
    while(!check_stack.empty()){
        int u = check_stack.top();
        check_stack.pop();
        is_in_stack.reset(u);
        if(is_observed(u) && P.find(u) != P.end())
        if(P.find(u) != P.end()){
            for(auto &v: neighbor[u]){
                if(!is_in_stack.find(v)){
                    check_stack.push(v);
                    is_in_stack.set(v);
                }
            }
        }
    }
}



#endif //PDSP_PDSP_H
