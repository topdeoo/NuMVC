#pragma once

#ifndef PDS_HPP
#define PDS_HPP
#include <fstream>
#include <stack>

#include "basic.hpp"
#include "graph.hpp"

struct VertexState {
    bool used;
    bool update;
    bool excluded;
    bool pre_selected;
};

class NuPDS {
public:
    NuPDS() = default;
    ~NuPDS() = default;
    NuPDS( const NuPDS & ) = default;
    NuPDS( NuPDS && ) = default;
    NuPDS( const Graph &graph ) : graph_( graph ) {
        for ( auto &v : graph_.vertices() ) {
            unobserved_degree_[v] = graph_.degree( v );
            cc_[v] = true;
            tabu_[v] = 0;
        }
    };
    void init( std::ifstream &fin );
    void pre_process();

private:
    Graph graph_;
    Graph dependencies_;
    map<u32, u32> compress_;
    set<u32> solution_;
    map<u32, u32> unobserved_degree_;
    set<u32> non_observed_;
    map<u32, double> add_score_;
    map<u32, double> remove_score_;
    map<u32, bool> cc_;
    map<u32, u32> tabu_;
    u32 tabu_size_;
    set<u32> best_solution_;
    u32 timestamp_;
    u32 cutoff_;
    set<u32> pre_selected_;  // already in solution
    map<u32, VertexState> vertices_state_;
    set<u32> available_candidates_;
    set<u32> excluded_;         // will nerver in solution
    set<u32> non_propagating_;  // won't do propagation
    double alpha_;
    // double gamma_;

public:
    // main procedure
    void solve();
    void greedy();

    // dominating
    void observe_one( u32 vertex, u32 origin, std::stack<u32> &stack_ );
    void propagate( std::stack<u32> &stack_ );
    bool is_observed( u32 v );

    // update candidate set
    bool is_effected( u32, const set<u32> & );
    set<u32> get_neighbor( const set<u32> &, bool closed = false );
    void update_candidate_after_add( u32 vertex );
    void update_candidate_after_remove( u32 vertex );

    // select function
    std::pair<u32, double> select_add_vertex( bool first = false );
    u32 select_remove_vertex( bool random = false );

    // update solution
    void add_into_solution( u32 v );
    void remove_from_solution( u32 v );
    void redundant_removal();

    void update_pre_selected();

    // hlep func
    bool all_observed();
    bool is_in_solution( u32 v );
    bool not_exculded( u32 v );
    bool is_tabu( u32 v );
    inline bool can_propagate( u32 v ) { return non_propagating_.count( v ) == 0; }

public:
    const set<u32> &get_observed_vertex() const;
    const set<u32> &get_solution() const;
    const set<u32> &get_best_solution() const;
};

#endif  // PDS_HPP