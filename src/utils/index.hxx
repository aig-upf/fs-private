
#pragma once

#include <vector>
#include <map>

namespace fs0 {

//! A double index of objects
template <typename T>
class Index {
protected:
	std::vector<T> _elements;
	std::map<T, unsigned> _index;
	
public:
	Index() {}
	
	void add(const T& element) {
		auto it = _index.find(element);
		if (it != _index.end()) throw std::runtime_error("Object already indexed");
		_index.insert(it, std::make_pair(element, _elements.size())); // Order of insertion matters
		_elements.push_back(element);
	}
	
	unsigned index(const T& element) const {
		auto it = _index.find(element);
		if (it == _index.end()) throw std::runtime_error("Object not found");
		return it->second;
	}
	
	std::size_t size() const { return _elements.size(); }
	
	const std::vector<T>& elements() const { return _elements; }
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const Index& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
};

} // namespaces


