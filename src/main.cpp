#include "pds.hpp"
#include <fstream>
#include <iostream>

int main(int argc, const char *argv[]) {

  if (argc <= 2) {
    exit(1);
  }

  std::ifstream fin(argv[1]);
  std::ofstream fout(argv[2]);

  NuPDS pds;
  pds.init(fin);
  pds.solve();

  auto solution = pds.get_best_solution();
  fout << solution.size() << std::endl;
  for (auto &v : solution) {
    fout << v << " ";
  }
  fout << std::endl;

  return 0;
}
