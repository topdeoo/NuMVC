#include "basic.hpp"
#include "graph.hpp"
#include "pds.hpp"
#include <iostream>
#include <stack>

bool check(const Graph &g, const set<u32> &sol) {
  NuPDS pds(g);
  for (auto &v : sol) {
    std::stack<u32> stack_;
    pds.observe_one(v, v, stack_);
    for (auto &w : g.get_in_edges(v)) {
      pds.observe_one(w, v, stack_);
    }
    pds.propagate(stack_);
  }
  return pds.all_observed();
}

int main(int argc, const char *argv[]) {
  if (argc <= 2) {
    exit(1);
  }
  std::ifstream fin(argv[1]);
  std::ifstream sol(argv[2]);

  Graph g;
  u32 n, m;
  fin >> n >> m;
  for (u32 i = 0; i < m; i++) {
    u32 u, v;
    fin >> u >> v;
    g.add_edge(u, v);
    g.add_edge(v, u);
  }

  set<u32> solution;

  u32 k;
  sol >> k;
  for (u32 i = 0; i < k; i++) {
    u32 v;
    sol >> v;
    solution.insert(v);
  }

  std::cout << argv[2] << " ";

  if (check(g, solution)) {
    std::cout << "OK" << std::endl;
  } else {
    std::cout << "WA" << std::endl;
  }

  return 0;
}
