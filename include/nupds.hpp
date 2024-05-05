#pragma once

#include "basic.hpp"
#ifndef NUPDS_HPP
#define NUPDS_HPP

#include <utility>

#include "pdsgraph.hpp"
#include "utility.hpp"

class NuPDS {
public:
    PDSGraph pds_graph_;
    mpgraphs::set<PDSGraph::Vertex> add_available_vertices_;
    mpgraphs::map<PDSGraph::Vertex, double> remove_available_vertices_;
    std::vector<PDSGraph::Vertex> best_solution_;

public:
    NuPDS() = default;

    NuPDS( const NuPDS& ) = default;
    NuPDS( NuPDS&& ) = default;
    NuPDS& operator=( NuPDS&& ) = default;
    NuPDS& operator=( const NuPDS& ) = default;

private:
    std::pair<PDSGraph::Vertex, double> selectVertexToAdd( bool );
    std::pair<PDSGraph::Vertex, double> selectVertexToRemove();
    std::pair<PDSGraph::Vertex, double> getMaxObserved();
    std::vector<PDSGraph::Vertex> testAddVertex( PDSGraph, PDSGraph::Vertex );
    void updateAfterDominating( PDSGraph::Vertex vertex, double score,
                                mpgraphs::set<PDSGraph::Vertex>& newly_observed );
    void getClouser( PDSGraph::Vertex vertex, mpgraphs::set<PDSGraph::Vertex>& clouser,
                     mpgraphs::set<PDSGraph::Vertex>& newly_observed );

    void updateAfterRemoving( PDSGraph::Vertex vertex );

public:
    void init( std::ifstream& );
    void GRASP();
    void localSearch();
    void search();
    std::vector<unsigned long> getSolution();

    // Debug
public:
    std::vector<PDSGraph::Vertex> setDominating( PDSGraph::Vertex vertex ) {
        auto newly_observed = pds_graph_.setDominating( vertex );
        updateAfterDominating( vertex, newly_observed.size() * ( 1 + random_alpha() ), newly_observed );
        return newly_observed | ranges::to<std::vector<PDSGraph::Vertex>>();
    };
};

#endif  // NUPDS_HPP