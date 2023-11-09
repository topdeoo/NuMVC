#pragma once

#ifndef __GRAPH_HPP__
#define __GRAPH_HPP__
#include "basic.hpp"
#include <queue>
#include <utility>
#include <vector>

// NOTE: assume that the graph is undirected
class Graph {
public:
  Graph() = default;
  Graph(u32 n, u32 m) : n_(n), m_(m){};
  ~Graph() = default;
  Graph(const Graph &graph) {
    n_ = graph.n_;
    m_ = graph.m_;
    vertices_ = graph.vertices_;
    neighbors_ = graph.neighbors_;
  };
  Graph(Graph &&) = default;
  Graph &operator=(Graph &&) = default;

private:
  u32 n_, m_;         // n: number of vertices, m: number of edges
  set<u32> vertices_; // vertices_ indicates the set of vertices
  map<u32, set<u32>>
      neighbors_; // neighbors_[i] indicates the set of neighbors of node i

  map<u32, set<u32>> in_neighbors_;  // in_neighbors_[i] indicates the set of
                                     // in-neighbors of node i
  map<u32, set<u32>> out_neighbors_; // out_neighbors_[i] indicates the set of
                                     // out-neighbors of node i

public:
  // function of graph
  u32 vertex_nums() const;
  u32 edge_nums() const;

  // function of vertex
  const set<u32> &vertices() const;
  void add_vertex(u32 v);
  void remove_vertex(u32 v);
  bool has_vertex(u32 v) const;
  u32 degree(u32 v) const;
  u32 in_degree(u32 v);
  u32 out_degree(u32 v);

  // function of edge
  void add_edge(u32 u, u32 v);
  void remove_edge(u32 u, u32 v);
  bool is_neighbor(u32 u, u32 v) const;
  bool has_edge(u32 from, u32 to) const;
  const set<u32> &get_in_edges(u32 v) const;

  // function of neighbors
  const set<u32> &get_neighbors(u32 v);
  const set<u32> &get_in_neighbors(u32 v);
  const set<u32> &get_out_neighbors(u32 v);
  bool has_neighbors(u32 v) const;
};

#endif // __GRAPH_HPP__