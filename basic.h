//! @file basic data structure
//! @author Virgil

#ifndef PDSP_BASIC_H
#define PDSP_BASIC_H

// Data Structure

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <random>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

//! @var n: number of nodes
//! @var m: number of edges
int n, m;

//! @struct custom_hash, a safe and fast hash function
//! @link https://codeforces.com/blog/entry/62393

struct custom_hash {
  //! @fn splitmix64
  //! @brief hash function
  static uint64_t splitmix64(uint64_t x) {
    x += 0x9e3779b97f4a7c15;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
    x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
    return x ^ (x >> 31);
  }

  //! @brief get hash value
  size_t operator()(uint64_t x) const {
    static const uint64_t FIXED_RANDOM = std::chrono::steady_clock::now().time_since_epoch().count();
    return splitmix64(x + FIXED_RANDOM);
  }
};

//! @brief bitmap for support set
struct bitmap {
  //! @var _bit: bitmap
  std::vector<size_t> _bit;

  bitmap() = default;

  explicit bitmap(size_t size) : _bit(size / 32 + 1) {}

  //! @fn set
  //! @brief set the bit to 1 at pos
  void set(const size_t &pos) {
    size_t index = pos / 32;
    size_t offset = pos % 32;
    _bit[index] |= (1 << offset);
  }

  //! @fn find
  //! @brief find the bit whether is 1 at pos
  bool find(const size_t &pos) {
    size_t index = pos / 32;
    size_t offset = pos % 32;
    return (_bit[index] >> offset) & 1;
  }

  //! @fn reset
  //! @brief reset the bit to 0 at pos
  void reset(const size_t &pos) {
    size_t index = pos / 32;
    size_t offset = pos % 32;
    _bit[index] &= ~(1 << offset);
  }

  // NOTE low efficiency
  //! @fn is_only
  //! @brief judge the vertex u whether is only domain by vertex v
  bool is_only() {
    int cnt = 0;
    for (auto &i : _bit) {
      if (i != 0) cnt++;
    }
    return cnt == 1;
  }
};

std::unordered_set<int, custom_hash> C;                                        //! candidate set
std::unordered_set<int, custom_hash> P;                                        //! observed set
std::unordered_map<int, std::unordered_set<int, custom_hash>, custom_hash> S;  //! support set
std::unordered_map<int, int> Link;  //! record the vertex is supported by which vertex

//! edges
std::unordered_map<int, std::vector<int>, custom_hash> edges;

//! neighbor list
std::unordered_map<int, std::vector<int>, custom_hash> neighbor;

//! @struct vertex
//! @details
//! score for vertex, sorted by score in increasing order
//! add_score is end()
//! remove_score is begin()
struct vertex {
  int score, age;
  vertex() = default;
  explicit vertex(int score, int age) : score(score), age(age) {}
};

// score for adding vertex
std::map<int, vertex> add_score;

// score for removing vertex
std::map<int, vertex> remove_score;

// weight for each vertex
std::vector<int> weight;

// weight mean
double mean;

// update weight
double gama = 0.5;
double roi = 0.3;

// CC strategy
std::vector<bool> cc;
// tabu strategy 未来 n 次内不能被选中删除
std::vector<short> tabu;

// best solution
std::unordered_set<int, custom_hash> C_star;

// random seed
std::random_device rd;
std::knuth_b gen(rd());

// standard answer
int standard_answer;

#endif  // PDSP_BASIC_H
