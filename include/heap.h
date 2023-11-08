#pragma once

#ifndef __HEAP_H__
#define __HEAP_H__

#include "basic.hpp"
#include "graph.hpp"

enum class HEAPTYPE { ADD, REMOVE };

extern std::vector<std::pair<u32, u32>> add_score;
extern u32 add_heap_index;
extern std::vector<std::pair<u32, u32>> removing_score;
extern u32 remove_heap_index;
extern map<u32, u32> age;

u32 parient(u32 index);
u32 left_child(u32 index);
u32 right_child(u32 index);

void insert(const std::pair<u32, u32> &entry, HEAPTYPE which);
std::pair<u32, u32> &top(HEAPTYPE which);
void pop(HEAPTYPE which);
void init_heap(u32 n, Graph &graph);

#endif // __HEAP_H__