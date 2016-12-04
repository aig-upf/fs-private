
#pragma once

namespace fs0 { 

class SearchStats {
public:
	SearchStats() : _expanded(0), _generated(0) {}
	
	void expansion() { _expanded++; }
	void generation() { _generated++; }

	unsigned long expanded() const { return _expanded; }
	unsigned long generated() const { return _generated; }
	
protected:
	unsigned long _expanded;
	unsigned long _generated;
};

} // namespaces
