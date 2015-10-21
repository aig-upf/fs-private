
#pragma once

#include <vector>
#include <ostream>

namespace fs0 { namespace print {

template<class InputIt>
class _container {
	public:
		//! The type of values we're printing
		typedef typename std::iterator_traits<InputIt>::value_type ValT;
		//! The type of a printer function
		typedef std::function<void(const ValT& element, std::ostream& os)> PrinterT;
		
	protected:
		InputIt _first;
		InputIt _last;
		
		PrinterT _printer;
		 
	public:
		_container(InputIt first, InputIt last) : _first(first), _last(last) {}
		_container(InputIt first, InputIt last, PrinterT printer) : _first(first), _last(last), _printer(printer) {}
		
		friend std::ostream& operator<<(std::ostream &os, const _container& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const {
			os << "{";
			for (InputIt it(_first); it != _last;) {
				if (_printer) _printer(*it, os);
				else os << *it;
				
				++it;
				
				if (it != _last) os << ", ";
			}
			os << "}";
			return os;
		}
};

//! A helper to allow for automatic template argument deduction
template <typename Container, typename R = _container<typename Container::const_iterator>>
const R container(const Container& object, typename R::PrinterT printer = [](const typename R::ValT& t, std::ostream& os) { os << t; }) {
	return R(object.cbegin(), object.cend(), printer);
}

} } // namespaces
