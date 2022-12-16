#include <omp.h>

#include "../../include/epiworld/epiworld.hpp"

using namespace epiworld;

int main(int argc, char * argv[]) {

    auto nthreads = strtol(argv[1], nullptr, 0);
    auto nsims    = strtol(argv[2], nullptr, 0);

    omp_set_num_threads(nthreads);
    epimodels::ModelSIR<> model(
        "a virus", // Name of the virus
        0.01,      // Initial prevalence
        0.9,       // Infectiousness
        0.3        // Recovery rate
    );

    model.agents_from_adjlist(
        rgraph_smallworld(100000, 4, .01, false, model)
    );

    auto sav = make_save_run<int>(std::string("%03lu-episim.txt"));
    auto start = std::chrono::high_resolution_clock::now();
    model.init(100, 222);
    model.run_multiple(nsims, sav, true, true, nthreads);
    auto end = std::chrono::high_resolution_clock::now();

    
    printf(
        "Elapsed time: %ld milliseconds\n",
        (std::chrono::duration_cast<std::chrono::milliseconds>(end - start)).count()
    );

    model.print();

    return 0;
}
