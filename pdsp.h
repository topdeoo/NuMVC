//
// Created by LENOVO on 2023/3/11.
//

#ifndef PDSP_PDSP_H
#define PDSP_PDSP_H

#include "basic.h"


bool can_prop(const int &u) {
    if (C.find(u) != C.end())
        return true;
    int cnt = 0;
    for (auto &v: neighbor[u]) {
        if (C.find(v) != C.end())
            return true;
        if (P.find(v) != P.end())
            cnt++;
    }
    if (cnt == neighbor[u].size() - 1)
        return true;
    return false;
}

int get_prop_vertex(const int u) {
    if (neighbor[u].size() == 1)
        return -1;
    int count = 0, t = -1, prev = -1;
    for (auto &v: neighbor[u]) {
        if (t != prev && prev != -1) {
            t = -1;
            break;
        }
        if (P.find(v) != P.end())
            count++;
        else
            prev = t, t = v;
    }
    if (count >= neighbor[u].size() - 1 && t != -1)
        return t;
    return -1;
}

// 对新加入的 vertex 进行 prop 性质的检查
// 相当于做一次 dfs
// 1. 将 N[N[vertex]] 加入栈中, 然后对栈中的元素进行 prop 检查
// 2. 如果栈中的元素 u 可以 prop, 则将此新 prop 的顶点 v 其加入 P 中, 并将 v 与 N[v] / {u} 加入栈中
// 3. 将 v 加入到 S[u] 中, 表明 v 是通过 u prop 的
void check_prop(const int &vertex) {
    std::stack<int> _stack;
    bitmap is_in_stack(n);

    for (auto &v: neighbor[vertex]) {
        _stack.emplace(v);
        is_in_stack.set(v);
        for (auto &u: neighbor[v]) {
            if (P.find(u) != P.end() && u != vertex && !is_in_stack.find(u)) {
                _stack.emplace(u);
                is_in_stack.set(u);
            }
        }
    }

    while (!_stack.empty()) {
        int u = _stack.top();
        _stack.pop();
        is_in_stack.reset(u);

        int t = get_prop_vertex(u);
        if (t != -1) {
            P.insert(t);
            S[u].insert(t);
            S[vertex].insert(t);
            if (!is_in_stack.find(t)) {
                _stack.emplace(t);
                is_in_stack.set(t);
            }

            for (auto &v: neighbor[t]) {
                add_score[v].score -= weight[t];
                if (!is_in_stack.find(v) && P.find(v) != P.end() && v != u) {
                    _stack.emplace(v);
                    is_in_stack.set(v);
                }
            }
        }
    }

}

