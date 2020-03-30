/*
 * This is largely based verbatim on Fast Downward's GPL'd Planner signal-handling routine:
 * http://www.fast-downward.org/ObtainingAndRunningFastDownward
 */

#pragma once

#if defined(_MSC_VER)
#define NO_RETURN __declspec(noreturn)
#else
#define NO_RETURN __attribute__((noreturn))
#endif


/*
 * Author:  David Robert Nadeau
 * Site:    http://NadeauSoftware.com/
 * License: Creative Commons Attribution 3.0 Unported License
 *          http://creativecommons.org/licenses/by/3.0/deed.en_US
 */
namespace fs0::utils {

size_t getCurrentRSS( );
size_t getPeakRSS( );

} // namespaces


namespace fs0 {

void out_of_memory_handler();

//! Entry point to perform some calls to the system
//! relevant to the well-functioning of FS.
void init_fs_system();

/* Let us keep here a Fast-Downward-compatible list of exit codes, meaning: use their exit codes as much as possible,
   so that we get extra compatibility with tools such as Lab, etc., for free */
enum class ExitCode : int {
    // 0-9: exit codes denoting a plan was found
    SUCCESS = 0,

    // 10-19: exit codes denoting no plan was found (without any error)
    SEARCH_UNSOLVABLE = 11,  // Task is provably unsolvable with given bound.
    SEARCH_UNSOLVED_INCOMPLETE = 12,  // Search ended without finding a solution.

    // 20-29: "expected" failures
    SEARCH_OUT_OF_MEMORY = 22,
    SEARCH_OUT_OF_TIME = 23,

    // 30-39: unrecoverable errors
    SEARCH_CRITICAL_ERROR = 32,
    SEARCH_INPUT_ERROR = 33,
    SEARCH_UNSUPPORTED = 34
};


NO_RETURN extern void exit_with(ExitCode returncode);
NO_RETURN extern void exit_after_receiving_signal(ExitCode returncode);

int get_peak_memory_in_kb();
const char *get_exit_code_message_reentrant(ExitCode exitcode);
bool is_exit_code_error_reentrant(ExitCode exitcode);
void register_event_handlers();
void report_exit_code_reentrant(ExitCode exitcode);
int get_process_id();

size_t get_current_memory_in_kb();

//! Disable unix core dumps
void limit_core_dump_size();

} // namespaces
