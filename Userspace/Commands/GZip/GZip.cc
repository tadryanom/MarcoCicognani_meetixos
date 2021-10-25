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
#include <zlib.h>

#define V_MAJOR 0
#define V_MINOR 0
#define V_PATCH 1

int main(int argc, const char** argv) {
    auto archives = std::vector<std::string>{};

    auto args_parser = Utils::ArgsParser{ "GNU Zip Utility", V_MAJOR, V_MINOR, V_PATCH };
    args_parser.add_positional_argument(archives, "Archives to extract", "Archive", true);

    /* parse the arguments */
    args_parser.parse(argc, argv);

    for ( auto& archive : archives ) {
        /* check for right extension in filename */
        if ( !archive.ends_with(".gz") ) {
            std::cerr << archive << ": Not sure this file is gzipped\n";
            continue;
        }

        /* open the gz-file */
        auto gz_file = gzopen(archive.c_str(), "r");
        if ( !gz_file ) {
            std::cerr << archive << ": Unable to open\n";
            continue;
        }

        /* open the output stream */
        auto dest_path   = archive.substr(0, archive.length() - 3);
        auto dest_stream = std::ofstream{ dest_path };
        if ( !dest_stream.is_open() ) {
            std::cerr << dest_path << ": Unable to open/create\n";
            gzclose(gz_file);
            continue;
        }

        /* extract from the archive */
        while ( !gzeof(gz_file) ) {
            /* read the extracted content */
            char buffer[4096];
            auto read_bytes = gzread(gz_file, &buffer, 4096);

            /* write-out to the file */
            dest_stream.write(reinterpret_cast<const char*>(&buffer), read_bytes);
        }

        gzclose(gz_file);
    }
    return EXIT_SUCCESS;
}