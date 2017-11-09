

#pragma once

#define BOOST_DYNAMIC_BITSET_DONT_USE_FRIENDS // see http://stackoverflow.com/a/3897217
#include <boost/dynamic_bitset.hpp>
#include <boost/functional/hash_fwd.hpp>

namespace boost {
template <typename B, typename A>
std::size_t hash_value(const boost::dynamic_bitset<B, A>& bs) {
	std::size_t seed = 0;
	boost::hash_combine(seed, boost::hash_value(bs.m_bits));
	boost::hash_combine(seed, boost::hash_value(bs.m_num_bits));
	return seed;
}
} // namespace
