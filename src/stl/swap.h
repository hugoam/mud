#pragma once

#include <infra/Config.h>

#ifdef USE_STL
#ifndef TWO_MODULES
#include <utility>
#endif
namespace stl
{
	using std::swap;
}
#else
#include <stl/move.h>
namespace stl
{
	//namespace
	//{
		template <class T>
		inline void swap(T& t1, T& t2)
		{
			T temp = move(t1);
			t1 = move(t2);
			t2 = move(temp);
		}
	//}
}
#endif

#include <stl/move.h>
namespace two
{
#ifdef TWO_MODULES
	export_ template <class T>
	inline void swap(T& t1, T& t2)
	{
		T temp = move(t1);
		t1 = move(t2);
		t2 = move(temp);
	}
#else
	namespace
	{
		template <class T>
		inline void swap(T& t1, T& t2)
		{
			T temp = move(t1);
			t1 = move(t2);
			t2 = move(temp);
		}
	}
#endif
}

