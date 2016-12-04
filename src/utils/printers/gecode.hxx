
#pragma once

#include <fs_types.hxx>
#include <gecode/int.hh>
#include <ostream>


namespace fs0 { namespace gecode { 
	class CSPTranslator; class GecodeCSP;
}}

namespace fs0 { namespace print {

//! Prints the full extension of a Gecode tupleset
class tupleset {
	protected:
		const Gecode::TupleSet& _tupleset;
	public:
		tupleset(const Gecode::TupleSet& tset) : _tupleset(tset) {}
		
		friend std::ostream& operator<<(std::ostream &os, const tupleset& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};

//! Prints a Gecode variable set along with its currently assigned domains
class varset {
	protected:
		const Gecode::IntVarArgs& _vars;
	public:
		varset(const Gecode::IntVarArgs& vars) : _vars(vars) {}
		
		friend std::ostream& operator<<(std::ostream &os, const varset& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};

class extensional {
	protected:
		const Gecode::IntVarArgs& _vars;
		const Gecode::TupleSet& _tupleset;
		
	public:
		extensional(const Gecode::IntVarArgs& vars, const Gecode::TupleSet& tset) : _vars(vars), _tupleset(tset) {}
		
		friend std::ostream& operator<<(std::ostream &os, const extensional& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};

class element {
	protected:
		const Gecode::IntVarArgs& _table;
		const Gecode::IntVar& _index;
		const Gecode::IntVar& _output;
		
	public:
		element(const Gecode::IntVarArgs& table, const Gecode::IntVar& index, const Gecode::IntVar& output) : _table(table), _index(index), _output(output) {}
		
		friend std::ostream& operator<<(std::ostream &os, const element& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};

class csp {
	protected:
		const gecode::CSPTranslator& _translator;
		const gecode::GecodeCSP& _csp;
		
	public:
		csp(const gecode::CSPTranslator& translator, const gecode::GecodeCSP& csp) : _translator(translator), _csp(csp) {}
		
		friend std::ostream& operator<<(std::ostream &os, const csp& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};

} } // namespaces
