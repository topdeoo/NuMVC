#include "fss.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

auto now() { return std::chrono::high_resolution_clock::now(); }

int main(int argc, const char *argv[]) {

  if (argc <= 2) {
    exit(1);
  }

  std::ifstream fin(argv[1]);
  std::ofstream fout(argv[2]);

  FSS fss;

  std::string t;
  fin >> t;

  fss.init(fin);

  auto t0 = now();

  fss.search();

  auto t1 = now();

  fout << t << std::endl;

  fout << std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count()
       << "us" << std::endl;

  auto solution = fss.get_best_solution();
  fout << solution.size() << std::endl;
  for (auto &v : solution) {
    fout << v << " ";
  }
  fout << std::endl;

  return 0;
}
