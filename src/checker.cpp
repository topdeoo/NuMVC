#include "basic.hpp"
#include "graph.hpp"
#include <fstream>
#include <iostream>
#include <stack>

set<u32> observed;
map<u32, u32> unobserved_deg;

void observed_one(u32 vertex, u32 origin, std::stack<u32> &stack, Graph &g) {
  if (observed.find(vertex) != observed.end())
    return;
  observed.insert(vertex);
  if (unobserved_deg[vertex] == 1) {
    stack.push(vertex);
  }
  for (auto &w : g.get_neighbors(vertex)) {
    if (unobserved_deg[w] > 0)
      unobserved_deg[w] -= 1;
    if (unobserved_deg[w] == 1 && observed.find(w) == observed.end()) {
      stack.push(w);
    }
  }
}

void propagate(std::stack<u32> &stack_, Graph &g) {
  while (!stack_.empty()) {
    auto v = stack_.top();
    stack_.pop();
    if (observed.find(v) != observed.end() && unobserved_deg[v] == 1) {
      for (auto &w : g.get_neighbors(v)) {
        if (observed.find(w) == observed.end()) {
          observed_one(w, v, stack_, g);
          break;
        }
      }
    }
    for (auto &w : g.vertices()) {
      if (unobserved_deg[w] == 1 && observed.find(w) != observed.end()) {
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
    std::stack<u32> stack_;
    observed_one(v, v, stack_, g);
    for (auto &w : g.get_neighbors(v)) {
      observed_one(w, v, stack_, g);
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
