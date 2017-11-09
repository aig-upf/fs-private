#include <fs/core/search/drivers/base.hxx>
#include <fs/core/utils/printers/printers.hxx>
#include <fs/core/utils/printers/helper.hxx>
#include <fs/core/utils/printers/actions.hxx>
#include <fs/core/problem.hxx>
#include <fs/core/problem_info.hxx>

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

    //! Scalar values
    archive_scalar_stats(report);

    //! the plan
    Value the_plan(kArrayType);
    {
        const auto& actions = Problem::getInstance().getGroundActions();
    	for (const auto& action_id:plan) {
            Value name;
            name.SetString(StringRef(actions.at(action_id)->getName().c_str()));
            the_plan.PushBack( name.Move(), allocator );
    	}
    }
    report.AddMember("plan", the_plan, allocator);

    JSONArchive::write_to_file( filename, report );
}

void
EmbeddedDriver::archive_scalar_stats( rapidjson::Document& doc ) {
    using namespace rapidjson;
    Document::AllocatorType& allocator = doc.GetAllocator();
    //! Result
    Value result_string;
    result_string.SetString(StringRef(exit_code_string( result )));
    doc.AddMember( "result", result_string.Move(), allocator );
    doc.AddMember( "solved", Value(solved).Move(), allocator);
    doc.AddMember( "oom", Value(oom).Move(), allocator);
    doc.AddMember( "search_time", Value(search_time).Move(), allocator);
    doc.AddMember( "total_planning_time", Value(total_planning_time).Move(), allocator);
    doc.AddMember( "valid", Value(valid).Move(), allocator);
    doc.AddMember( "gen_speed", Value(gen_speed).Move(), allocator);
    doc.AddMember( "eval_speed", Value(eval_speed).Move(), allocator);
    doc.AddMember( "peak_memory", Value(peak_memory).Move(), allocator);
}

}}
