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

template <typename ColorSets>
void index<ColorSets>::pseudoalign_full_intersection_timed(std::string const& sequence,
                                                     std::vector<uint32_t>& colors, 
                                                     std::vector<essentials::timer<std::chrono::high_resolution_clock, std::chrono::nanoseconds>>& timers) const {
    if (sequence.length() < m_k2u.k()) return;
    colors.clear();
    std::vector<uint64_t> unitig_ids;

    timers[0].start();
    stream_through(m_k2u, sequence, unitig_ids);
    timers[0].stop();

    intersect_unitigs_timed(unitig_ids, colors, timers);
}

template <typename ColorSets>
void index<ColorSets>::intersect_unitigs_timed(std::vector<uint64_t>& unitig_ids,
                           std::vector<uint32_t>& colors,
                           std::vector<essentials::timer<std::chrono::high_resolution_clock, std::chrono::nanoseconds>>& timers) const {
    /* here we use it to hold the color class ids;
       in meta_intersect we use it to hold the partition ids */
    std::vector<uint32_t> tmp;
    std::vector<typename ColorSets::iterator_type> iterators;

    /* deduplicate unitig_ids */
    timers[1].start();
    std::sort(unitig_ids.begin(), unitig_ids.end());
    auto end_unitigs = std::unique(unitig_ids.begin(), unitig_ids.end());
    tmp.reserve(end_unitigs - unitig_ids.begin());
    for (auto it = unitig_ids.begin(); it != end_unitigs; ++it) {
        uint32_t unitig_id = *it;
        uint32_t color_set_id = u2c(unitig_id);
        tmp.push_back(color_set_id);
    }

    /* deduplicate color class ids */
    std::sort(tmp.begin(), tmp.end());
    auto end_tmp = std::unique(tmp.begin(), tmp.end());
    iterators.reserve(end_tmp - tmp.begin());
    for (auto it = tmp.begin(); it != end_tmp; ++it) {
        uint64_t color_set_id = *it;
        auto fwd_it = m_color_sets.color_set(color_set_id);
        iterators.push_back(fwd_it);
    }
    timers[1].stop();
    timers[2].start();

    tmp.clear();  // don't need color class ids anymore
    if constexpr (ColorSets::meta_colored) {
        meta_intersect<typename ColorSets::iterator_type, ColorSets::differential_colored>(iterators, colors, tmp);
    } else if constexpr (ColorSets::differential_colored) {
        diff_intersect(iterators, colors);
    } else {
        intersect(iterators, colors, tmp);
    }
    timers[2].stop();

    assert(util::check_intersection(iterators, colors));
}

}  // namespace fulgor
