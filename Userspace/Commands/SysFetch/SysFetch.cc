/**
 * @brief
 * This file is part of the MeetiX Operating System.
 * Copyright (c) 2017-2022, Marco Cicognani (marco.cicognani@meetixos.org)
 *
 * @developers
 * Marco Cicognani (marco.cicognani@meetixos.org)
 *
 * @license
 * GNU General Public License version 3
 */

#include <iostream>
#include <LibIO/Shell.hh>
#include <LibUtils/ArgsParser.hh>
#include <LibUtils/Environment.hh>

#define V_MAJOR 0
#define V_MINOR 0
#define V_PATCH 1

int main(int argc, const char** argv) {
    Utils::ArgsParser args_parser{ "System Info Fetch Utility", V_MAJOR, V_MINOR, V_PATCH };

    /* parse the arguments */
    args_parser.parse(argc, argv);

    auto size_value = [](usize value) {
        char multiplier = 'K';
        if ( value >= 1024 * 1024 ) {
            value      = value / 1024 / 1024;
            multiplier = 'G';
        } else if ( value >= 1024 ) {
            value      = value / 1024;
            multiplier = 'M';
        }

        std::stringstream ss{};
        ss << value << multiplier;
        return ss.str();
    };

    /* obtain kernel information */
    KernelName kern_name{};
    s_kernel_name(&kern_name);

    SystemInfo sys_info{};
    s_system_info(&sys_info);

    std::cout << "\033[32m\n";
    std::cout << "                           *************" << '\n';
    std::cout << "                 ***********************" << '\n';
    std::cout << "          ******************************" << '\n';
    std::cout << "     *\033[0m@@@@@\033[32m******\033[0m@@@@\033[32m*******************" << '\n';
    std::cout << "  ****\033[0m@@@*\033[0m@@\033[32m****\033[0m@@@@@\033[32m******\033[0m@@@@@@@@@\033[32m****" << '\n';
    std::cout << " *****\033[0m@@@\033[32m**\033[0m@@\033[32m**\033[0m@@\033[32m*\033[0m@@@\033[32m****\033[0m@@@\033["
                 "32m*******\033[0m@@@\033[32m**"
              << '\n';
    std::cout << "******\033[0m@@@\033[32m***\033[0m@@@@\033[32m**\033[0m@@@\033[32m****\033[0m@@@@"
                 "@@@@@@@@@\033[32m**"
              << '\n';
    std::cout << " *****\033[0m@@@\033[32m****\033[0m@@\033[32m***\033[0m@@@\033[32m****\033[0m@@"
                 "\033[32m*************"
              << '\n';
    std::cout << "    "
                 "**\033[0m@@@\033[32m*********\033[0m@@@\033[32m****\033[0m@@@\033[32m*******\033[0m@@@\033[32m**"
              << '\n';
    std::cout << "     *\033[0m@@@\033[32m*********\033[0m@@@\033[32m******\033[0m@@@@@@@@@\033[32m****" << '\n';
    std::cout << "          ******************************" << '\n';
    std::cout << "                 ***********************" << '\n';
    std::cout << "                           *************" << '\n';
    std::cout << "\033[0m";

    IO::Shell::instance().move_cursor_up(12);

    /* print out user@hostname vVERSION */
    IO::Shell::instance().move_cursor_forward(45);
    std::cout << "\033[32m" << Utils::Environment::logged_user() << "\033[0m@\033[31m";
    std::cout << Utils::Environment::hostname() << "\033[0m v";
    std::cout << Utils::Environment::version() << '\n';

    /* print out a separator */
    IO::Shell::instance().move_cursor_forward(45);
    std::cout << "--------------------------------------\n";

    /* print out system name */
    IO::Shell::instance().move_cursor_forward(45);
    std::cout << "\033[32mOS\033[0m: MeetiX OS\n";

    /* print out shell */
    IO::Shell::instance().move_cursor_forward(45);
    std::cout << "\033[32mShell\033[0m: MxSh (/Bins/MxSh)\n";

    /* print out terminal */
    IO::Shell::instance().move_cursor_forward(45);
    std::cout << "\033[32mTerminal\033[0m: Terminal (/Apps/Terminal/Bin/Terminal)\n";

    /* print out loader */
    IO::Shell::instance().move_cursor_forward(45);
    std::cout << "\033[32mLoader\033[0m: ";
    std::cout << kern_name.m_loader_name << " v";
    std::cout << kern_name.m_loader_major << '.';
    std::cout << kern_name.m_loader_minor << '.';
    std::cout << kern_name.m_loader_patch << '\n';

    /* print out kernel */
    IO::Shell::instance().move_cursor_forward(45);
    std::cout << "\033[32mKernel\033[0m: ";
    std::cout << kern_name.m_kernel_name << " v";
    std::cout << kern_name.m_kernel_major << '.';
    std::cout << kern_name.m_kernel_minor << '.';
    std::cout << kern_name.m_kernel_sub << '.';
    std::cout << kern_name.m_kernel_patch << '\n';

    /* print out memory */
    IO::Shell::instance().move_cursor_forward(45);
    std::cout << "\033[32mMemory\033[0m: ";
    std::cout << size_value(sys_info.m_memory_total_amount - sys_info.m_memory_free_amount) << '/';
    std::cout << size_value(sys_info.m_memory_total_amount) << '\n';

    /* print out CPU name */
    IO::Shell::instance().move_cursor_forward(45);
    std::cout << "\033[32mCPU\033[0m: " << sys_info.m_cpu_vendor << '\n';

    /* print out CPU cores count */
    IO::Shell::instance().move_cursor_forward(45);
    std::cout << "\033[32mCPU Cores\033[0m: " << sys_info.m_cpu_count << '\n';

    /* print out colors */
    IO::Shell::instance().move_cursor_forward(45);
    std::cout << "\033[40m  \033[41m  \033[42m  \033[43m  \033[44m  \033[45m  \033[46m  \033[47m  ";
    std::cout << "\033[0m\n\n\n";
    return EXIT_SUCCESS;
}
