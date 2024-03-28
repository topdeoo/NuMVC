#include "fss.hpp"
#include <chrono>
#include <fstream>
#include <iostream>

auto now() { return std::chrono::high_resolution_clock::now(); }

int main(int argc, const char *argv[]) {

  if (argc <= 2) {
    exit(1);
  }

  std::ifstream fin(argv[1]);
  std::ofstream fout(argv[2]);

  FSS fss;

  fss.init(fin);

  fss.pre_processing();

  auto t0 = now();

  fss.search();

  auto t1 = now();

  if (fss.beyond_time) {
    fout << "Time limit exceeded" << std::endl;
    fout << "1800s" << std::endl;
  } else {
    fout << std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0)
                .count()
         << "us" << std::endl;
  }
  auto solution = fss.get_best_solution();
  fout << solution.size() << std::endl;
  for (auto &v : solution) {
    fout << v << " ";
  }
  fout << std::endl;

  return 0;
}
