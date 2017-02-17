
#include "base.hxx"

#include <ostream>

namespace lapkt { namespace novelty {


std::ostream& operator<<(std::ostream& os, const Novelty& nov) {
	if (nov == Novelty::Unknown) os << "=?";
	else if (nov == Novelty::One) os << "=1";
	else if (nov == Novelty::OneAndAHalf) os << "=1.5";
	else if (nov == Novelty::GTOne) os << ">1";
	else if (nov == Novelty::Two) os << "=2";
	else if (nov == Novelty::GTTwo) os << ">2";
	else throw std::runtime_error("Unknown novelty value");	
	return os;
}

} } // namespaces