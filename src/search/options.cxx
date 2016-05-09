

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include <search/options.hxx>
#include <utils/config.hxx>
#include <aptk2/tools/logging.hxx>

namespace po = boost::program_options;

namespace fs0 { namespace drivers {

EngineOptions::EngineOptions(int argc, char** argv) {
	po::options_description description("Allowed options");
	description.add_options()
		("help,h", "Display this help message")
		("timeout,t", po::value<int>()->default_value(10),                        "The timeout, in seconds.")
		("data", po::value<std::string>()->default_value("data"),                 "The directory where the input data is stored.")
		("driver,d", po::value<std::string>()->required(),                        "The desired driver.")
		("defaults", po::value<std::string>()->default_value("./defaults.json"),  "The planner configuration file.")
		("options", po::value<std::string>()->default_value(""),                  "Additional configuration options.")
		("out", po::value<std::string>()->default_value("."),                     "The directory where the results data is to be output.");

	po::variables_map vm;
	try {
		po::store(po::command_line_parser(argc, argv).options(description).run(), vm);
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
	_defaults = vm["defaults"].as<std::string>();
	_output_dir = vm["out"].as<std::string>();
	_driver = vm["driver"].as<std::string>();
	
	// Populate the map of additional options
	std::string options = vm["options"].as<std::string>();
	if (options != "") {
		std::vector<std::string> config_options;
		boost::split(config_options, options, boost::is_any_of(","));
		for (auto& option:config_options) {
			std::vector<std::string> key_val;
			boost::split(key_val, option, boost::is_any_of("="));
			if (key_val.size() != 2) throw std::runtime_error(std::string("Cannot recognize configuration option ") + option);
			
			auto res = _user_options.insert(std::make_pair(key_val[0], key_val[1]));
			if (!res.second) throw std::runtime_error(std::string("Duplicate configuration key ") + key_val[0]);
		}
	}
}

} } // namespaces
