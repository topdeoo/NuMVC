//
// Created by LENOVO on 2023/3/11.
//

#ifndef PDSP_BASIC_H
#define PDSP_BASIC_H


//! Data Structure

#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <vector>
#include <set>
#include <stack>
#include <cmath>
#include <fstream>
#include <iostream>
#include <random>
#include <map>

int n, m; //! n: number of nodes, m: number of edges

//! safe and fast hash function
//! reference: https://codeforces.com/blog/entry/62393

struct custom_hash {
    static uint64_t splitmix64 (uint64_t x) {
        x += 0x9e3779b97f4a7c15;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
        x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
        return x ^ (x >> 31);
    }

    size_t operator() (uint64_t x) const {
        static const uint64_t FIXED_RANDOM = std::chrono::steady_clock::now().time_since_epoch().count();
        return splitmix64(x + FIXED_RANDOM);
    }
};


//! bitmap for support set
struct bitmap {
    std::vector<size_t> _bit;

    bitmap () = default;

    explicit bitmap (size_t size) : _bit(size / 32 + 1) {}

    void set (const size_t &pos) {
        size_t index = pos / 32;
        size_t offset = pos % 32;
        _bit[index] |= (1 << offset);
    }

    bool find (const size_t &pos) {
        size_t index = pos / 32;
        size_t offset = pos % 32;
        return (_bit[index] >> offset) & 1;
    }

    void reset (const size_t &pos) {
        size_t index = pos / 32;
        size_t offset = pos % 32;
        _bit[index] &= ~(1 << offset);
    }

    //! judge the vertex u whether is only domain by vertex v
    bool is_only () {
        int cnt = 0;
        for (auto &i: _bit) {
            if (i != 0)
                cnt++;
        }
        return cnt == 1;
    }

};

std::unordered_set<int, custom_hash> C; //! candidate set
std::unordered_set<int, custom_hash> P; //! observed set
std::unordered_map<int, std::unordered_set<int, custom_hash>, custom_hash> S; //! support set

//! edges
std::unordered_map<int, std::vector<int>, custom_hash> edges;

//! neighbor list
std::unordered_map<int, std::vector<int>, custom_hash> neighbor;


//! score for vertex, sorted by score in increasing order
//! add_score is end()
//! remove_score is begin()
struct vertex {
    int score, age;
    vertex () = default;
    explicit vertex(int score, int age) : score(score), age(age) {}
};

//! score for adding vertex
std::map<int, vertex> add_score;

//! score for removing vertex
std::map<int, vertex> remove_score;

//! weight for each vertex
std::vector<int> weight;

//! weight mean
double mean;

//! update weight
double gama = 0.5;
double roi = 0.3;

//! CC strategy
std::vector<bool> cc;
int tabu;

//! best solution
std::unordered_set<int, custom_hash> C_star;

//! random seed
std::random_device rd;
std::knuth_b gen(rd());

//! standard answer
int standard_answer;

#endif //PDSP_BASIC_H
