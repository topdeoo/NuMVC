#include <fstream>
#include <iostream>
#include <stack>

#include "basic.hpp"
#include "graph.hpp"
#include "pds.hpp"

int main( int argc, const char *argv[] ) {
    if ( argc <= 2 ) {
        exit( 1 );
    }
    std::ifstream fin( argv[1] );
    std::ifstream sol( argv[2] );

    std::string t;

    Graph g;
    u32 n, m;
    fin >> n >> m;
    for ( u32 i = 0; i < m; i++ ) {
        u32 u, v;
        fin >> u >> v;
        g.add_edge( u, v );
        g.add_edge( v, u );
    }

    NuPDS pds( g );

    sol >> t;

    u32 k;
    sol >> k;
    for ( u32 i = 0; i < k; i++ ) {
        u32 v;
        sol >> v;
        pds.add_into_solution( v );
    }

    std::cout << argv[2] << " ";

    if ( pds.all_observed() ) {
        std::cout << "OK" << std::endl;
    } else {
        std::cout << "WA" << std::endl;
        for ( auto &v : pds.get_solution() ) {
            std::cout << v << ' ';
        }
        std::cout << std::endl;
        auto observed = pds.get_observed_vertex();
        for ( auto &v : observed ) {
            std::cout << v << ' ';
        }
    }

    return 0;
}
