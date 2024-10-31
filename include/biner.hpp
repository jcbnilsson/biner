/* biner - Combine and separate text files
 *
 * Copyright(c) 2022-2024 Jacob Nilsson
 * Licensed under the GNU General Public License version 3.0
 */

#pragma once

#include <string>
#include <vector>

namespace biner {
    enum class Mode {
        Combine,
        Separate,
        Undefined,
    };

    struct Settings {
        bool verbose{false};
        std::string directory{"./"};
        std::string begin_marker{"--!- BINER FILE BEGIN -!--"};
        std::string end_marker{"--!- BINER FILE END -!--"};
    };

    void print_help(bool error);

    template <typename T> std::string combine_files(const struct Settings& settings, const std::vector<T>& files);
    template <typename T> void separate_files(const struct Settings& settings, const std::vector<T>& files);
}
