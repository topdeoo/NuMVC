#pragma once

#ifndef PDS_HPP
#define PDS_HPP
#include "basic.hpp"
#include "graph.hpp"
#include <chrono>
#include <cstdint>
#include <fstream>
#include <stack>
#include <vector>
class NuPDS {
public:
  NuPDS() = default;
  ~NuPDS() = default;
  NuPDS(const NuPDS &) = default;
  NuPDS(NuPDS &&) = default;
  NuPDS(const Graph &graph) : graph_(graph){};
  void init(std::ifstream &fin);
  void pre_process();

private:
  Graph graph_;
  Graph dependencies_;
  set<u32> observed_;
  set<u32> solution_;
  map<u32, u32> unobserved_degree_;
  map<u32, bool> cc;
  map<u32, u32> tabu;
  set<u32> best_solution_;
  u32 timestamp_;
  u32 cutoff_;
  set<u32> pre_selected_; // already in solution
  set<u32> excluded_;     // will nerver in solution

public:
  void solve();
  void greedy();
  void observe(u32 vertex, u32 origin);
  void observe_one(u32 vertex, u32 origin, std::stack<u32> &stack_);
  void propagate(std::stack<u32> &stack_);
  bool is_observed(u32 v);
  bool is_in_solution(u32 v);
  bool not_exculded(u32 v);
  void add_into_solution(u32 v);
  void remove_from_solution(u32 v);
  u32 select_add_vertex();
  u32 select_remove_vertex();
  bool all_observed();
  u32 Ob(u32 v);

public:
  const set<u32> &get_solution() const;
  const set<u32> &get_best_solution() const;
};

#endif // PDS_HPP