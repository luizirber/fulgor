#include <iostream>
#include <fstream>
#include <sstream>

#include "external/CLI11.hpp"
#include "external/sshash/include/gz/zip_stream.hpp"

#include "tests/utils.cpp"

using namespace fulgor;

template <typename FulgorIndex>
int intersect_color_sets(std::string const& index_filename, uint64_t size, std::string const& algo,
                         const bool quiet) {
    FulgorIndex index;
    if (!quiet) essentials::logger("loading index from disk...");
    essentials::load(index, index_filename.c_str());
    if (!quiet) essentials::logger("DONE");

    // pseudoalignment_algorithm algo = pseudoalignment_algorithm::FULL_INTERSECTION;

    essentials::timer<std::chrono::high_resolution_clock, std::chrono::milliseconds> t;
    t.start();

    uint32_t num_intersections = 1000000;

    srand(42);
    uint32_t n = size;

    for (uint32_t i = 0; i < num_intersections; i++) {
        vector<typename FulgorIndex::color_sets_type::iterator_type> iterators;
        for (uint32_t j = 0; j < n; j++) {
            iterators.push_back(index.color_set(rand() % index.num_color_sets()));
        }
        vector<uint32_t> colors;
        if (algo == "geq") {
            next_geq_intersect(iterators, colors, index.num_colors());
        } else {
            counting_intersect(iterators, colors, index.num_colors());
        }
    }

    t.stop();
    if (!quiet) essentials::logger("DONE");

    if (!quiet) {
        std::cout << "intersected " << num_intersections << " reads" << std::endl;
        std::cout << "elapsed = " << t.elapsed() << " millisec / ";
        std::cout << t.elapsed() / 1000 << " sec / ";
        std::cout << t.elapsed() / 1000 / 60 << " min / ";
        std::cout << (t.elapsed() * 1000) / num_intersections << " musec/intersection" << std::endl;
    }

    return 0;
}

int intersect_color_sets(int argc, char** argv) {
    /* params */
    std::string index_filename;
    std::string algo;
    uint64_t size = 2;
    bool quiet = false;

    CLI::App app{"Perform (color-only) pseudoalignment to a Fulgor index."};
    app.add_option("-i,--index", index_filename, "The Fulgor index filename,")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("-s,--size", size, "Number of color sets to intersect.")->required();
    app.add_option("-a,--algorithm", algo, "Intersection algorithm ['geq' or 'count'].")
        ->required();
    app.add_flag("--quiet", quiet, "Quiet mode: do not print status messages to stdout.");
    CLI11_PARSE(app, argc, argv);

    if (!quiet) util::print_cmd(argc, argv);

    if (algo != "geq" && algo != "count") {
        std::cerr << "Wrong intersection algorithm" << std::endl;
        return 1;
    }

    if (sshash::util::ends_with(index_filename,
                                constants::meta_diff_colored_fulgor_filename_extension)) {
        return intersect_color_sets<meta_differential_index_type>(index_filename, size, algo,
                                                                  quiet);
    } else if (sshash::util::ends_with(index_filename,
                                       constants::meta_colored_fulgor_filename_extension)) {
        return intersect_color_sets<meta_index_type>(index_filename, size, algo, quiet);
    } else if (sshash::util::ends_with(index_filename,
                                       constants::diff_colored_fulgor_filename_extension)) {
        return intersect_color_sets<differential_index_type>(index_filename, size, algo, quiet);
    } else if (sshash::util::ends_with(index_filename, constants::fulgor_filename_extension)) {
        return intersect_color_sets<index_type>(index_filename, size, algo, quiet);
    }

    std::cerr << "Wrong filename supplied." << std::endl;

    return 1;
}
