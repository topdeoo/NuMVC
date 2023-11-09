#include "pds.hpp"
#include "basic.hpp"
#include "heap.h"
#include <algorithm>
#include <cstdint>
#include <optional>
#include <stack>
#include <utility>
#include <vector>

void NuPDS::init(std::ifstream &fin) {
  u32 n, m;
  fin >> n >> m;

  for (u32 i = 0; i < m; i++) {
    u32 from, to;
    fin >> from >> to;
    graph_.add_edge(from, to);
    graph_.add_edge(to, from);
  }

  u32 k;
  fin >> k;
  for (u32 i = 0; i < k; i++) {
    u32 v;
    fin >> v;
    pre_selected_.insert(v);
  }

  u32 l;
  fin >> l;
  for (u32 i = 0; i < l; i++) {
    u32 v;
    fin >> v;
    excluded_.insert(v);
  }

  timestamp_ = 0;
  tabu_size_ = 4;
  alpha_ = 0.5;

  for (auto &v : graph_.vertices()) {
    unobserved_degree_[v] = graph_.degree(v);
    age[v] = 0;
    cc_[v] = true;
    tabu_[v] = 0;
  }

  // TODO
  cutoff_ = 1000;

  init_heap(n, graph_);
}

void NuPDS::pre_process() {

  add_heap_index = 0;
  remove_heap_index = 0;

  for (auto &v : pre_selected_) {
    add_into_solution(v);
    tabu_[v] = 0;
  }
}

void NuPDS::observe(u32 vertex, u32 origin) {
  std::stack<u32> stack_;
  observe_one(vertex, origin, stack_);
  propagate(stack_);
}

void NuPDS::observe_one(u32 vertex, u32 origin, std::stack<u32> &stack_) {
  if (!is_observed(vertex)) {
    dependencies_.add_vertex(vertex);
    if (origin != vertex)
      dependencies_.add_edge(origin, vertex);
    if (unobserved_degree_[vertex] == 1)
      stack_.push(vertex);
    for (auto w : graph_.get_neighbors(vertex)) {
      cc_[w] = true;
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
          break;
        }
      }
    }
  }
}

bool NuPDS::is_observed(u32 v) { return dependencies_.has_vertex(v); }

bool NuPDS::all_observed() {
  return dependencies_.vertex_nums() == graph_.vertex_nums();
}

bool NuPDS::is_in_solution(u32 v) {
  return solution_.find(v) != solution_.end();
}

bool NuPDS::not_exculded(u32 v) { return excluded_.find(v) == excluded_.end(); }

bool NuPDS::is_tabu(u32 v) { return tabu_[v] != 0; }

