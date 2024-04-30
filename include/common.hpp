//
// Created by max on 14.10.22.
//
#ifndef MPGRAPHS_COMMON_HPP
#define MPGRAPHS_COMMON_HPP

#include "utility.hpp"

namespace mpgraphs {
struct Empty {};

/**
 * Used to specify the graph type.
 */
enum class EdgeDirection {
    /// Directed graph.
    Directed,
    /// Directed graph aware of in-edges.
    Bidirectional,
    /// Undirected graph.
    Undirected
};
} // namespace settgraph

#endif //MPGRAPHS_COMMON_HPP