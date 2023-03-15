//
// Created by LENOVO on 2023/3/11.
//

//! TODO: PROP property check maybe wrong

#ifndef PDSP_PDSP_H
#define PDSP_PDSP_H

#include "basic.h"

bool can_observed (int vertex) {
    if (neighbor.find(vertex) == neighbor.end())
        return false;
    if (neighbor[vertex].size() == 1 && P.find(neighbor[vertex][0]) != P.end())
        return true;
    else {
        int cnt = 0;
        for (auto &v: neighbor[vertex]) {
            if (P.find(v) != P.end())
                cnt++;
        }
        return cnt >= neighbor[vertex].size() - 1;
    }
}


//! PROP property
void check_prop (int vertex) {
    bitmap is_in_stack(n);
    std::stack<int> check_stack;
    for (auto &v: neighbor[vertex]) {
        for (auto &u: neighbor[v]) {
            if (is_in_stack.find(u) || P.find(u) != P.end())
                continue;
            else {
                check_stack.push(u);
                is_in_stack.set(u);
            }
        }
    }
    while (!check_stack.empty()) {
        int u = check_stack.top();
        check_stack.pop();
        is_in_stack.reset(u);
        if (can_observed(u)) {
            //! if u can be observed, then u is in P
            //! update S[u] by N(u) if N(u) is in
            P.insert(u);
            for (auto &v: neighbor[u]) {
                bool flag = P.find(v) != P.end();
                if (flag)
                    S[u].set(v);
                if (!is_in_stack.find(v) && !flag) {
                    check_stack.push(v);
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

    //! can optimize
    for (auto &v: add_score) {
        int temp = 0;
        for (auto &u: neighbor[v.v]) {
            if (P.find(u) != P.end())
                temp += weight[u];
        }
        const_cast <vertex &> (v).score = ceil(v.score * roi);
    }

    for (auto &v: remove_score) {
        int temp = 0;
        for (auto &u: neighbor[v.v]) {
            if (S[v.v].find(u))
                temp += weight[u];
        }
        const_cast <vertex &> (v).score = ceil(v.score * roi);
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


//! build the initial graph
void init () {
    std::cin >> standard_answer;
    std::cin >> n >> m;
    weight.resize(n + 1);
    std::fill(weight.begin(), weight.end(), 1);
    for (int i = 0; i < m; i++) {
        int from, to;
        std::cin >> from >> to;
        edges[from].emplace_back(to);
        edges[to].emplace_back(from);
        neighbor[from].emplace_back(to);
        neighbor[to].emplace_back(from);
    }
    for (int v = 1; v <= n; v++) {
        S.insert(std::make_pair(v, bitmap(neighbor[v].size())));
        int score = 0;
        for (auto &u: neighbor[v]) {
            S[v].set(u);
            score += weight[u];
        }
        add_score.insert(vertex{ v, score, 0 });
        cc.emplace_back(0);
    }
    mean = 1;
}


//! Greedy Search
//! TODO: do not erase the vertex from add_score and remove_score, just update the score and the age
void greedy_search () {
    while (P.size() != n) {
        auto &v = *(add_score.rbegin());
        add_score.erase(v);

        C.insert(v.v);
        P.insert(v.v);
        for (auto &u: neighbor[v.v]) {
            P.insert(u);
        }
        check_prop(v.v);
        update_weight();
    }
    std::cout << "Greedy: " << C.size() << std::endl;
}

vertex &random_select () {
    //! TODO: select a vertex from C randomly
    std::uniform_int_distribution<> dis(0, int(remove_score.size() - 1));
    int idx = dis(gen);
    auto &v = *(remove_score.begin());
    return const_cast <vertex &> (v);
}

void remove (const vertex &v) {
    //! TODO: remove vertex from C
    C.erase(v.v);
}

void add (vertex &v) {
    //! TODO: add vertex to C
    C.insert(v.v);
    P.insert(v.v);
    cc[v.v] = 0;
    for (auto &u: neighbor[v.v]) {
        P.insert(u);
        cc[u] = 1;
        //TODO: update u in add_score
        auto &t = *(add_score.find(vertex{ u, 0, 0 }));
        const_cast <vertex &> (t).score -= weight[v.v];
    }
    check_prop(v.v);
    v.score = -v.score;
    v.age = v.age + 1;
}

vertex &select_remove_vertex (vertex &v) {
    int max_age = -1, min_score = 1e9;
    for (auto &u: remove_score) {
        if (min_score < u.score)
            break;
        if (cc[u.v] == 1) {
            if (u.score < min_score) {
                min_score = u.score;
                max_age = u.age;
                v = u;
            } else if (u.age > max_age) {
                max_age = u.age;
                v = u;
            }
        }
    }
    return v;
}

vertex &select_add_vertex (vertex &v) {
    int max_age = -1, max_score = -1;
    for (std::reverse_iterator<std::set<vertex>::iterator> it(add_score.rbegin()); it != add_score.rend();
         ++it) {
        if (max_score > it->score)
            break;
        if (cc[it->v] == 1) {
            if (it->score > max_score) {
                max_score = it->score;
                max_age = it->age;
                v = *it;
            } else if (it->age > max_age) {
                max_age = it->age;
                v = *it;
            }
        }
    }
    return v;
}

void pdsp () {
    greedy_search();
    //! TODO: condition is not correct
    while (C.size() > standard_answer){
        if (P.size() == n) {
            auto &v = random_select();
            remove(v);
            continue;
        }
        auto v = vertex{ 0, 0, 0 };
        select_remove_vertex(v);
        remove(v);
        auto u = vertex{ 0, 0, 0 };
        select_add_vertex(u);
        add(u);
        update_weight();
    }
}


#endif //PDSP_PDSP_H
