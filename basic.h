#pragma once
#include <iostream>
#include <algorithm>
#include <queue>
#include <ctime>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <cmath>
#include <fstream>
#include <queue>
using namespace std;

typedef long long uint64;

// param for algorithm
int max_time;
int optimal;
int seeds;
double gama;
double roi = 0.3;


default_random_engine e; // random number gen

struct Edge {
	int v1, v2;
};


int v_num; // vec num
int e_num; // edge num
Edge* edge; // edge set

int* dscore; // dscore
uint64* age; // age of vec
int* weight; // edge weight
double weight_mean;
unordered_map<int, vector<int>> v_neig;
unordered_map<int, vector<int>> v_edges;

// 


// For compare dscore
struct cmp {
	bool operator() (const int &a, const int &b) const {
		bool j1 = dscore[a] == dscore[b];
		bool j2 = age[a] == age[b];
		if (j1&&!j2)
			return age[a] > age[b];
		else if (j1&&j2)
			return a > b;
		else 
			return dscore[a] > dscore[b];
	}
};
set<int, cmp> dscore_vec; // heap for high dscore of all vec

unordered_set<int> C_vec;
set<int, cmp> C_vec_dscore;


// sol struct
int* v_in_c; // vec in sol C
int c_size;
//unordered_set<int> uncover_e;
unordered_set<int> unobserved_v;
int step;

// For CC
int* conf_change;
int tabu;

// For dscore
int* only_domain;

// For prop
int* vis;
queue<int> openlist;



// best sol
int* best_v_in_c;
int best_c_size;
