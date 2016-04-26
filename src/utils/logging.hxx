
#pragma once


#include <fstream>
#include <boost/format.hpp>
#include <boost/timer/timer.hpp>
#include <map>


namespace fs0 {


/**
 * The Logger class uses a singleton and keeps track of time in order to timestamp the log messages.
 */
class Logger
{
protected:
	typedef std::map <std::string, std::ofstream *> FileMap;

	FileMap _files;

	//! The directory where the log files will be written
	std::string _logDirectory;

	//! The starting time, for reference
	boost::timer::cpu_timer _timer;

	//! Singleton
	static std::unique_ptr<Logger> _instance;

	//! Basic version of the logging: returns a stream to write a message to the given filename
	std::ofstream& log(const std::string& fileName);

public:
	Logger(const std::string& logDirectory);
	virtual ~Logger();

	//! Explicit initizalition of the singleton
	static void init(const std::string& logDirectory) {
		_instance = std::unique_ptr<Logger>(new Logger(logDirectory));
	}

	//! Retrieve the singleton instance, which has been previously initialized
	static Logger& instance() {
		if (!_instance) throw std::runtime_error("The logger needs to be explicitly initialized before using it");
		return *_instance;
	}

	//! Returns a stream to debug to the given filename
	std::ostream& log(const std::string& level, const std::string& filename);
};

} // namespaces


// ************************
// INFO MACRO
// ************************
#define FINFO(file, message) fs0::Logger::instance().log("INFO", file) << message << std::endl;

// ************************
// DEBUG MACRO
// ************************
#ifdef DEBUG
#define FDEBUG(file, message) fs0::Logger::instance().log("DEBUG", file) << message << std::endl;
#else
#define FDEBUG(file, message)
#endif


// ************************
// EXTREME DEBUG MACRO
// ************************
#ifdef EDEBUG
#define FFDEBUG(file, message) fs0::Logger::instance().log("EDEBUG", file) << message << std::endl;
#else
#define FFDEBUG(file, message)
#endif
