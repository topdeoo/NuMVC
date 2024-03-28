#include "fss.hpp"
#include "basic.hpp"
#include <optional>

// extern auto now() { return std::chrono::high_resolution_clock::now(); }

// static std::random_device rd{"hw"};
// static double alpha_() {
//   // NOTE alpha \sim U(1, R), the "R" here is 2
//   static auto gen = std::bind(std::uniform_real_distribution<>(1, 2),
//                               std::default_random_engine(rd()));
//   return gen();
// }

// static u32 index(u32 size) {
//   static auto gen = std::bind(std::uniform_int_distribution<>(0, size - 1),
//                               std::default_random_engine(rd()));
//   return gen();
// }

void FSS::observe(u32 vertex, u32 origin) {
  std::stack<u32> stack_;
  observe_one(vertex, origin, stack_);
  propagate(stack_);
}

void FSS::observe_one(u32 vertex, u32 origin, std::stack<u32> &stack_) {
  if (!is_observed(vertex)) {
    dependencies_.add_vertex(vertex);
    if (origin != vertex)
      dependencies_.add_edge(origin, vertex);
    if (unobserved_degree_[vertex] == 1)
      stack_.push(vertex);
    for (auto w : graph_.get_out_neighbors(vertex)) {
      unobserved_degree_[w] -= 1;
      if (unobserved_degree_[w] == 1 && is_observed(w))
        stack_.push(w);
    }
  }
}

void FSS::propagate(std::stack<u32> &stack_) {
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

void FSS::add_into_solution(u32 vertex) {

  if (dependencies_.has_vertex(vertex)) {
    auto neighbors = dependencies_.get_in_neighbors(vertex);
    for (auto &u : neighbors) {
      dependencies_.remove_edge(u, vertex);
    }
  }
  std::stack<u32> stack_;
  observe_one(vertex, vertex, stack_);
  for (auto w : graph_.get_neighbors(vertex)) {
    observe_one(w, vertex, stack_);
  }
  propagate(stack_);
}

void FSS::remove_from_solution(u32 v) {
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

  propagate(propagating);
}

inline bool FSS::all_observed() {
  return dependencies_.vertices().size() == graph_.vertices().size();
}

inline bool FSS::is_observed(u32 v) { return dependencies_.has_vertex(v); }

inline bool FSS::is_in_solution(u32 v) { return solution_.count(v) > 0; }
