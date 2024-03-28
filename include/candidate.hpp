#pragma once

#ifndef __CANDIDATE_HPP__
#define __CANDIDATE_HPP__

#include "basic.hpp"
#include <vector>

class CandidateNode {
  bool updated_;
  bool used_;
  bool exist_;
  u32 vertex_;
  u32 vertice_count;
  std::vector<bool> neighbor_;

public:
  CandidateNode() { exist_ = false; }
  CandidateNode(u32 vertex) {
    vertex_ = vertex;
    updated_ = false;
    used_ = false;
    exist_ = true;
  }
  ~CandidateNode() = default;
  CandidateNode(const CandidateNode &node) {
    vertex_ = node.vertex_;
    updated_ = node.updated_;
    used_ = node.used_;
    exist_ = true;
    vertice_count = node.vertice_count;
    neighbor_.resize(vertice_count + 1);
  }

  u32 vertex() const { return vertex_; }
  bool updated() const { return updated_; }
  bool used() const { return used_; }
  bool exist() const { return exist_; }

  void set_update(bool value) { updated_ = value; }
  void set_used(bool value) { used_ = value; }
  void set_exist(bool value) { exist_ = value; }

  bool is_effected(const set<u32> &effected) {
    for (auto &v : effected) {
      if (neighbor_[v])
        return true;
    }
    return false;
  }
};

class CandidateSet {
  std::vector<CandidateNode> candidates_;
  set<u32> available_candidates;

public:
  ~CandidateSet() = default;
  CandidateSet(const CandidateSet &set) {
    candidates_ = set.candidates_;
    available_candidates = set.available_candidates;
  }

  set<u32> get_effected(const set<u32> &changed) {
    set<u32> effected;
    for (auto &v : available_candidates) {
      if (candidates_[v].exist()) {
        if (!candidates_[v].used() && !candidates_[v].updated()) {
          if (candidates_[v].is_effected(changed)) {
            effected.insert(v);
          }
        }
      }
    }
    return effected;
  }

  void set_update(u32 vertex, bool value) {
    candidates_[vertex].set_update(value);
  }

  void remove_candidate(u32 vertex) {
    candidates_[vertex].set_used(true);
    candidates_[vertex].set_exist(false);
    available_candidates.erase(vertex);
  }

  void set_used(u32 vertex) { candidates_[vertex].set_used(true); }
};

#endif //__CANDIDATE_HPP__
