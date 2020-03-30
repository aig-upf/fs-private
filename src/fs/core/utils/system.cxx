/*
 * This is largely based verbatim on Fast Downward's GPL'd Planner signal-handling routine:
 * http://www.fast-downward.org/ObtainingAndRunningFastDownward
 */


#include <csignal>
#include <cstdlib>
#include <cstring>
#include <ctype.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <limits>
#include <unistd.h>

#include <fs/core/utils/system.hxx>

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#include <sys/resource.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <mach/mach.h>

#elif (defined(_AIX) || defined(__TOS__AIX__)) || (defined(__sun__) || defined(__sun) || defined(sun) && (defined(__SVR4) || defined(__svr4__)))
#include <fcntl.h>
#include <procfs.h>

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
#include <stdio.h>

#endif

#else
#error "Cannot define getPeakRSS( ) or getCurrentRSS( ) for an unknown OS."
#endif

namespace fs0 {


void write_reentrant(int filedescr, const char *message, int len) {
    while (len > 0) {
        int written;
        do {
            written = write(filedescr, message, len);
        } while (written == -1 && errno == EINTR);
        /*
          We could check for other values of errno here but all errors except
          EINTR are catastrophic enough to abort, so we do not need the
          distinction.
        */
        if (written == -1)
            abort();
        message += written;
        len -= written;
    }
}

void write_reentrant_str(int filedescr, const char *message) {
    write_reentrant(filedescr, message, strlen(message));
}

void write_reentrant_char(int filedescr, char c) {
    write_reentrant(filedescr, &c, 1);
}

void write_reentrant_int(int filedescr, int value) {
    char buffer[32];
    int len = snprintf(buffer, sizeof(buffer), "%d", value);
    if (len < 0)
        abort();
    write_reentrant(filedescr, buffer, len);
}

bool read_char_reentrant(int filedescr, char *c) {
    int result;
    do {
        result = read(filedescr, c, 1);
    } while (result == -1 && errno == EINTR);
    /*
      We could check for other values of errno here but all errors except
      EINTR are catastrophic enough to abort, so we do not need the
      distinction.
    */
    if (result == -1)
        abort();
    return result == 1;
}

void print_peak_memory_reentrant() {
    int proc_file_descr = TEMP_FAILURE_RETRY(open("/proc/self/status", O_RDONLY));
    if (proc_file_descr == -1) {
        write_reentrant_str(
            STDERR_FILENO,
            "critical error: could not open /proc/self/status\n");
        abort();
    }

    const char magic[] = "\nVmPeak:";
    char c;
    size_t pos_magic = 0;
    const size_t len_magic = sizeof(magic) - 1;

    // Find magic word.
    while (pos_magic != len_magic && read_char_reentrant(proc_file_descr, &c)) {
        if (c == magic[pos_magic]) {
            ++pos_magic;
        } else {
            pos_magic = 0;
        }
    }

    if (pos_magic != len_magic) {
        write_reentrant_str(
            STDERR_FILENO,
            "critical error: could not find VmPeak in /proc/self/status\n");
        abort();
    }

    write_reentrant_str(STDOUT_FILENO, "Peak memory: ");

    // Skip over whitespace.
    while (read_char_reentrant(proc_file_descr, &c) && isspace(c))
        ;

    do {
        write_reentrant_char(STDOUT_FILENO, c);
    } while (read_char_reentrant(proc_file_descr, &c) && !isspace(c));

    write_reentrant_str(STDOUT_FILENO, " KB\n");
    /*
      Ignore potential errors other than EINTR (there is nothing we can do
      about I/O errors or bad file descriptors here).
    */
    TEMP_FAILURE_RETRY(close(proc_file_descr));
}

void exit_handler(int, void *) {
    print_peak_memory_reentrant();
}

void out_of_memory_handler() {
    /*
      We do not use any memory padding currently. The methods below should
      only use stack memory. If we ever run into situations where the stack
      memory is not sufficient, we can consider using sigaltstack to reserve
      memory for the stack of the signal handler and raising a signal here.
    */
    write_reentrant_str(STDOUT_FILENO, "Failed to allocate memory.\n");
    exit_with(ExitCode::SEARCH_OUT_OF_MEMORY);
}

void signal_handler(int signal_number) {
    print_peak_memory_reentrant();
    write_reentrant_str(STDOUT_FILENO, "caught signal ");
    write_reentrant_int(STDOUT_FILENO, signal_number);
    write_reentrant_str(STDOUT_FILENO, " -- exiting\n");
    if (signal_number == SIGXCPU) {
        exit_after_receiving_signal(ExitCode::SEARCH_OUT_OF_TIME);
    }
    raise(signal_number);
}

size_t get_current_memory_in_kb() { return utils::getCurrentRSS() / 1024; }


/*
  NOTE: we have two variants of obtaining peak memory information.
        get_peak_memory_in_kb() is used during the regular execution.
        print_peak_memory_in_kb_reentrant() is used in signal handlers.
        The latter is slower but guarantees reentrancy.
*/
int get_peak_memory_in_kb() {
    // On error, produces a warning on cerr and returns -1.
    int memory_in_kb = -1;

    std::ifstream procfile;
    procfile.open("/proc/self/status");
    std::string word;
    while (procfile.good()) {
        procfile >> word;
        if (word == "VmPeak:") {
            procfile >> memory_in_kb;
            break;
        }
        // Skip to end of line.
        procfile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    if (procfile.fail()) {
        memory_in_kb = -1;
	}

    if (memory_in_kb == -1) {
        std::cerr << "warning: could not determine peak memory" << std::endl;
	}
    return memory_in_kb;
}


void register_event_handlers() {
    // Terminate when running out of memory.
    std::set_new_handler(out_of_memory_handler);

    // On exit or when receiving certain signals such as SIGINT (Ctrl-C),
    // print the peak memory usage.
    on_exit(exit_handler, 0);
    struct sigaction default_signal_action;
    default_signal_action.sa_handler = signal_handler;
    // Block all signals we handle while one of them is handled.
    sigemptyset(&default_signal_action.sa_mask);
    sigaddset(&default_signal_action.sa_mask, SIGABRT);
    sigaddset(&default_signal_action.sa_mask, SIGTERM);
    sigaddset(&default_signal_action.sa_mask, SIGSEGV);
    sigaddset(&default_signal_action.sa_mask, SIGINT);
    sigaddset(&default_signal_action.sa_mask, SIGXCPU);
    // Reset handler to default action after completion.
    default_signal_action.sa_flags = SA_RESETHAND;

    sigaction(SIGABRT, &default_signal_action, 0);
    sigaction(SIGTERM, &default_signal_action, 0);
    sigaction(SIGSEGV, &default_signal_action, 0);
    sigaction(SIGINT, &default_signal_action, 0);
    sigaction(SIGXCPU, &default_signal_action, 0);
}

void report_exit_code_reentrant(ExitCode exitcode) {
    const char *message = get_exit_code_message_reentrant(exitcode);
    bool is_error = is_exit_code_error_reentrant(exitcode);
    if (message) {
        int filedescr = is_error ? STDERR_FILENO : STDOUT_FILENO;
        write_reentrant_str(filedescr, message);
        write_reentrant_char(filedescr, '\n');
    } else {
        write_reentrant_str(STDERR_FILENO, "Exitcode: ");
        write_reentrant_int(STDERR_FILENO, static_cast<int>(exitcode));
        write_reentrant_str(STDERR_FILENO, "\nUnknown exitcode.\n");
        abort();
    }
}

int get_process_id() {
    return getpid();
}

void limit_core_dump_size() {
	// Set core max size to 0
	struct rlimit rlim;
	rlim.rlim_cur = rlim.rlim_max = 0;
	setrlimit(RLIMIT_CORE, &rlim);
}

void init_fs_system() {
#if !defined(DEBUG)
	limit_core_dump_size();
#endif

	register_event_handlers();
}


const char *get_exit_code_message_reentrant(ExitCode exitcode) {
    switch (exitcode) {
        case ExitCode::SUCCESS:
            return "Solution found.";
        case ExitCode::SEARCH_CRITICAL_ERROR:
            return "Unexplained error occurred.";
        case ExitCode::SEARCH_INPUT_ERROR:
            return "Usage error occurred.";
        case ExitCode::SEARCH_UNSUPPORTED:
            return "Tried to use unsupported feature.";
        case ExitCode::SEARCH_UNSOLVABLE:
            return "Task is provably unsolvable.";
        case ExitCode::SEARCH_UNSOLVED_INCOMPLETE:
            return "Search stopped without finding a solution.";
        case ExitCode::SEARCH_OUT_OF_MEMORY:
            return "Memory limit has been reached.";
        case ExitCode::SEARCH_OUT_OF_TIME:
            return "Time limit has been reached.";
        default:
            return nullptr;
    }
}

bool is_exit_code_error_reentrant(ExitCode exitcode) {
    switch (exitcode) {
        case ExitCode::SUCCESS:
        case ExitCode::SEARCH_UNSOLVABLE:
        case ExitCode::SEARCH_UNSOLVED_INCOMPLETE:
        case ExitCode::SEARCH_OUT_OF_MEMORY:
        case ExitCode::SEARCH_OUT_OF_TIME:
            return false;
        case ExitCode::SEARCH_CRITICAL_ERROR:
        case ExitCode::SEARCH_INPUT_ERROR:
        case ExitCode::SEARCH_UNSUPPORTED:
        default:
            return true;
    }
}

void exit_with(ExitCode exitcode) {
    report_exit_code_reentrant(exitcode);
    exit(static_cast<int>(exitcode));
}

void exit_after_receiving_signal(ExitCode exitcode) {
    /*
      In signal handlers, we have to use the "safe function" _Exit() rather
      than the unsafe function exit().
    */
    report_exit_code_reentrant(exitcode);
    _Exit(static_cast<int>(exitcode));
}

} // namespaces

namespace fs0::utils {

/**
 * Returns the peak (maximum so far) resident set size (physical
 * memory use) measured in bytes, or zero if the value cannot be
 * determined on this OS.
 */
size_t getPeakRSS( )
{
#if defined(_WIN32)
    /* Windows -------------------------------------------------- */
    PROCESS_MEMORY_COUNTERS info;
    GetProcessMemoryInfo( GetCurrentProcess( ), &info, sizeof(info) );
    return (size_t)info.PeakWorkingSetSize;

#elif (defined(_AIX) || defined(__TOS__AIX__)) || (defined(__sun__) || defined(__sun) || defined(sun) && (defined(__SVR4) || defined(__svr4__)))
    /* AIX and Solaris ------------------------------------------ */
    struct psinfo psinfo;
    int fd = -1;
    if ( (fd = open( "/proc/self/psinfo", O_RDONLY )) == -1 )
        return (size_t)0L;      /* Can't open? */
    if ( read( fd, &psinfo, sizeof(psinfo) ) != sizeof(psinfo) )
    {
        close( fd );
        return (size_t)0L;      /* Can't read? */
    }
    close( fd );
    return (size_t)(psinfo.pr_rssize * 1024L);

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
    /* BSD, Linux, and OSX -------------------------------------- */
    struct rusage rusage;
    getrusage( RUSAGE_SELF, &rusage );
#if defined(__APPLE__) && defined(__MACH__)
    return (size_t)rusage.ru_maxrss;
#else
    return (size_t)(rusage.ru_maxrss * 1024L);
#endif

#else
    /* Unknown OS ----------------------------------------------- */
    return (size_t)0L;          /* Unsupported. */
#endif
}



/**
 * Returns the current resident set size (physical memory use) measured
 * in bytes, or zero if the value cannot be determined on this OS.
 */
size_t getCurrentRSS( )
{
#if defined(_WIN32)
    /* Windows -------------------------------------------------- */
    PROCESS_MEMORY_COUNTERS info;
    GetProcessMemoryInfo( GetCurrentProcess( ), &info, sizeof(info) );
    return (size_t)info.WorkingSetSize;

#elif defined(__APPLE__) && defined(__MACH__)
    /* OSX ------------------------------------------------------ */
    struct mach_task_basic_info info;
    mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
    if ( task_info( mach_task_self( ), MACH_TASK_BASIC_INFO,
        (task_info_t)&info, &infoCount ) != KERN_SUCCESS )
        return (size_t)0L;      /* Can't access? */
    return (size_t)info.resident_size;

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
    /* Linux ---------------------------------------------------- */
    long rss = 0L;
    FILE* fp = NULL;
    if ( (fp = fopen( "/proc/self/statm", "r" )) == NULL )
        return (size_t)0L;      /* Can't open? */
    if ( fscanf( fp, "%*s%ld", &rss ) != 1 )
    {
        fclose( fp );
        return (size_t)0L;      /* Can't read? */
    }
    fclose( fp );
    return (size_t)rss * (size_t)sysconf( _SC_PAGESIZE);

#else
    /* AIX, BSD, Solaris, and Unknown OS ------------------------ */
    return (size_t)0L;          /* Unsupported. */
#endif
}

} // namespaces
