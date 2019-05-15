
#pragma once

#include <fs/core/fstrips/fol.hxx>
#include <fs/core/fstrips/language.hxx>
#include <fs/core/fstrips/interpretation.hxx>


namespace fs0 { namespace fstrips {

class LanguageInfo;

//!
class Problem {
public:
    Problem(const std::string& name, const std::string& domain_name,
            const std::vector<const ActionSchema*>& schemas, const Interpretation& init, const Formula* goal)
        :
            name_(name),
            domain_name_(domain_name),
            schemas_(schemas),
			init_(init),
            goal_(goal)
    {}
	~Problem() {
        for (auto s:schemas_) delete s;
        delete goal_;
    };
	
	Problem(const Problem&);
	Problem(Problem&&) = default;
	Problem& operator=(const Problem&) = delete;
	Problem& operator=(Problem&&) = delete;
	
	Problem* clone() const { return new Problem(*this); }
	
	//! Prints a representation of the object to the given stream.
//	friend std::ostream& operator<<(std::ostream &os, const Problem& o);
	std::ostream& print(std::ostream& os, const LanguageInfo& info) const;

	//! Accessors
	const std::string& name() const { return name_; }
	const std::string& domain_name() const { return domain_name_; }
	const std::vector<const ActionSchema*> schemas() const { return schemas_; }
	const Interpretation& init() const { return init_; }
	const Formula* goal() const { return goal_; }
	
protected:
    const std::string name_;
    const std::string domain_name_;
    const std::vector<const ActionSchema*> schemas_;
	const Interpretation init_;
    const Formula* goal_;
};


} } // namespaces
