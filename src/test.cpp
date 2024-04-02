#include <fstream>
#include <iostream>

#include "basic.hpp"
#include "graph.hpp"
#include "pds.hpp"

int main( int argc, const char* argv[] ) {
    if ( argc <= 1 ) {
        exit( 1 );
    }
    std::ifstream fin( argv[1] );

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

    pds.add_into_solution( 23 );
    for ( auto& v : pds.get_observed_vertex() ) {
        std::cout << v << ' ';
    }
    std::cout << std::endl;

    pds.add_into_solution( 6 );
    for ( auto& v : pds.get_observed_vertex() ) {
        std::cout << v << ' ';
    }
    std::cout << std::endl;

    pds.add_into_solution( 10 );
    for ( auto& v : pds.get_observed_vertex() ) {
        std::cout << v << ' ';
    }
    std::cout << std::endl;

    pds.add_into_solution( 30 );
    for ( auto& v : pds.get_observed_vertex() ) {
        std::cout << v << ' ';
    }
    std::cout << std::endl;
    return 0;
}
