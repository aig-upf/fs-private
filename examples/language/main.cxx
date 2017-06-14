
#include <iostream>

#include <lapkt/tools/logging.hxx>

#include <utils/system.hxx>
#include <fstrips/language_info.hxx>


using namespace fs0;

// Our main function simply creates a runner with the command-line options plus the generator function
// which is specific to a single problem instance, and then runs the search engine
int main(int argc, char** argv) {
	fs0::init_fs_system();
	lapkt::tools::Logger::init("./logs");
//  	fs0::drivers::Runner runner(fs0::drivers::EngineOptions(argc, argv), generate);
// 	return runner.run();
	
	
	fstrips::LanguageInfo lang;
	
	TypeIdx block_t = lang.add_fstype("block");
	
	auto id = lang.add_object("b1", block_t);
	
	
	std::cout << "Object b1 has id "<< id << std::endl;
	
	
}
