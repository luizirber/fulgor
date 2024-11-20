#include "include/index.hpp"
#include "external/sshash/include/query/streaming_query_canonical_parsing.hpp"

namespace fulgor {

template <typename Iterator>
void counting_intersect(std::vector<Iterator>& iterators, std::vector<uint32_t>& colors,
                        uint32_t num_colors) {
    std::vector<uint16_t> counts(num_colors, 0);
    uint32_t num_iterators = iterators.size();
    for (auto it : iterators) {
        while (*it != num_colors) {
            counts[*it]++;
            ++it;
        }
    }
    for (uint32_t color = 0; color < num_colors; color++) {
        if (counts[color] == num_iterators) { colors.push_back(color); }
    }
}

}  // namespace fulgor
