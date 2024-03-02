#pragma once

#ifndef __FSS_HPP__
#define __FSS_HPP__
#include "basic.hpp"
#include "graph.hpp"
#include <algorithm>
#include <fstream>
#include <random>
#include <stack>
#include <vector>

class FSS {
private:
  u32 timestarmp_;
  u32 cutoff_;
  double size_;
  double factor_;
  u32 k_;
  u32 N_;
  Graph graph_;
  Graph dependencies_;
  map<u32, u32> unobserved_degree_;

  set<u32> solution_;
  set<u32> best_solution_;

  std::vector<set<u32>> population_;
  u32 population_size_;
  std::pair<u32, u32> worst_solution_;

  std::vector<set<u32>> skn_;
  set<u32> b_;

  u32 stagnation_;

public:
  FSS() {
    timestarmp_ = 0;
    size_ = 0;
    factor_ = 1;
    population_size_ = 100;
    k_ = 10;
    N_ = 101;
  };

  FSS(Graph &graph) : graph_(graph) {
    timestarmp_ = 0;
    size_ = 0;
    factor_ = 1;
    population_size_ = 100;
    k_ = 10;
    N_ = 101;
  };

public:
  void search();
  void grasp();
  void clear();
  void init(std::ifstream &fin);
  set<u32> get_best_solution();
  void fix_search();
  void greedy();
  void local_search();
  // void Ob(std::vector<std::pair<u32, double>> &candidate, u32 v);
  void observe(u32 vertex, u32 origin);
  void observe_one(u32 vertex, u32 origin, std::stack<u32> &stack_);
  void propagate(std::stack<u32> &stack_);
  inline bool is_observed(u32 v);
  inline bool is_in_solution(u32 v);
  bool all_observed();
  void add_into_solution(u32 vertex);
  void remove_from_solution(u32 vertex);
  void next_size();
};

#endif // __FSS_HPP__