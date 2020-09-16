#pragma once
#include <infra/Config.h>

#ifdef USE_STL
#ifndef TWO_MODULES
#include <map>
#endif
namespace stl
{
	using std::map;
}
#else
#include <stl/unordered_map.h>
namespace stl
{
	template <class K, class T>
	using map = stl::unordered_map<K, T>;
}
#endif

namespace two
{
	export_ using stl::map;
}
