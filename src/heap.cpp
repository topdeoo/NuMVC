#include "heap.h"
#include "basic.hpp"
#include "graph.hpp"
#include <utility>
#include <vector>

std::vector<std::pair<u32, u32>> add_score;
u32 add_heap_index;
std::vector<std::pair<u32, u32>> removing_score;
u32 remove_heap_index;
map<u32, u32> age;

u32 parient(u32 index) { return index / 2; }
u32 left_child(u32 index) { return 2 * index; }
u32 right_child(u32 index) { return 2 * index + 1; }

static void up(u32 index, HEAPTYPE which) {
  if (which == HEAPTYPE::ADD) {
    while (parient(index) > 0 &&
           (add_score[parient(index)].second <= add_score[index].second)) {
      auto u = add_score[parient(index)], v = add_score[index];
      if (u.second > v.second) {
        goto add_swap;
      } else if (u.second == v.second && age[u.first] < age[v.first]) {
        goto add_swap;
      }
    add_swap:
      std::swap(add_score[parient(index)], add_score[index]);
      index = parient(index);
    }
  } else {
    while (parient(index) > 0 && removing_score[parient(index)].second >=
                                     removing_score[index].second) {
      auto u = removing_score[parient(index)], v = removing_score[index];
      if (u.second > v.second) {
        goto remove_swap;
      } else if (u.second == v.second && age[u.first] < age[v.first]) {
        goto remove_swap;
      }
    remove_swap:
      std::swap(removing_score[parient(index)], removing_score[index]);
      index = parient(index);
    }
  }
}

static void down(u32 index, HEAPTYPE which) {
  if (which == HEAPTYPE::ADD) {
    auto temp = index;
    if (left_child(index) <= add_heap_index &&
        add_score[temp].second < add_score[left_child(index)].second)
      temp = left_child(index);
    if (right_child(index) <= add_heap_index &&
        add_score[temp].second < add_score[right_child(index)].second)
      temp = right_child(index);
    if (temp != index) {
      std::swap(add_score[temp], add_score[index]);
      down(temp, which);
    }
  } else {
    auto temp = index;
    if (left_child(index) <= remove_heap_index &&
        removing_score[temp].second > removing_score[left_child(index)].second)
      temp = left_child(index);
    if (right_child(index) <= add_heap_index &&
        removing_score[temp].second > removing_score[right_child(index)].second)
      temp = right_child(index);
    if (temp != index) {
      std::swap(removing_score[temp], removing_score[index]);
      down(temp, which);
    }
  }
}

void insert(const std::pair<u32, u32> &entry, HEAPTYPE which) {
  if (which == HEAPTYPE::ADD) {
    add_score[++add_heap_index] = entry;
    up(add_heap_index, which);

  } else {
    removing_score[++remove_heap_index] = entry;
    up(remove_heap_index, HEAPTYPE::REMOVE);
  }
}

std::pair<u32, u32> &top(HEAPTYPE which) {
  if (which == HEAPTYPE::ADD) {
    return add_score[1];
  } else {
    return removing_score[1];
  }
}

void pop(HEAPTYPE which) {
  if (which == HEAPTYPE::ADD) {
    std::swap(add_score[1], add_score[add_heap_index]);
    add_heap_index--;
  } else {
    std::swap(removing_score[1], removing_score[remove_heap_index]);
    remove_heap_index--;
  }
  down(1, which);
}

void init_heap(u32 n, Graph &graph) {

  add_heap_index = remove_heap_index = 0;
  add_score.resize(n + 1);
  removing_score.resize(n + 1);

  auto vertices = graph.vertices();
  for (auto &v : vertices) {
    u32 score = 0;
    for (auto &w : graph.get_neighbors(v)) {
      score += graph.degree(w);
    }
    score += graph.degree(v);
    insert({v, score}, HEAPTYPE::ADD);
  }
}
