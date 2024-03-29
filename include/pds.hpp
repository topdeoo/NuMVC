#pragma once

#ifndef PDS_HPP
#define PDS_HPP
#include <fstream>
#include <stack>

#include "basic.hpp"
#include "graph.hpp"
#include "heap.h"

class NuPDS {
public:
    NuPDS() = default;
    ~NuPDS() = default;
    NuPDS( const NuPDS & ) = default;
    NuPDS( NuPDS && ) = default;
    NuPDS( const Graph &graph ) : graph_( graph ) {
        for ( auto &v : graph_.vertices() ) {
            unobserved_degree_[v] = graph_.degree( v );
            base_score_[v] = graph_.degree( v );
            age[v] = 0;
            cc_[v] = true;
            tabu_[v] = 0;
        }
        init_heap( graph.vertices().size(), graph_ );
    };
    void init( std::ifstream &fin );
    void pre_process();

private:
    Graph graph_;
    Graph dependencies_;
    set<u32> solution_;
    map<u32, u32> unobserved_degree_;
    map<u32, u32> base_score_;
    map<u32, u32> remove_score_;
    map<u32, bool> cc_;
    map<u32, u32> tabu_;
    u32 tabu_size_;
    set<u32> best_solution_;
    u32 timestamp_;
    u32 cutoff_;
    set<u32> pre_selected_;  // already in solution
    set<u32> candidate_solution_set_;
    set<u32> excluded_;         // will nerver in solution
    set<u32> non_propagating_;  // won't do propagation
    double alpha_;
    double gamma_;

public:
    void solve();
    void greedy();
    void observe( u32 vertex, u32 origin );
    void observe_one( u32 vertex, u32 origin, std::stack<u32> &stack_ );
    void propagate( std::stack<u32> &stack_ );
    bool is_observed( u32 v );
    bool is_in_solution( u32 v );
    bool not_exculded( u32 v );
    bool is_tabu( u32 v );
    void add_into_solution( u32 v );
    void remove_from_solution( u32 v );
    u32 select_add_vertex();
    u32 select_remove_vertex();
    bool all_observed();
    u32 Ob( u32 v );
    void tabu_forget();
    void update_score();
    void forget_score();
    void redundant_removal();
    inline bool can_propagate( u32 v ) { return non_propagating_.count( v ) == 0; }

    void update_pre_selected();

public:
    const set<u32> &get_observed_vertex() const;
    const set<u32> &get_solution() const;
    const set<u32> &get_best_solution() const;
};

#endif  // PDS_HPP