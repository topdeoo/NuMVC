#include "pdsp.h"

//! build the initial graph
void init () {
    std::cin >> standard_answer;
    std::cin >> n >> m;
    weight.resize(n + 1);
    cc.resize(n + 1);
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
    for(int v = 0; v < n; v++){
/*    for (int v = 1; v <= n; v++) {*/
        int score = 0;
        for (auto &u: neighbor[v])
            score += weight[u];
        add_score.insert({ v, vertex{ score, 0 }});
    }
    mean = 1;
}

int main () {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    /*std::ifstream ifs("example.in");*/
    init();
    pdsp();
    return 0;
}
