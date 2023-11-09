#include "graph.hpp"

u32 Graph::vertex_nums() const { return vertices_.size(); }

u32 Graph::edge_nums() const { return m_; }

const set<u32> &Graph::vertices() const { return vertices_; }

void Graph::add_vertex(u32 v) {
  if (!has_vertex(v)) {
    vertices_.insert(v);
    neighbors_[v] = set<u32>();
    in_neighbors_[v] = set<u32>();
    n_++;
  }
}

bool Graph::has_vertex(u32 v) const {
  return vertices_.find(v) != vertices_.end();
}

void Graph::remove_vertex(u32 v) {
  if (has_vertex(v)) {
    vertices_.erase(v);
    for (auto &w : in_neighbors_[v]) {
      neighbors_[w].erase(v);
    }
    in_neighbors_.erase(v);
    neighbors_.erase(v);
  }
}

u32 Graph::degree(u32 v) const {
  if (has_vertex(v)) {
    return neighbors_.at(v).size();
  } else {
    return 0;
  }
}

u32 Graph::in_degree(u32 v) {
  if (in_neighbors_.find(v) == in_neighbors_.end()) {
    in_neighbors_[v] = set<u32>();
    return 0;
  }
  return in_neighbors_.at(v).size();
}

u32 Graph::out_degree(u32 v) {
  if (out_neighbors_.find(v) == out_neighbors_.end()) {
    out_neighbors_[v] = set<u32>();
    return 0;
  }
  return out_neighbors_.at(v).size();
}

void Graph::add_edge(u32 u, u32 v) {
  if (!has_vertex(u) || !has_vertex(v)) {
    add_vertex(v);
    add_vertex(u);
  }
  neighbors_[u].insert(v);
  in_neighbors_[v].insert(u);
  out_neighbors_[u].insert(v);
  m_++;
}

void Graph::remove_edge(u32 u, u32 v) {
  if (has_vertex(u) && has_vertex(v)) {
    neighbors_[u].erase(v);
    in_neighbors_[v].erase(u);
    out_neighbors_[u].erase(v);
    m_--;
  }
}

bool Graph::is_neighbor(u32 u, u32 v) const {
  if (has_vertex(u) && has_vertex(v)) {
    return neighbors_.at(u).find(v) != neighbors_.at(u).end();
  } else {
    return false;
  }
}

bool Graph::has_edge(u32 from, u32 to) const {
  if (has_vertex(from) && has_vertex(to)) {
    return neighbors_.at(from).find(to) != neighbors_.at(from).end();
  } else {
    return false;
  }
}

bool Graph::has_neighbors(u32 v) const {
  return neighbors_.find(v) != neighbors_.end();
}

const set<u32> &Graph::get_in_edges(u32 v) const { return in_neighbors_.at(v); }

const set<u32> &Graph::get_neighbors(u32 v) {
  if (neighbors_.find(v) == neighbors_.end()) {
    neighbors_[v] = set<u32>();
  }
  return neighbors_.at(v);
}

const set<u32> &Graph::get_in_neighbors(u32 v) {
  if (in_neighbors_.find(v) == in_neighbors_.end()) {
    in_neighbors_[v] = set<u32>();
  }
  return in_neighbors_.at(v);
}

const set<u32> &Graph::get_out_neighbors(u32 v) {
  if (out_neighbors_.find(v) == out_neighbors_.end()) {
    out_neighbors_[v] = set<u32>();
  }
  return out_neighbors_.at(v);
}