//! forget weight and recalculate score
//! TODO: change when change test case
void forget_weight() {
    for (auto &w: weight) {
        w = ceil(w * roi);
    }
/*    for (auto v = 0; v < n; v++) {*/

    for (auto v = 1; v <= n; v++) {
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
void update_weight() {
    mean = 0;
    for (auto &w: weight) {
        w += 1;
        mean += w;
    }
    for (auto &v: add_score) {
        for (auto &u: neighbor[v.first])
            if (P.find(u) == P.end())
                v.second.score += weight[u];
    }

    for (auto &v: remove_score)
        v.second.score -= int(S[v.first].size());

    mean /= double(weight.size());
    if (mean >= gama)
        forget_weight();
}

int random_select() {
    std::uniform_int_distribution<> dis(0, int(C.size()) - 1);
    int offset = dis(gen), count = 0;
    auto v = C.begin();
    while (count < offset) {
        v++;
        count++;
    }
    if (tabu[*v] != 0) {
        tabu[*v]--;
        for (auto &u: C)
            if (tabu[u] == 0)
                return u;
    } else
        return *v;
    return -1;
}

/*
void prop_remove (const int vertex) {
    auto &_prop = S[vertex];
    std::vector<int> remove_list;
    std::stack<int> _stack;
    bitmap is_in_stack(n);

    for (auto &v: _prop) {
        std::vector<int> temp;
        for (auto &u: neighbor[v]) {
            if (can_prop(u)) {
                P.insert(v);
                S[u].insert(v);
                cc[v] = false;
                temp.emplace_back(v);
                remove_list.emplace_back(v);
                _stack.emplace(v);
                is_in_stack.set(v);
                break;
            }
        }
    }

    while (!_stack.empty()) {
        int u = _stack.top();
        _stack.pop();
        is_in_stack.reset(u);

        int t = get_prop_vertex(u);
        if (t != -1) {
            P.insert(t);
            remove_list.emplace_back(t);
            S[u].insert(t);
            if (!is_in_stack.find(t)) {
                _stack.emplace(t);
                is_in_stack.set(t);
            }
            for (auto &v: neighbor[t]) {
                if (!is_in_stack.find(v) && P.find(v) != P.end() && v != u) {
                    _stack.emplace(v);
                    is_in_stack.set(v);
                }
            }
        }
    }

    for (auto &v: remove_list) {
        S[vertex].erase(v);
        remove_score[v].score -= weight[v];
    }


}
*/


void back_prop(const int &vertex) {
    std::stack<int> _stack;
    bitmap is_in_stack(n);
    for (auto &u: S[vertex]) {
        if (C.find(u) != C.end()) {
            continue;
        }
        P.erase(u);
        _stack.emplace(u);
    }
    std::stack<int> add_prop;
    // debug

    while (!_stack.empty()) {
        int u = _stack.top();
        _stack.pop();
        is_in_stack.reset(u);
        for (auto &v: neighbor[u]) {
            if (can_prop(v)) {
                P.insert(u);
                int t = get_prop_vertex(u);
                if (t != -1)
                    add_prop.emplace(t);
                break;
            }
        }
    }

    while (!add_prop.empty()) {
        int u = add_prop.top();
        add_prop.pop();
        auto t = get_prop_vertex(u);
        if (t != -1) {
            P.insert(t);
            add_prop.emplace(t);
            for (auto &v: neighbor[t]) {
                if (P.find(v) != P.end() && v != u) {
                    add_prop.emplace(v);
                }
            }
        }
    }
}

// 将 v 从 C 中移除
// 在此过程中，我们的策略是 [function](back_prop)
// 将 S[v] 中的点全部从 P 中移除, 然后查看是否有不需要移除的点
void remove(const int &v) {
    C.erase(v);
    P.erase(v);
    cc[v] = false;

    for (auto &u: neighbor[v]) {
        cc[u] = true;
    }
/*    remove_observed(v);*/
//    prop_remove(v);
    back_prop(v);
    int age = remove_score[v].age;
    remove_score.erase(v);
    int score = 0;
    for (auto &u: neighbor[v]) {
        if (P.find(u) == P.end())
            score += weight[u];
    }
    if (P.find(v) == P.end())
        score += weight[v];
    add_score[v].score = score;
    add_score[v].age = age;
}


// 将 v 加入到 C 中
// 在此过程中，我们需要更新
// 1. C, P, N[v]的score值, 其中score的定义为 u 的邻居中不在 P 中的点的权重之和
// 2. 更新 S 集合, 其含义为 因 v 而被 prop 的点([function]check_prop())
// 3. 更新 v 在 remove_score 中的值, 其含义为 S[v] 中的点的权重之和
void add(const int &v) {
    C.insert(v);
    P.insert(v);
    S[v].insert(v);
    for (auto &u: neighbor[v]) {
        if (P.find(u) == P.end()) {
            P.insert(u);
            S[v].insert(u);
        }
        auto &t = add_score[u];
        t.score -= weight[v];
    }
    check_prop(v);

    int age = add_score[v].age + 1;
    add_score.erase(v);
    //! insert v to remove_score
    int score = weight[v];
    for (auto &u: S[v]) {
        score += weight[u];
    }
    auto &r = remove_score[v];
    r.score = score;
    r.age = age;
}

int select_remove_vertex() {
    std::vector<std::pair<int, vertex>> temp(remove_score.begin(), remove_score.end());
    std::sort(temp.begin(), temp.end(), [](std::pair<int, vertex> &lhs, std::pair<int, vertex> &rhs) {
        if (lhs.second.score == rhs.second.score)
            return lhs.second.age > rhs.second.age;
        return lhs.second.score < rhs.second.score;
    });
    /* debug */
    /* for(auto &i: temp){
        std::cout << "v: " << i.first << " score: " << i.second.score << std::endl;
    }*/
    int ret = -1;
    for (auto &i: temp) {
        if (cc[i.first]) {
            if (tabu[i.first] != 0)
                tabu[i.first]--;
            else {
                ret = i.first;
                break;
            }
        }
    }
    return ret;
}

int select_add_vertex() {
    std::vector<std::pair<int, vertex>> temp(add_score.begin(), add_score.end());
    std::sort(temp.begin(), temp.end(), [](std::pair<int, vertex> &lhs, std::pair<int, vertex> &rhs) {
        if (lhs.second.score == rhs.second.score)
            return lhs.second.age > rhs.second.age;
        return lhs.second.score > rhs.second.score;
    });
    /* debug */
    /* for(auto &i: temp){
        std::cout << "v: " << i.first << " score: " << i.second.score << std::endl;
    }*/
    int ret = -1;
    for (auto &i: temp) {
        if (cc[i.first]) {
            ret = i.first;
            break;
        } else if (cc[i.first] && ret == -1)
            ret = i.first;
    }
    return ret;
}

//! Greedy Search
void greedy_search() {
    while (P.size() != n) {
        auto v = select_add_vertex();
        add(v);
        remove_score[v].age = 0;
    }
    std::cout << "Greedy: " << C.size() << std::endl;
}


void pdsp() {

    greedy_search();

    while (C.size() > standard_answer || P.size() != n) {
        if (P.size() == n) {

            std::vector<std::pair<int, vertex>> temp(remove_score.begin(), remove_score.end());
            std::sort(temp.begin(), temp.end(), [](std::pair<int, vertex> &lhs, std::pair<int, vertex> &rhs) {
                if (lhs.second.score == rhs.second.score)
                    return lhs.second.age > rhs.second.age;
                return lhs.second.score < rhs.second.score;
            });
            if (temp[0].second.score == 0) {
                for (auto &v: temp) {
                    if (v.second.score > 0)
                        break;
                    if (v.second.score == 0)
                        remove(v.first);
                }
            }

            auto v = random_select();
            while (v == -1) v = select_add_vertex();
            remove(v);
            continue;
        }

        /* std::cout << n - P.size() << std::endl; */
        auto v = select_remove_vertex();
        if (v == -1) continue;
        remove(v);
        auto u = select_add_vertex();
        add(u);
        tabu[u] = 10; //! tabu
        update_weight();
    }
    std::cout << "PDSP: " << C.size() << std::endl;
}


#endif //PDSP_PDSP_H
