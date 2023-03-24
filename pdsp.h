//
// Created by LENOVO on 2023/3/11.
//

//! TODO: PROP property check maybe wrong

#ifndef PDSP_PDSP_H
#define PDSP_PDSP_H

#include "basic.h"

bool can_observed (int father) {
    if (C.find(father) != C.end())
        return true;
    if (neighbor[father].size() <= 1) {
        return true;
    } else {
        int cnt = 0;
        for (auto &v: neighbor[father]) {
            if (P.find(v) != P.end())
                cnt++;
        }
        return cnt >= neighbor[father].size() - 1;
    }
}

//! PROP property
//! remove函数用prop算法反向维护
void check_prop (int vertex) {
    bitmap is_in_stack(n);
    std::stack<std::pair<int, int>> _stack;
    for (auto &v: neighbor[vertex]) {
        for (auto &u: neighbor[v]) {
            if (is_in_stack.find(u) || P.find(u) != P.end())
                continue;
            else {
                _stack.emplace(u, v);
                is_in_stack.set(u);
            }
        }
    }

    while (!_stack.empty()) {
        std::pair<int, int> u = _stack.top();
        _stack.pop();
        is_in_stack.reset(u.first);
        if (can_observed(u.second)) {
            S[u.second].insert(u.first);
            P.insert(u.first);
            for (auto &v: neighbor[u.first]) {
                bool flag = P.find(v) != P.end();
                if (!is_in_stack.find(v) && !flag) {
                    _stack.emplace(v, u.first);
                    is_in_stack.set(v);
                }
            }
        }
    }
}

//! forget weight and recalculate score
void forget_weight () {
    for (auto &w: weight) {
        w = ceil(w * roi);
    }
    for (auto v = 0; v < n; v++) {

        /*for (auto v = 1; v <= n; v++) {*/
        int temp = 0;
        if (C.find(v) == C.end()) {
            for (auto &u: neighbor[v]) {
                if (P.find(u) == P.end())
                    temp += weight[u];
            }
            add_score[v].score = temp;
        } else {
            temp = 0;
            for (auto &u: S[v]) {
                temp += weight[u];
            }
            remove_score[v].score = temp;
        }
    }
}

//! update weight
void update_weight () {
    mean = 0;
    for (auto &w: weight) {
        w += 1;
        mean += w;
    }
    mean /= double(weight.size());
    if (mean >= gama)
        forget_weight();
}

int random_select () {
    std::uniform_int_distribution<> dis(0, int(C.size()));
    int offset = dis(gen), count = 0;
    auto v = C.begin();
    while (count < offset) {
        v++;
        count++;
    }
    if (tabu == *v)
        return *C.begin();
    else {
        tabu = *v;
        return *v;
    }
}

void check_prop_remove (int vertex) {
    auto &_prop = S[vertex];
    std::vector<int> remove_list;
    for (auto &v: _prop) {
        for (auto &u: neighbor[v]) {
            if (can_observed(u)) {
                P.insert(v);
                S[u].insert(v);
                cc[v] = false;
                remove_list.emplace_back(v);
            }
        }
    }
    for (auto &v: remove_list) {
        _prop.erase(v);
    }
    for (auto &u: neighbor[vertex]) {
        if (can_observed(u)) {
            P.insert(vertex);
            S[u].insert(vertex);
        }
    }
}

void remove (const int &v) {
    C.erase(v);
    P.erase(v);
    cc[v] = false;
    for (auto &u: S[v]) {
        P.erase(u);
        cc[u] = true;
    }
    check_prop_remove(v);
    int age = remove_score[v].age;
    remove_score.erase(v);
    int score = 0;
    for (auto &u: neighbor[v]) {
        if (P.find(u) == P.end())
            score += weight[u];
    }
    add_score[v].score = score;
    add_score[v].age = age;
}

void add (const int &v) {
    C.insert(v);
    P.insert(v);
    for (auto &u: neighbor[v]) {
        P.insert(u);
        S[v].insert(u);
        auto &t = add_score[u];
        t.score -= weight[v];
    }
    check_prop(v);

    int age = add_score[v].age + 1;
    add_score.erase(v);
    //! insert v to remove_score
    int score = 0;
    for (auto &u: S[v]) {
        score += weight[u];
    }
    auto &r = remove_score[v];
    r.score = score;
    r.age = age;
}

int select_remove_vertex () {
    std::vector<std::pair<int, vertex>> temp(remove_score.begin(), remove_score.end());
    std::sort(temp.begin(), temp.end(), [] (std::pair<int, vertex> &lhs, std::pair<int, vertex> &rhs) {
        if (lhs.second.score == rhs.second.score)
            return lhs.second.age > rhs.second.age;
        return lhs.second.score < rhs.second.score;
    });
    for (auto &i: temp)
        if (cc[i.first]) {
            tabu = i.first;
            break;
        }
    return tabu;
}

int select_add_vertex () {
    std::vector<std::pair<int, vertex>> temp(add_score.begin(), add_score.end());
    std::sort(temp.begin(), temp.end(), [] (std::pair<int, vertex> &lhs, std::pair<int, vertex> &rhs) {
        if (lhs.second.score == rhs.second.score)
            return lhs.second.age > rhs.second.age;
        return lhs.second.score > rhs.second.score;
    });
    for (auto &i: temp)
        if (cc[i.first] && i.first != tabu) {
            tabu = i.first;
            break;
        }
    return tabu;
}

//! Greedy Search
void greedy_search () {
    while (P.size() != n) {
        auto v = select_add_vertex();
        add(v);
        check_prop(v);
        update_weight();
    }
    std::cout << "Greedy: " << C.size() << std::endl;
}

void pdsp () {
    greedy_search();
    /* exit(0); */
    while (C.size() > standard_answer) {
        if (P.size() == n) {
            auto v = random_select();
            remove(v);
            continue;
        }
        auto v = select_remove_vertex();
        remove(v);
        auto u = select_add_vertex();
        add(u);
        update_weight();
    }
    std::cout << "PDSP: " << C.size() << std::endl;
}


#endif //PDSP_PDSP_H
