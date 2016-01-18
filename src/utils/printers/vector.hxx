
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
		const std::string _separator = ", ";
		
		PrinterT _printer;
		 
	public:
		_container(InputIt first, InputIt last) : _first(first), _last(last) {}
		_container(InputIt first, InputIt last, PrinterT printer) : _first(first), _last(last), _printer(printer) {}
		_container(InputIt first, InputIt last, const std::string& separator) : _first(first), _last(last), _separator(separator) {}
		
		friend std::ostream& operator<<(std::ostream &os, const _container& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const {
			os << "{";
			for (InputIt it(_first); it != _last;) {
				if (_printer) _printer(*it, os);
				else os << *it;
				
				++it;
				
				if (it != _last) os << _separator;
			}
			os << "}";
			return os;
		}
};

//! A helper to allow for automatic template argument deduction, both with and without printer function
template <typename Container, typename R = _container<typename Container::const_iterator>>
const R container(const Container& object, typename R::PrinterT printer) {
	return R(object.cbegin(), object.cend(), printer);
}

template <typename Container, typename R = _container<typename Container::const_iterator>>
const R container(const Container& object, const std::string separator = ", ") {
	return R(object.cbegin(), object.cend(), separator);
}

} } // namespaces
