#include "fss.hpp"
#include "basic.hpp"
#include <chrono>
#include <fstream>
#include <functional>
#include <optional>
#include <random>
#include <utility>
#include <vector>

extern auto now() { return std::chrono::high_resolution_clock::now(); }

static std::random_device rd{"hw"};
static double alpha_() {
  // NOTE alpha \sim U(1, R), the "R" here is 2
  static auto gen = std::bind(std::uniform_real_distribution<>(1, 2),
                              std::default_random_engine(rd()));
  return gen();
}

static u32 index(u32 size) {
  static auto gen = std::bind(std::uniform_int_distribution<>(0, size - 1),
                              std::default_random_engine(rd()));
  return gen();
}

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
  solution_.insert(vertex);

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

bool FSS::all_observed() {
  return dependencies_.vertices().size() == graph_.vertices().size();
}

inline bool FSS::is_observed(u32 v) { return dependencies_.has_vertex(v); }

inline bool FSS::is_in_solution(u32 v) { return solution_.count(v) > 0; }

void FSS::greedy() {
  while (!all_observed()) {
    std::vector<std::pair<u32, double>> candidate;
    for (auto &v : graph_.vertices()) {
      if (!is_in_solution(v)) {
        auto prev_dependencies = dependencies_;
        add_into_solution(v);
        auto score = dependencies_.vertices().size() -
                     prev_dependencies.vertices().size();
        candidate.push_back({v, score * alpha_()});
        dependencies_ = prev_dependencies;
        solution_.erase(v);
      }
    }
    sort(candidate.begin(), candidate.end(),
         [](const std::pair<u32, u32> &a, const std::pair<u32, u32> &b) {
           return a.second > b.second;
         });
    if (!candidate.empty()) {
      add_into_solution(candidate[0].first);
    } else {
      break;
    }
  }
}

void FSS::local_search() {
  auto candidate = solution_;
  for (auto &v : candidate) {
    auto prev = dependencies_;
    remove_from_solution(v);
    if (!all_observed()) {
      dependencies_ = prev;
      solution_.insert(v);
    }
  }
}

void FSS::next_size() {
  double size = size_ + factor_ / 2;
  size_ = size;
  factor_ /= 2;
}

void FSS::fix_search() {
  solution_.clear();
  auto total_size = (u32)(size_ * b_.size());
  map<u32, u32> score;
  for (auto &s_i : skn_) {
    for (auto &v : s_i) {
      if (b_.count(v) > 0) {
        score[v] += 1;
      }
    }
  }
  std::vector<std::pair<u32, u32>> candidate(score.begin(), score.end());
  sort(candidate.begin(), candidate.end(),
       [](const std::pair<u32, u32> &a, const std::pair<u32, u32> &b) {
         return a.second > b.second;
       });
  for (auto i = 0; i < total_size; i++) {
    add_into_solution(candidate[i].first);
  }
}

void FSS::grasp() {
  greedy();
  if (!all_observed()) {
    return;
  }
  local_search();
  if (solution_.size() < best_solution_.size() || best_solution_.empty()) {
    best_solution_ = solution_;
  }
}

void FSS::clear() {
  solution_.clear();
  dependencies_.clear();
  unobserved_degree_.clear();
  for (auto &v : graph_.vertices()) {
    unobserved_degree_[v] = graph_.degree(v);
  }
}

void FSS::pre_processing() {
  // generate `population_size_ = max(n_, m_)` best solutions
  for (auto i = 0; i < N_; i++) {
    if (i > population_size_) {
      break;
    }
    clear();
    grasp();
    if (best_solution_.size() > worst_solution_.second ||
        worst_solution_.second == 0) {
      worst_solution_.second = best_solution_.size();
      worst_solution_.first = i;
    }
    population_.push_back(best_solution_);
  }
}

void FSS::search() {

  auto start = now();

  next_size();

  u32 not_improve = 0, prev_size = best_solution_.size();

  while (prev_size > 1) {
    auto _now = now();
    if (std::chrono::duration_cast<std::chrono::seconds>(_now - start).count() >
        1800) {
      beyond_time = true;
      return;
    }
    // set `S_kn` to be the best `k` solutions in `P_n`
    skn_.clear();
    set<u32> skn;
    while (skn.size() < k_) {
      skn.insert(index(population_size_));
    }
    for (auto i : skn) {
      skn_.push_back(population_[i]);
    }

    b_ = population_[index(population_size_)];

    fix_search();

    grasp();

    // Union into `P`
    if (solution_.size() < worst_solution_.second) {
      population_[worst_solution_.first] = solution_;
      for (auto i = 0; i < population_size_; i++) {
        if (population_[i].size() > worst_solution_.second) {
          worst_solution_.second = population_[i].size();
          worst_solution_.first = i;
        }
      }
    }

    if (best_solution_.size() == prev_size) {
      not_improve += 1;
    } else {
      not_improve = 0;
      prev_size = best_solution_.size();
    }

    if (not_improve > stagnation_) {
      next_size();
      not_improve = 0;
    }
  }
}

void FSS::init(std::ifstream &fin) {
  cutoff_ = 1000;
  stagnation_ = 100;

  beyond_time = false;

  u32 n, m;
  fin >> n >> m;

  for (u32 i = 0; i < m; i++) {
    u32 from, to;
    fin >> from >> to;
    graph_.add_edge(from, to);
    graph_.add_edge(to, from);
  }

  for (auto &v : graph_.vertices()) {
    unobserved_degree_[v] = graph_.degree(v);
  }
}

set<u32> FSS::get_best_solution() { return best_solution_; }
