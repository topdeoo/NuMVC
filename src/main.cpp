#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

#include "nupds.hpp"

auto now() { return std::chrono::high_resolution_clock::now(); }

int main( int argc, const char *argv[] ) {
    if ( argc <= 2 ) {
        exit( 1 );
    }

    std::ifstream fin( argv[1] );
    std::ofstream fout( argv[2] );

    NuPDS solver;

    std::string t;
    fin >> t;

    solver.init( fin );
    // pds.pre_process();

    auto t0 = now();

    solver.search();

    auto t1 = now();

    fout << t << std::endl;

    fout << std::chrono::duration_cast<std::chrono::microseconds>( t1 - t0 ).count() << "us"
         << std::endl;

    auto solution = solver.getSolution();

    // auto solution = pds.get_best_solution();
    fout << solution.size() << std::endl;
    for ( auto &v : solution ) {
        fout << v << " ";
    }
    fout << std::endl;

    return 0;
}
