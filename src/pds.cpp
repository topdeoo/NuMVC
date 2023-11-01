#include "pds.hpp"
#include "basic.hpp"
#include "heap.h"
#include <optional>
#include <stack>
#include <utility>

void NuPDS::init(std::ifstream &fin) {
  u32 n, m;
  fin >> n >> m;

  for (int i = 0; i < m; i++) {
    u32 from, to;
    fin >> from >> to;
    graph_.add_edge(from, to);
    graph_.add_edge(to, from);
  }

  timestamp_ = 0;

  for (auto &v : graph_.vertices()) {
    unobserved_degree_[v] = graph_.degree(v);
    age[v] = 0;
    cc[v] = true;
    tabu[v] = 10;
  }

  // TODO
  cutoff_ = 100;

  init_heap(n, graph_);
}

void NuPDS::observe(u32 vertex, u32 origin) {
  std::stack<u32> stack_;
  observe_one(vertex, origin, stack_);
  propagate(stack_);
}

void NuPDS::observe_one(u32 vertex, u32 origin, std::stack<u32> &stack_) {
  if (!is_observed(vertex)) {
    observed_.insert(vertex);
    if (origin != vertex)
      dependencies_.add_edge(origin, vertex);
    if (unobserved_degree_[vertex] == 1)
      stack_.push(vertex);
    for (auto w : graph_.get_neighbors(vertex)) {
      unobserved_degree_[w] -= 1;
      if (unobserved_degree_[w] == 1 && is_observed(w))
        stack_.push(w);
    }
  }
}

void NuPDS::propagate(std::stack<u32> &stack_) {
  while (!stack_.empty()) {
    auto v = stack_.top();
    stack_.pop();
    if (is_observed(v) && unobserved_degree_[v] == 1) {
      for (auto w : graph_.get_neighbors(v)) {
        if (!is_observed(w)) {
          observe_one(w, v, stack_);
        }
      }
    }
  }
}

bool NuPDS::is_observed(u32 v) { return observed_.find(v) != observed_.end(); }

bool NuPDS::all_observed() { return observed_.size() == graph_.vertex_nums(); }

bool NuPDS::is_in_solution(u32 v) {
  return solution_.find(v) != solution_.end();
}

void NuPDS::add_into_solution(u32 v) {
  if (!is_in_solution(v)) {
    solution_.insert(v);
    if (dependencies_.has_vertex(v)) {
      while (dependencies_.in_degree(v) > 0) {
        auto edge = *dependencies_.get_in_edges(v).begin();
        dependencies_.remove_edge(edge, v);
      }
    }
    std::stack<u32> stack_;
    observe_one(v, v, stack_);
    for (auto w : graph_.get_neighbors(v)) {
      cc[w] = true;
      observe_one(w, v, stack_);
    }
    propagate(stack_);
    insert({v, dependencies_.in_degree(v)}, HEAPTYPE::REMOVE);
  }
}

void NuPDS::remove_from_solution(u32 v) {
  if (is_in_solution(v)) {
    std::stack<u32> propagating;
    std::stack<u32> stack_;
    set<u32> enqueued;

    solution_.erase(v);
    stack_.push(v);
    enqueued.insert(v);
    while (!stack_.empty()) {
      auto v = stack_.top();
      stack_.pop();
      std::optional<u32> observer;
      for (auto w : graph_.get_neighbors(v)) {
        unobserved_degree_[w] += 1;
        if (!is_in_solution(w)) {
          // w is observed from v
          if (dependencies_.has_edge(v, w)) {
            if (enqueued.find(w) == enqueued.end()) {
              stack_.push(w);
              enqueued.insert(w);
            }
          } else if (is_observed(w)) {
            if (dependencies_.has_neighbors(w)) {
              for (auto x : dependencies_.get_neighbors(w)) {
                if (enqueued.find(w) == enqueued.end()) {
                  stack_.push(x);
                  enqueued.insert(x);
                }
              }
            }
            propagating.push(w);
          }
        } else {
          observer = {w};
        }
      }
      // mark unobserved
      dependencies_.remove_vertex(v);
      if (observer.has_value()) {
        observe_one(v, observer.value(), propagating);
      }
    }
    propagate(propagating);
  }
  insert({v, unobserved_degree_[v]}, HEAPTYPE::ADD);
}

u32 NuPDS::select_add_vertex() {
  std::vector<std::pair<u32, u32>> candidates;
  u32 ret = 0;
  while (1) {
    auto v = top(HEAPTYPE::ADD);
    pop(HEAPTYPE::ADD);
    if (cc[v.first] && tabu[v.first] == 0) {
      ret = v.first;
      tabu[v.first] = 10;
      break;
    } else {
      tabu[v.first]--;
      candidates.push_back(v);
    }
  }
  for (auto &v : candidates) {
    insert(v, HEAPTYPE::ADD);
  }
  return ret;
}

u32 NuPDS::select_remove_vertex() {
  std::vector<std::pair<u32, u32>> candidates;
  u32 ret = 0;
  while (1) {
    auto v = top(HEAPTYPE::REMOVE);
    pop(HEAPTYPE::REMOVE);
    if (cc[v.first] && tabu[v.first] == 0) {
      ret = v.first;
      tabu[v.first] = 10;
      break;
    } else {
      tabu[v.first]--;
      candidates.push_back(v);
    }
  }
  for (auto &v : candidates) {
    insert(v, HEAPTYPE::REMOVE);
  }
  return ret;
}

void NuPDS::greedy() {
  while (observed_.size() != graph_.vertex_nums()) {
    auto v = top(HEAPTYPE::ADD);
    pop(HEAPTYPE::ADD);
    add_into_solution(v.first);
    age[v.first] = timestamp_;
    timestamp_++;
  }
}

void NuPDS::solve() {
  greedy();
  while (timestamp_ < cutoff_ && solution_.size() > 1) {
    timestamp_++;
    if (observed_.size() == graph_.vertex_nums()) {
      best_solution_ = solution_;
      auto v = top(HEAPTYPE::REMOVE);
      pop(HEAPTYPE::REMOVE);
      remove_from_solution(v.first);
      age[v.first] = timestamp_;
      continue;
    }

    auto v_remove = select_remove_vertex();
    remove_from_solution(v_remove);
    age[v_remove] = timestamp_;
    auto v_add = select_add_vertex();
    pop(HEAPTYPE::ADD);
    add_into_solution(v_add);
    age[v_add] = timestamp_;
  }
  if (best_solution_.empty()) {
    best_solution_ = solution_;
  }
}

const set<u32> &NuPDS::get_solution() const { return solution_; }

const set<u32> &NuPDS::get_best_solution() const { return best_solution_; }