u32 NuPDS::Ob(u32 v) {
  u32 score = 0;
  if (dependencies_.has_vertex(v)) {
    auto neighbors = dependencies_.get_in_neighbors(v);
    for (auto &u : neighbors) {
      dependencies_.remove_edge(u, v);
    }
  }

  std::stack<u32> stack_;
  observe_one(v, v, stack_);
  for (auto w : graph_.get_neighbors(v)) {
    observe_one(w, v, stack_);
  }
  propagate(stack_);
  score = dependencies_.vertex_nums();

  std::stack<u32> propagating;
  while (!stack_.empty())
    stack_.pop();
  set<u32> enqueued;
  stack_.push(v);
  enqueued.insert(v);

  while (!stack_.empty()) {
    auto u = stack_.top();
    stack_.pop();
    std::optional<u32> observer;
    for (auto w : graph_.get_neighbors(u)) {
      unobserved_degree_[w] += 1;

      if (!is_in_solution(w)) {
        // w is observed from u
        if (dependencies_.has_edge(u, w)) {
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
    dependencies_.remove_vertex(u);
    if (observer.has_value()) {
      observe_one(v, observer.value(), propagating);
    }
  }
  propagate(propagating);
  return score - dependencies_.vertex_nums();
}

void NuPDS::add_into_solution(u32 v) {
  solution_.insert(v);

  tabu_[v] = tabu_size_;

  u32 score = dependencies_.vertex_nums();

  if (dependencies_.has_vertex(v)) {
    auto neighbors = dependencies_.get_in_neighbors(v);
    for (auto &u : neighbors) {
      dependencies_.remove_edge(u, v);
    }
  }
  std::stack<u32> stack_;
  observe_one(v, v, stack_);
  for (auto w : graph_.get_neighbors(v)) {
    cc_[w] = true;
    observe_one(w, v, stack_);
  }
  propagate(stack_);
  score = dependencies_.vertex_nums() - score;
  insert(
      {
          v,
          score,
      },
      HEAPTYPE::REMOVE);
}

void NuPDS::remove_from_solution(u32 v) {
  std::stack<u32> propagating;
  std::stack<u32> stack_;
  set<u32> enqueued;

  tabu_[v] = tabu_size_;

  solution_.erase(v);
  cc_[v] = false;
  stack_.push(v);
  enqueued.insert(v);

  u32 score = 0;

  while (!stack_.empty()) {
    auto v = stack_.top();
    stack_.pop();
    std::optional<u32> observer;
    for (auto w : graph_.get_neighbors(v)) {
      cc_[w] = true;
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
            for (auto x : dependencies_.get_out_neighbors(w)) {
              if (enqueued.find(x) == enqueued.end()) {
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

  score = dependencies_.vertex_nums();

  propagate(propagating);

  score = dependencies_.vertex_nums() - score;

  insert({v, score}, HEAPTYPE::ADD);
}

u32 NuPDS::select_add_vertex() {
  std::vector<std::pair<u32, u32>> candidates;
  u32 ret = UINT32_MAX;
  while (add_heap_index > 0) {
    if (add_heap_index == 0)
      break;
    auto v = top(HEAPTYPE::ADD);
    pop(HEAPTYPE::ADD);
    if (cc_[v.first] && !is_tabu(v.first) && not_exculded(v.first) &&
        !is_in_solution(v.first)) {
      ret = v.first;
      break;
    } else if (!is_in_solution(v.first) && not_exculded(v.first)) {
      tabu_[v.first]--;
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
  u32 ret = INT32_MAX;
  while (remove_heap_index > 0) {
    if (remove_heap_index == 0)
      break;
    auto v = top(HEAPTYPE::REMOVE);
    pop(HEAPTYPE::REMOVE);
    if (cc_[v.first] && !is_tabu(v.first) && is_in_solution(v.first)) {
      ret = v.first;
      break;
    } else if (is_in_solution(v.first)) {
      tabu_[v.first]--;
      candidates.push_back(v);
    }
  }
  for (auto &v : candidates) {
    insert(v, HEAPTYPE::REMOVE);
  }
  return ret;
}

void NuPDS::tabu_forget() {}

void NuPDS::update_score() {}

void NuPDS::greedy() {

  while (1) {

    // auto v = top(HEAPTYPE::ADD);
    // pop(HEAPTYPE::ADD);
    // add_into_solution(v.first);
    // age[v.first] = timestamp_;
    // tabu_.pop();
    // in_tabu_.erase(v.first);
    // timestamp_++;

    // // rebuild add heap
    // if (all_observed()) {
    //   std::vector<std::pair<u32, u32>> candidates;
    //   for (auto &u : graph_.vertices()) {
    //     if (!is_in_solution(u) && not_exculded(u)) {
    //       u32 score = Ob(u);
    //       candidates.push_back({u, score});
    //     }
    //   }
    //   add_heap_index = 0;
    //   for (auto &u : candidates) {
    //     if (!is_in_solution(u.first) && not_exculded(u.first)) {
    //       insert({u.first, u.second}, HEAPTYPE::ADD);
    //     }
    //   }
    //   break;
    // }

    std::vector<std::pair<u32, u32>> candidates;
    for (auto &v : graph_.vertices()) {
      if (!is_in_solution(v) && not_exculded(v)) {
        u32 score = Ob(v);
        candidates.push_back({v, score});
      }
    }
    std::sort(candidates.begin(), candidates.end(),
              [](auto &a, auto &b) { return a.second > b.second; });
    add_into_solution(candidates[0].first);
    tabu_[candidates[0].first] = 0;
    age[candidates[0].first] = timestamp_;
    timestamp_++;

    if (timestamp_ == 1) {
      for (auto &v : candidates) {
        base_score_[v.first] = v.second;
      }
    }

    // rebuild add heap
    if (all_observed()) {
      add_heap_index = 0;
      for (auto &v : candidates) {
        if (!is_in_solution(v.first) && not_exculded(v.first)) {
          insert({v.first, v.second}, HEAPTYPE::ADD);
        }
      }
      break;
    }
  }
}

void NuPDS::solve() {

  if (!all_observed()) {
    greedy();
  }

  if (solution_.size() == 1) {
    best_solution_ = solution_;
    return;
  }

  for (auto &[k, v] : unobserved_degree_) {
    v = 0;
  }

  while (timestamp_ < cutoff_) {
    timestamp_++;
    if (all_observed()) {
      best_solution_ = solution_;
      auto v = select_remove_vertex();
      if (v != INT32_MAX) {
        remove_from_solution(v);
        age[v] = timestamp_;
      } else {
        tabu_forget();
      }
      continue;
    }

    auto v_remove = select_remove_vertex();
    auto v_add = select_add_vertex();
    if (v_remove == INT32_MAX || v_add == INT32_MAX) {
      tabu_forget();
      continue;
    }

    remove_from_solution(v_remove);
    age[v_remove] = timestamp_;
    add_into_solution(v_add);
    age[v_add] = timestamp_;

    update_score();
  }
}

const set<u32> &NuPDS::get_solution() const { return solution_; }

const set<u32> &NuPDS::get_best_solution() const { return best_solution_; }
