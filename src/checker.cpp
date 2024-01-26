#include "basic.hpp"
#include "fss.hpp"
#include "graph.hpp"
#include <fstream>
#include <iostream>
#include <stack>

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

  FSS fss(g);

  sol >> t;

  u32 k;
  sol >> k;
  for (u32 i = 0; i < k; i++) {
    u32 v;
    sol >> v;
    fss.add_into_solution(v);
  }

  std::cout << argv[2] << " ";

  if (fss.all_observed()) {
    std::cout << "OK" << std::endl;
  } else {
    std::cout << "WA" << std::endl;
  }

  return 0;
}
