#include "include/index.hpp"
#include "external/sshash/include/query/streaming_query_canonical_parsing.hpp"

#include "tests/intersect_color_sets.cpp"
#include "tests/psa_time_breakdown.cpp"

namespace fulgor {

int test(int argc, char** argv) {
    auto tool = std::string(argv[1]);

    if (tool == "intersect-color-sets") {
        return intersect_color_sets(argc - 1, argv + 1);
    } else if (tool == "psa-time-breakdown") {
        return psa_time_breakdown(argc - 1, argv + 1);
    }

    return 0;
}

}  // namespace fulgor