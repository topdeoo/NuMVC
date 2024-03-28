#pragma once

#ifndef __FSS_HPP__
#define __FSS_HPP__
#include "basic.hpp"
#include "candidate.hpp"
#include "graph.hpp"
#include <stack>

class FSS {
private:
  Graph graph_;
  Graph dependencies_;
  map<u32, u32> unobserved_degree_;

  CandidateSet candidate_;

  set<u32> solution_;
  set<u32> best_solution_;

public:
  // propagate function
  void observe(u32 vertex, u32 origin);
  void observe_one(u32 vertex, u32 origin, std::stack<u32> &stack_);
  void propagate(std::stack<u32> &stack_);

  // add & delete opreator
  void add_into_solution(u32 vertex);
  void remove_from_solution(u32 vertex);

  // helper function
  inline bool is_observed(u32 v);
  inline bool is_in_solution(u32 v);
  inline bool all_observed();

  // main search function
};

#endif // __FSS_HPP__