#include "basic.hpp"
#include "graph.hpp"
#include <fstream>
#include <iostream>
#include <stack>

set<u32> observed;
map<u32, u32> unobserved_deg;

void propagate(std::stack<u32> &stack_, Graph &g) {
  while (!stack_.empty()) {
    auto v = stack_.top();
    stack_.pop();
    for (auto &w : g.get_neighbors(v)) {
      if (observed.find(w) == observed.end()) {
        observed.insert(w);
        stack_.push(w);
      }
    }
  }
}

bool check(Graph &g, const set<u32> &sol) {

  for (auto &v : g.vertices()) {
    unobserved_deg[v] = g.degree(v);
  }

  for (auto &v : sol) {
    observed.insert(v);
    std::stack<u32> stack_;
    for (auto &u : g.get_neighbors(v)) {
      unobserved_deg[u] -= 1;
      observed.insert(u);
      if (unobserved_deg[u] == 1) {
        stack_.push(u);
      }
    }
    propagate(stack_, g);
  }

  return observed.size() == g.vertex_nums();
}

int main(int argc, const char *argv[]) {
  if (argc <= 2) {
    exit(1);
  }
  std::ifstream fin(argv[1]);
  std::ifstream sol(argv[2]);

  std::string t;
  fin >> t;

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

  sol >> t;
  sol >> t;

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
