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

#include <Api.h>
#include <Api/utils/local.hpp>
#include <iostream>
#include <LibUtils/ArgsParser.hh>
#include <string_view>

#define V_MAJOR 0
#define V_MINOR 0
#define V_PATCH 1

int main(int argc, const char** argv) {
    Tid              target_thread_id = -1;
    std::string_view task_type{ "Thread" };

    Utils::ArgsParser args_parser{ "Kill Utility", V_MAJOR, V_MINOR, V_PATCH };
    args_parser.add_positional_argument(target_thread_id, "Thread/Process ID", "TargetID");

    /* parse the arguments */
    args_parser.parse(argc, argv);

    /* obtain the pid of the process */
    auto target_proc_id = s_get_pid_for_tid(target_thread_id);
    if ( target_proc_id == target_thread_id )
        task_type = "Process";

    /* obtain the task name */
    Local task_name{ new char[STRING_IDENTIFIER_MAX_LENGTH] };
    s_get_task_identifier(target_thread_id, task_name());

    std::cout << task_type << " '" << task_name() << "':" << target_thread_id << ": ";

    /* well now kill the task */
    auto kill_status = s_kill(target_thread_id);
    switch ( kill_status ) {
        case KILL_STATUS_SUCCESSFUL:
            std::cout << "Killed successfully" << std::endl;
            break;
        case KILL_STATUS_NOT_FOUND:
            std::cout << "Not found" << std::endl;
            break;
        case KILL_STATUS_FAILED:
            std::cout << "Failed" << std::endl;
            break;
        default:
            std::cout << "Received an unknown kill_status" << std::endl;
            break;
    }
    return kill_status == KILL_STATUS_SUCCESSFUL ? EXIT_SUCCESS : EXIT_FAILURE;
}
