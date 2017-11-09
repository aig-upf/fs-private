
#include <fs/core/utils/printers/gecode.hxx>
#include <fs/core/constraints//gecode/csp_translator.hxx>

namespace fs0 { namespace print {

std::ostream& tupleset::print(std::ostream& os) const {
	unsigned arity = _tupleset.arity();
	for (unsigned i = 0; i < (unsigned)_tupleset.tuples(); ++i) {
		os << "<";
		for (unsigned j = 0; j < arity; ++j) {
			os << _tupleset[i][j];
			if (j < arity-1) os << ", ";
		}
		os << ">" << std::endl;
	}
	return os;
}

std::ostream& varset::print(std::ostream& os) const {
	unsigned i = 0;
	for (auto var:_vars) os << "X" << i++ << " \\in " << var << std::endl;
	return os;
}

std::ostream& extensional::print(std::ostream& os) const {
	os << "Gecode::extensional(variables, tuples):" << std::endl;
	os << "Variables:" << std::endl << print::varset(_vars) << std::endl;
	os << "Tuples:" << std::endl << print::tupleset(_tupleset) << std::endl;
	return os;
}

std::ostream& element::print(std::ostream& os) const {
	os << "Gecode::element(table, index, output):" << std::endl;
	os << "index \\in " << _index << std::endl;
	os << "output \\in " << _output << std::endl;
	os << "table = {" << std::endl;
	for (auto var:_table) os << var << std::endl;
	os << "}" << std::endl;
	return os;
}

std::ostream& csp::print(std::ostream& os) const {
	return _translator.print(os, _csp);
}

} } // namespaces
