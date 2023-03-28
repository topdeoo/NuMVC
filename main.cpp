#include "pdsp.h"

//! build the initial graph
void init () {
    std::cin >> standard_answer;
    std::cin >> n >> m;
    weight.resize(n + 1);
    cc.resize(n + 1);
    tabu.resize(n + 1);
    std::fill(weight.begin(), weight.end(), true);
    std::fill(cc.begin(), cc.end(), 1);
    for (int i = 0; i < m; i++) {
        int from, to;
        std::cin >> from >> to;
        edges[from].emplace_back(to);
        edges[to].emplace_back(from);
        neighbor[from].emplace_back(to);
        neighbor[to].emplace_back(from);
    }
    //! for IEEE test case
    //! for other test case
    for (int v = 0; v < n; v++) {
/*    for (int v = 1; v <= n; v++) {*/
        int score = 0;
        for (auto &u: neighbor[v])
            score += weight[u];
        score += weight[v];
        add_score.insert({ v, vertex{ score, 0 }});
    }
    mean = 1;
}

bool check_observed (int father, std::unordered_set<int> &_prop) {
    if (C.find(father) != C.end())
        return true;
    if (neighbor[father].size() <= 1) {
        return true;
    } else {
        int cnt = 0;
        for (auto &v: neighbor[father]) {
            if (_prop.find(v) != _prop.end())
                cnt++;
        }
        return cnt >= neighbor[father].size() - 1;
    }
}

void prop (const int &vertex, std::unordered_set<int> &_prop) {
    bitmap is_in_stack(n);
    std::stack<std::pair<int, int>> _stack;
    for (auto &v: neighbor[vertex]) {
        for (auto &u: neighbor[v]) {
            if (is_in_stack.find(u) || _prop.find(u) != _prop.end())
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
        if (check_observed(u.second, _prop)) {
            _prop.insert(u.first);
            for (auto &v: neighbor[u.first]) {
                bool flag = _prop.find(v) != _prop.end();
                if (!is_in_stack.find(v) && !flag) {
                    _stack.emplace(v, u.first);
                    is_in_stack.set(v);
                }
            }
        }
    }
}

void check_solution () {
    std::unordered_set<int> observed_set;
    for (auto &v: C) {
        observed_set.insert(v);
        for (auto &u: neighbor[v]) {
            observed_set.insert(u);
        }
        prop(v, observed_set);
    }
    if (observed_set.size() != n)
        std::cout << "Not Correct" << std::endl;
    else
        std::cout << "Completely Dominating" << std::endl;
}

int main () {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    /*std::ifstream ifs("example.in");*/
    init();
    pdsp();
/*    if (P.size() == n)
        std::cout << "Completely Dominating" << std::endl;
    else
        std::cout << "Not Correct" << std::endl;*/
    check_solution();
    return 0;
}
