
#include <search/options.hxx>
#include <search/runner.hxx>
#include <utils/system.hxx>

// This include will dinamically point to the adequate per-instance automatically generated file
#include <components.hxx>  


// Our main function simply creates a runner with the command-line options plus the generator function
// which is specific to a single problem instance, and then runs the search engine
int main(int argc, char** argv) {
	fs0::register_event_handlers();
	fs0::drivers::Runner runner(fs0::drivers::EngineOptions(argc, argv), generate);
	return runner.run();
}
