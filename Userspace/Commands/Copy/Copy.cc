/**
 * @brief
 * This file is part of the MeetiX Operating System.
 * Copyright (c) 2017-2021, Marco Cicognani (marco.cicognani@meetixos.org)
 *
 * @developers
 * Marco Cicognani (marco.cicognani@meetixos.org)
 *
 * @license
 * GNU General Public License version 3
 */

#include <fstream>
#include <iostream>
#include <string>
#include <Utils/ArgsParser.hh>

#define V_MAJOR 0
#define V_MINOR 0
#define V_PATCH 1

int main(int argc, const char** argv) {
    auto src_path = std::string{};
    auto dst_path = std::string{};

    auto args_parser = Utils::ArgsParser{ "Copy Utility", V_MAJOR, V_MINOR, V_PATCH };
    args_parser.add_positional_argument(src_path, "Source file to copy", "SourcePath");
    args_parser.add_positional_argument(dst_path, "Destination file", "DestinationPath");

    /* parse the arguments */
    args_parser.parse(argc, argv);

    /* open the source file to copy */
    auto src_stream = std::ifstream{ src_path };
    if ( !src_stream.is_open() ) {
        std::cerr << "Failed to open '" << src_path << "'" << std::endl;
        return EXIT_FAILURE;
    }

    /* open or create the destination file */
    auto dst_stream = std::ofstream{ dst_path };
    if ( !dst_stream.is_open() ) {
        std::cerr << "Failed to open '" << dst_path << "'" << std::endl;
        return EXIT_FAILURE;
    }

    /* copy the file content */
    dst_stream << src_stream.rdbuf();
    return EXIT_SUCCESS;
}
