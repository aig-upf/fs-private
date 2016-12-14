
#pragma once

namespace fs0 { 

class SearchStats {
public:
	SearchStats() : _expanded(0), _generated(0), _evaluated(0) {}
	
	void expansion() { ++_expanded; }
	void generation() { ++_generated; }
	void evaluation() { ++_evaluated; }

	unsigned long expanded() const { return _expanded; }
	unsigned long generated() const { return _generated; }
	unsigned long evaluated() const { return _evaluated; }
	
protected:
	unsigned long _expanded;
	unsigned long _generated;
	unsigned long _evaluated;
};

} // namespaces
