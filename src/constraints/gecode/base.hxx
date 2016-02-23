
#pragma once

#include <gecode/int.hh>
#include <problem.hxx>
#include <utils/logging.hxx>

namespace fs0 { namespace gecode {

class DONT_CARE {
protected:
	static int _value;

public:
	static void set(int value) { _value = value; }
	static int get() { return _value; }
};

	
} } // namespaces