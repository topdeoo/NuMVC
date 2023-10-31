#include "pds.hpp"
#include <fstream>
#include <iostream>

int main(int argc, const char *argv[]) {
  if (argc <= 1) {
    exit(1);
  }
  std::ifstream fin(argv[1]);
  NuPDS pds;
  pds.init(fin);
  pds.solve();

  auto solution = pds.get_best_solution();
  std::cout << solution.size() << std::endl;
  for (auto &v : solution) {
    std::cout << v << std::endl;
  }

  return 0;
}
