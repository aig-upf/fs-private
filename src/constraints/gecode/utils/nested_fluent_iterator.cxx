
#include <constraints/gecode/utils/nested_fluent_iterator.hxx>
#include <problem.hxx>
#include <constraints/gecode/utils/term_list_iterator.hxx>
#include <languages/fstrips/terms.hxx>

namespace fs0 { namespace gecode {

nested_fluent_iterator::nested_fluent_iterator(const fs::NestedTerm* term) :
	_term(term),
	_iterator(new term_list_iterator(term->getSubterms())),
	_index(0)
{}

nested_fluent_iterator::~nested_fluent_iterator() {
	delete _iterator;
}

const nested_fluent_iterator& nested_fluent_iterator::operator++() {
	++(*_iterator);
	++_index;
	return *this;
}
const nested_fluent_iterator nested_fluent_iterator::operator++(int) { nested_fluent_iterator tmp(*this); operator++(); return tmp; }

bool nested_fluent_iterator::ended() const { return _iterator->ended(); }

unsigned nested_fluent_iterator::getIndex() const { return _index; }

VariableIdx nested_fluent_iterator::getDerivedStateVariable() const {
	const ProblemInfo& info = Problem::getInfo();
	return info.resolveStateVariable(_term->getSymbolId(), _iterator->arguments());
}

Gecode::IntArgs nested_fluent_iterator::getIndexedIntArgsElement() const {
	return _iterator->getIntArgsElement(getIndex());
}

} } // namespaces
