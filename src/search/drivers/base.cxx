#include <search/drivers/base.hxx>
#include <utils/archive/json.hxx>
#include <utils/printers/printers.hxx>
#include <utils/printers/helper.hxx>
#include <utils/printers/actions.hxx>
#include <problem.hxx>
#include <problem_info.hxx>

namespace fs0 { namespace drivers {

void
EmbeddedDriver::reset_results() {
    plan.clear();
    solved = false;
    oom = false;
    search_time = false;
    total_planning_time = 0.0f;
    valid = false;
    gen_speed = 0.0;
    eval_speed = 0.0;
    peak_memory = 0;
    result = ExitCode::UNSOLVABLE;
}

void
EmbeddedDriver::archive_results_JSON(std::string filename) {
    using namespace rapidjson;

    const ProblemInfo& info = ProblemInfo::getInstance();

    Document report;
    Document::AllocatorType& allocator = report.GetAllocator();
    report.SetObject();

    //! Domain & report name
    Value domainName;
    domainName.SetString(StringRef(info.getDomainName().c_str()));
    report.AddMember("domain", domainName.Move(), allocator );

    Value instanceName;
    instanceName.SetString(StringRef(info.getInstanceName().c_str()));
    report.AddMember("instance", instanceName.Move(), allocator );

    //! Result
    Value result_string;
    result_string.SetString(StringRef(exit_code_string( result )));
    report.AddMember( "result", result_string.Move(), allocator );

    //! Scalar values
    report.AddMember( "solved", Value(solved), allocator);
    report.AddMember( "oom", Value(oom), allocator);
    report.AddMember( "search_time", Value(search_time), allocator);
    report.AddMember( "total_planning_time", Value(total_planning_time), allocator);
    report.AddMember( "valid", Value(valid), allocator);
    report.AddMember( "gen_speed", Value(gen_speed), allocator);
    report.AddMember( "eval_speed", Value(eval_speed), allocator);
    report.AddMember( "peak_memory", Value(peak_memory), allocator);

    //! the plan
    Value the_plan(kArrayType);
    {
        const auto& actions = Problem::getInstance().getGroundActions();
    	for (const auto& action_id:plan) {
            Value name;
            std::string buffer( printer() << print::action_header(*actions.at(action_id)));
            name.SetString(StringRef(buffer.c_str()));
            report.AddMember("instance", name.Move(), allocator );
    	}
    }
    report.AddMember("plan", the_plan, allocator);

    JSONArchive::write_to_file( filename, report );
}

}}
