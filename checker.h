//
// Created by LENOVO on 2023/4/19.
//

#ifndef PDSP_CHECKER_H
#define PDSP_CHECKER_H

#include "basic.h"

class checker {
private:
    std::unordered_set<int, custom_hash> solution;
    std::unordered_set<int, custom_hash> observed;

public:
    checker () = default;

    explicit checker (const std::unordered_set<int, custom_hash> &solution) : solution(solution) {}

    int can_prop (const int &u) {
        if (neighbor[u].size() == 1)
            return -1;
        int count = 0, t = -1, prev = -1;
        for (auto &v: neighbor[u]) {
            if (t != prev && prev != -1) {
                t = -1;
                break;
            }
            if (observed.find(v) != observed.end())
                count++;
            else
                prev = t, t = v;
        }
        if (count >= neighbor[u].size() - 1 && t != -1)
            return t;
        return -1;
    }

    void prop (std::stack<int> &_stack, bitmap &is_visited) {
        while (!_stack.empty()) {
            int v = _stack.top();
            _stack.pop();
            is_visited.reset(v);
            int t = can_prop(v);
            if (t != -1) {
                observed.insert(t);
                _stack.push(t);
                is_visited.set(t);
                for (auto &u: neighbor[t]) {
                    if (!is_visited.find(u) && observed.find(u) != observed.end() && u != v) {
                        _stack.push(u);
                        is_visited.set(u);
                    }
                }
            }
        }
    }

    void check () {
        std::stack<int> _stack;
        for (auto &vertex: solution) {
            bitmap is_visited(n + 1);
            observed.insert(vertex);
            _stack.push(vertex);
            is_visited.set(vertex);
            for (auto &u: neighbor[vertex]) {
                observed.insert(u);
                _stack.push(u);
                for (auto &v: neighbor[u]) {
                    if (!is_visited.find(v) && observed.find(v) != observed.end()) {
                        _stack.push(v);
                        is_visited.set(v);
                    }
                }
            }
            prop(_stack, is_visited);
        }

    }

    void is_correct () {
        if (observed.size() == n)
            std::cout << "Completely Dominating" << std::endl;
        else
            std::cout << "Not Correct" << std::endl;
    }

};


#endif //PDSP_CHECKER_H
