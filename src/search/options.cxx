

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <search/options.hxx>
#include <utils/config.hxx>
#include <utils/logging.hxx>

namespace po = boost::program_options;

namespace fs0 { namespace drivers {

EngineOptions::EngineOptions(int argc, char** argv) {
	po::options_description description("Allowed options");
	description.add_options()
		("help,h", "Display this help message")
		("timeout,t", po::value<int>()->default_value(10),                     "The timeout, in seconds.")
		("data", po::value<std::string>()->default_value("data"),              "The directory where the input data is stored.")
		("config", po::value<std::string>()->default_value("./config.json"),   "The planner configuration file.")
		("out", po::value<std::string>()->default_value("."),                  "The directory where the results data is to be output.");

	po::positional_options_description pos;
	pos.add("timeout", 1)
	   .add("data", 1)
	   .add("config", 1)
	   .add("out", 1);

	po::variables_map vm;
	try {
		po::store(po::command_line_parser(argc, argv).options(description).positional(pos).run(), vm);
		po::notify(vm);
	} catch(const boost::program_options::invalid_option_value& ex) {
		std::cout << "Wrong parameter types:";
		std::cout << ex.what() << std::endl;
		std::cout << std::endl << description << std::endl;
		throw std::runtime_error("Wrong engine options");
	}

	if (vm.count("help")) {
		std::cout << description << "\n";
		exit(0);
	}

	_timeout = vm["timeout"].as<int>();
	_data_dir = vm["data"].as<std::string>();
	_config = vm["config"].as<std::string>();
	_output_dir = vm["out"].as<std::string>();
}

} } // namespaces
