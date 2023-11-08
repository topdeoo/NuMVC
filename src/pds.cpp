#include "pds.hpp"
#include "basic.hpp"
#include "heap.h"
#include <algorithm>
#include <cstdint>
#include <optional>
#include <stack>
#include <utility>
#include <vector>

#define TABU 2

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

  for (auto &v : graph_.vertices()) {
    unobserved_degree_[v] = graph_.degree(v);
    age[v] = 0;
    cc[v] = true;
    tabu[v] = 0;
  }

  // TODO
  cutoff_ = 100000;

  init_heap(n, graph_);
}

void NuPDS::pre_process() {
  for (auto &v : pre_selected_) {
    add_into_solution(v);
  }
  while (add_heap_index > 0) {
    pop(HEAPTYPE::ADD);
  }
  while (remove_heap_index > 0) {
    pop(HEAPTYPE::REMOVE);
  }
  for (auto &v : graph_.vertices()) {
    if (!is_in_solution(v) && not_exculded(v)) {
      insert({v, unobserved_degree_[v]}, HEAPTYPE::ADD);
    }
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
      cc[w] = true;
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
        cc[w] = true;
        if (!is_observed(w)) {
          observe_one(w, v, stack_);
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

u32 NuPDS::Ob(u32 v) {
  u32 score = 0;
  if (dependencies_.has_vertex(v)) {
    while (dependencies_.in_degree(v) > 0) {
      auto edge = *dependencies_.get_in_edges(v).begin();
      dependencies_.remove_edge(edge, v);
      dependencies_.remove_edge(v, edge);
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
  tabu[v] = TABU;
  if (dependencies_.has_vertex(v)) {
    while (dependencies_.in_degree(v) > 0) {
      auto edge = *dependencies_.get_in_edges(v).begin();
      dependencies_.remove_edge(edge, v);
      dependencies_.remove_edge(v, edge);
    }
  }
  std::stack<u32> stack_;
  observe_one(v, v, stack_);
  for (auto w : graph_.get_neighbors(v)) {
    cc[w] = true;
    observe_one(w, v, stack_);
  }
  propagate(stack_);
  insert({v, dependencies_.out_degree(v)}, HEAPTYPE::REMOVE);
}

void NuPDS::remove_from_solution(u32 v) {
  std::stack<u32> propagating;
  std::stack<u32> stack_;
  set<u32> enqueued;

  solution_.erase(v);
  tabu[v] = TABU;
  cc[v] = false;
  stack_.push(v);
  enqueued.insert(v);

  u32 score = 0;

  while (!stack_.empty()) {
    auto v = stack_.top();
    stack_.pop();
    std::optional<u32> observer;
    for (auto w : graph_.get_neighbors(v)) {
      cc[w] = true;
      unobserved_degree_[w] += 1;
      score += unobserved_degree_[w];

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

  insert({v, unobserved_degree_[v] + score}, HEAPTYPE::ADD);
}

u32 NuPDS::select_add_vertex() {
  std::vector<std::pair<u32, u32>> candidates;
  u32 ret = UINT32_MAX;
  while (add_heap_index > 0) {
    if (add_heap_index == 0)
      break;
    auto v = top(HEAPTYPE::ADD);
    pop(HEAPTYPE::ADD);
    if (cc[v.first] && tabu[v.first] == 0 && not_exculded(v.first) &&
        !is_in_solution(v.first)) {
      ret = v.first;
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
  u32 ret = INT32_MAX;
  while (remove_heap_index > 0) {
    if (remove_heap_index == 0)
      break;
    auto v = top(HEAPTYPE::REMOVE);
    pop(HEAPTYPE::REMOVE);
    if (cc[v.first] && tabu[v.first] == 0 && is_in_solution(v.first)) {
      ret = v.first;
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

  while (dependencies_.vertex_nums() != graph_.vertex_nums()) {
    // auto v = top(HEAPTYPE::ADD);
    // pop(HEAPTYPE::ADD);
    // add_into_solution(v.first);
    // age[v.first] = timestamp_;
    // tabu[v.first] = 0;
    // timestamp_++;
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
  }
}

void NuPDS::solve() {
  greedy();
  best_solution_ = solution_;
  while (timestamp_ < cutoff_ && solution_.size() > 1) {
    timestamp_++;
    if (dependencies_.vertex_nums() == graph_.vertex_nums()) {
      best_solution_ = solution_;
      auto v = select_remove_vertex();
      if (v != INT32_MAX) {
        remove_from_solution(v);
        age[v] = timestamp_;
      }
      continue;
    }

    auto v_remove = select_remove_vertex();
    if (v_remove == INT32_MAX) {
      continue;
    }
    auto v_add = select_add_vertex();

    remove_from_solution(v_remove);
    age[v_remove] = timestamp_;
    add_into_solution(v_add);
    age[v_add] = timestamp_;
  }
}

const set<u32> &NuPDS::get_solution() const { return solution_; }

const set<u32> &NuPDS::get_best_solution() const { return best_solution_; }
