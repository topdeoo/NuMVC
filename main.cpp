//! @file main program
//! @author Virgil
#include "basic.h"
#include "checker.h"
#include "pdsp.h"
#include <chrono>
#include <fstream>

// TODO(virgil) refactor this project

// DONE(virgil) add comments for each function

// TAG change when change test case
//! @fn init
//! @brief initialize the graph with console input
void init() {
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
  /*    for (int v = 0; v < n; v++) {*/
  for (int v = 1; v <= n; v++) {
    Link[v] = 0;
    int score = 0;
    for (auto &u : neighbor[v])
      score += weight[u];
    score += weight[v];
    add_score.insert({v, vertex{score, 0}});
  }
  mean = 1;
}

// DONE (virgil) Add an initialization function for reading from a file
//! @fn init
//! @brief initialize the graph with file input
void init(std::ifstream &input) {
  input >> standard_answer;
  input >> n >> m;
  weight.resize(n + 1);
  cc.resize(n + 1);
  tabu.resize(n + 1);
  std::fill(weight.begin(), weight.end(), true);
  std::fill(cc.begin(), cc.end(), 1);
  for (int i = 0; i < m; i++) {
    int from, to;
    input >> from >> to;
    edges[from].emplace_back(to);
    edges[to].emplace_back(from);
    neighbor[from].emplace_back(to);
    neighbor[to].emplace_back(from);
  }
  //! for IEEE test case
  //! for other test case
  /*    for (int v = 0; v < n; v++) {*/
  for (int v = 1; v <= n; v++) {
    Link[v] = 0;
    int score = 0;
    for (auto &u : neighbor[v])
      score += weight[u];
    score += weight[v];
    add_score.insert({v, vertex{score, 0}});
  }
  mean = 1;
}

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
#if defined(__DEBUG__)
  std::ifstream ifs("../testcase/IEEE-30.graph");
  init(ifs);
#endif
  init();

  auto start = std::chrono::steady_clock::now();
  pdsp();
  auto end = std::chrono::steady_clock::now();
  std::cout << "Elapsed time in milliseconds: "
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     start)
                   .count()
            << " us" << std::endl;
  // debug: test case is IEEE-30.graph
  /*add(6);
  add(10);
  add(27);
  std::cout << Link[12];
  remove(6);
  std::cout << '\n' << Link[12] << '\n';
  for (auto &i: P) {
      std::cout << i << " ";
  }*/
  checker c(C);
  c.check();
  c.is_correct();
  return 0;
}

/*
12 13
1 2
1 4
1 3
1 5
2 12
3 11
3 10
3 9
4 8
4 6
4 7
5 8
7 8
 */
