/* biner - Combine and separate text files
 *
 * Copyright(c) 2022-2024 Jacob Nilsson
 * Licensed under the GNU General Public License version 3.0
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <exception>
#include <string>
#include <string_view>
#include <vector>
#include <filesystem>
#include <biner.hpp>

void biner::print_help(const bool Error) {
    constexpr std::string_view help{
        "usage: biner [-c] [-s] [-d directory] [-v] [-bm text] [-em text] [-o output] files\n"
    };
    if (Error) {
        std::cerr << help;
    } else {
        std::cout << help;
    }
}

template <typename T>
std::string biner::combine_files(const struct biner::Settings& settings, const std::vector<T>& files) {
    std::string combined_data{};

    for (const auto& it : files) {
        if (!std::filesystem::exists(it)) {
            throw std::runtime_error{"File passed to biner::combineFiles() does not exist."};
        }

        if (settings.verbose) {
            std::cerr << "Adding file '" << it << "' to buffer.\n";
        }

        const std::string itstr{it};
        std::ifstream file{itstr};
        std::ostringstream ss{};

        if (!file.is_open()) {
            throw std::runtime_error{"File passed to biner::combineFiles() failed to open."};
        }

        combined_data += settings.begin_marker + " " + itstr + "\n";

        ss << file.rdbuf();
        combined_data += ss.str();

        file.close();

        combined_data += settings.end_marker + " " + itstr + "\n";

        if (settings.verbose) {
            std::cerr << "Added file '" << it << "' to buffer.\n";
        }
    }

    if (settings.verbose) {
        std::cerr << "All done. No problems reported.\n";
    }

    return combined_data;
}

template <typename T>
void biner::separate_files(const struct Settings& settings, const std::vector<T>& files) {
    for (const auto& it : files) {
        std::string fileContents{it};

        if (std::filesystem::exists(it)) {
            const std::string itstr{it};
            std::ifstream file{itstr};
            std::ostringstream ss{};

            if (!file.is_open()) {
                throw std::runtime_error{"File passed to biner::separateFiles() failed to open."};
            }

            ss << file.rdbuf();
            fileContents = ss.str();

            file.close();

            if (settings.verbose) {
                std::cerr << "Processing file '" << it << "'.\n";
            }
        } else if (settings.verbose) {
            std::cerr << "'" << it << "' is not a file that exists, so treating it as raw data.\n";
        }

        std::size_t beginning{fileContents.find(settings.begin_marker)};
        if (beginning == std::string::npos || fileContents.find(settings.end_marker) == std::string::npos) {
            std::cerr << "The file or data specified is not valid, because it's missing biner marker data. If needed, try overriding the biner markers.\n";
            std::exit(EXIT_FAILURE);
        }
        while (beginning != std::string::npos) {
            const std::size_t end{fileContents.find(settings.end_marker, beginning)};

            if (settings.verbose) {
                std::cerr << "Parsing file.\n";
            }

            // each section
            if (end != std::string::npos) {
                const std::size_t fileNameBeginning{beginning + settings.begin_marker.size() + 1};
                const std::size_t fileNameEnd{fileContents.find('\n', fileNameBeginning)};
                const std::string fileName{
                    fileContents.substr(fileNameBeginning, fileNameEnd - fileNameBeginning)
                };

                const std::filesystem::path fs{settings.directory + fileName};
                std::string path{ fs.filename().string() };

                if (std::filesystem::exists(path)) {
                    int i{1};

                    while (i < 100000) {
                        if (!std::filesystem::exists(path + "_" + std::to_string(i))) {
                            path += "_" + std::to_string(i);
                            break;
                        }
                        i++;
                    }

                    if (i == 100000) {
                        throw std::runtime_error{"Too many duplicate files. Because I don't want to kill your SSD, I've decided to stop here."};
                    }

                    if (settings.verbose) {
                        std::cerr << "Duplicate file found, renaming it to '" << path << "'\n";
                    }
                }

                std::ofstream of{settings.directory + path};

                of << fileContents.substr(fileNameEnd + 1, end - fileNameEnd - 1); // 1 is the newline

                of.close();
            }

            fileContents.erase(beginning, end - beginning + settings.end_marker.size() + 1);
            beginning = fileContents.find(settings.begin_marker, beginning); // next file
        }

        if (settings.verbose) {
            std::cerr << "Parsed file.\n";
        }
    }

    if (settings.verbose) {
        std::cerr << "All done. No problems reported.\n";
    }
}

int main(int argc, char** argv) {
    std::string output_file{};
    const std::vector<std::string_view> arguments(argv, argv + argc);
    std::vector<std::string_view> files{};
    biner::Mode mode{biner::Mode::Undefined};
    biner::Settings settings{};

    for (std::size_t it{1}; it < arguments.size(); ++it) {
        const std::string_view arg{arguments.at(it)};

        if (arg == "-h" || arg == "--help") {
            biner::print_help(false);
            return EXIT_SUCCESS;
        } else if (arg == "-v" || arg == "--version") {
            settings.verbose = true;
        } else if (arg == "-c" || arg == "--combine") {
            mode = biner::Mode::Combine;
        } else if (arg == "-s" || arg == "--separate") {
            mode = biner::Mode::Separate;
        } else if (arg == "-d" || arg == "--directory") {
            if (arguments.size() <= it + 1) {
                std::cerr << "-d and --directory parameters require an extra parameter.\n";
                return EXIT_FAILURE;
            }
            settings.directory = arguments.at(++it);
        } else if (arg == "-bm" || arg == "--begin-marker") {
            if (arguments.size() <= it + 1) {
                std::cerr << "-bm and --begin-marker parameters require an extra parameter.\n";
                return EXIT_FAILURE;
            } else {
                settings.begin_marker = arguments.at(++it);
            }
        } else if (arg == "-em" || arg == "--end-marker") {
            if (arguments.size() <= it + 1) {
                std::cerr << "-em and --end-marker parameters require an extra parameter.\n";
                return EXIT_FAILURE;
            } else {
                settings.end_marker = arguments.at(++it);
            }
        } else if (arg == "-o" || arg == "--output") {
            if (arguments.size() <= it + 1) {
                std::cerr << "-o and --output parameters require an extra parameter.\n";
                return EXIT_FAILURE;
            } else {
                output_file = arguments.at(++it);
            }
        } else {
            if (std::filesystem::exists(arg)) {
                files.push_back(arg);
            } else {
                std::cerr << "File '" << arg << "' does not exist, or is an invalid parameter.\n";
            }
        }
    }

    if (settings.verbose) {
        std::cerr << "Verbose mode enabled (-v)\n";
        std::cerr << "Arguments:\n";
        for (const auto& it : arguments) {
            std::cerr << it << "\n";
        }
    }

    std::istream::sync_with_stdio(false);

    if (std::cin.rdbuf()->in_avail()) {
        std::string data{};
        if (settings.verbose) {
            std::cerr << "Reading from standard input.\n";
        }

        while (std::getline(std::cin, data)) {
            files.push_back(data);

            if (settings.verbose) {
                std::cerr << "Added file '" << data << "' to list.\n";
            }
        }
    } else if (settings.verbose) {
        std::cerr << "Not reading from standard input.\n";
    }

    if (!std::filesystem::exists(settings.directory)) {
        if (!std::filesystem::create_directory(settings.directory)) {
            std::cerr << "Failed to create directory, exiting.\n";
            return EXIT_FAILURE;
        }

        if (settings.verbose) {
            std::cerr << "Created directory '" << settings.directory << "' because it does not exist.\n";
        }
    }

#ifdef _WIN32
        if (settings.directory.at(settings.directory.size() - 1) == '\\')
            settings.directory += "\\";
#else
        if (settings.directory.at(settings.directory.size() - 1) == '/')
            settings.directory += "/";
#endif

    if (mode == biner::Mode::Undefined) {
        std::cerr << "You must specify a mode.\n";
        return EXIT_FAILURE;
    }

    if (settings.verbose) {
        std::cerr << "Files:\n";

        for (const auto& it : files) {
            std::cerr << it << "\n";
        }

        std::cerr << (mode == biner::Mode::Combine ? "Biner in combine mode.\n" : "Biner in separate mode.\n");
    }

    if (files.empty() == true) {
        if (mode == biner::Mode::Combine) {
            std::cerr << "You must specify at least two files to combine.\n";
        } else {
            std::cerr << "You must specify at least one file to split.\n";
        }

        return EXIT_FAILURE;
    }

    try {
        if (mode == biner::Mode::Separate) {
            biner::separate_files(settings, files);
        } else {
            if (output_file.empty()) {
                if (settings.verbose) {
                    std::cerr << "Outputting data to standard output (stdout)\n";
                }

                std::cout << biner::combine_files(settings, files);
            } else {
                if (settings.verbose) {
                    std::cerr << "Writing data to file '" << output_file << "'\n";
                }

                std::filesystem::path fs = output_file;
                std::string dirname = fs.remove_filename().string();

                if (!std::filesystem::exists(dirname) && !dirname.empty()) {
                    if (!std::filesystem::create_directories(fs)) {
                        throw std::runtime_error{"Failed to create directory."};
                    }
                }

                std::ofstream of{output_file};

                of << biner::combine_files(settings, files);

                of.close();
            }

            return EXIT_SUCCESS;
        }
    } catch (const std::exception& e) {
        std::cerr << "biner failed to perform the action you requested.\n" << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
