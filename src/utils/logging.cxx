
#include <utils/logging.hxx>
#include <boost/filesystem.hpp>


namespace fs0 {
	
Logger* Logger::_instance = nullptr;

Logger::Logger(const std::string& logDirectory)
	: _logDirectory(logDirectory) , _timer()
{
	if(_logDirectory.empty()) throw std::runtime_error("Empty logging directory specified");
	boost::filesystem::create_directory(_logDirectory);
}


Logger::~Logger() {
	for(auto it:_files) {
		 it.second->close();
		 delete it.second;
	}
}

std::ofstream& Logger::log( const std::string & key) {
	FileMap::iterator it = _files.find(key);
	
	if(it == _files.end()) {  // create a new file if it wasn't previously there
		std::string filename = _logDirectory + "/" + key + ".log";
		auto res = _files.insert(std::make_pair(key, new std::ofstream(filename.c_str())));
		it = res.first;
	}
	return *(it->second);
}

std::ostream& Logger::log(const std::string& level, const std::string& fileName) {
	boost::timer::cpu_times elapsed = _timer.elapsed();
	std::ostream& stream = log(fileName);
	
	// Append the log level
	stream  << "[" << level << "]";
	
	// Append the time
	stream  << "[" << boost::format("%8.5f") % (elapsed.wall / 1e9) << "]";
	
	// Append a space for readability
	stream << " ";
	return stream;
}

} // namespaces
