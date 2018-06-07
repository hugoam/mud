//  Copyright (c) 2018 Hugo Amiard hugo.amiard@laposte.net
//  This software is provided 'as-is' under the zlib License, see the LICENSE.txt file.
//  This notice and the license may not be removed or altered from any source distribution.

#pragma once

#include <obj/Generated/Forward.h>

#ifndef MUD_CPP_20
#include <cstdint>
#include <cstring>
#include <cassert>
#endif

namespace mud
{
	using cstring = const char*;

	export_ template <class T, size_t Size>
	struct carray
	{
		template <class... T_Args>
		carray(T_Args... args) : m_array{ T(args)... } {}
		T m_array[Size];
		size_t size() { return Size; }
		T* data() { return m_array; }
		T& operator[](size_t at) { return m_array[at]; }
		const T& operator[](size_t at) const { return m_array[at]; }
		operator array<T>() { return{ m_array, Size }; }
	};

	template <class... T_Args, size_t size = sizeof...(T_Args)>
	carray<cstring, size> cstrarray(T_Args... args) { return carray<cstring, size>{ args... }; }

	template <class... T_Args, size_t size = sizeof...(T_Args)>
	carray<cstring, size> strarray(T_Args... args) { return carray<cstring, size>{ args.c_str()... }; }

	export_ template <typename T>
	struct refl_ struct_ array
	{
	public:
		array() : m_pointer(nullptr), m_count(0) {}
		array(T* pointer, size_t count) : m_pointer(pointer), m_count(count) {}
		array(array<T> other, size_t offset) : m_pointer(other.m_pointer + offset), m_count(other.m_count - offset) {}
		array(array<T> other, size_t offset, size_t count) : m_pointer(other.m_pointer + offset), m_count(count) {}
		template <class U>
		array(U& container) : m_pointer(container.data()), m_count(container.size()) {}

		size_t size() { return m_count; }
		T* data() const { return m_pointer; }

		T& operator[](size_t at) { assert(at < m_count); return m_pointer[at]; }
		const T& operator[](size_t at) const { assert(at < m_count); return m_pointer[at]; }

		T* begin() { return m_pointer; }
		T* end() { return m_pointer + m_count; }
		const T* begin() const { return m_pointer; }
		const T* end() const { return m_pointer + m_count; }

		void copy(array<T>& dest) { memcpy(dest.m_pointer, m_pointer, sizeof(T) * m_count); }

		T* m_pointer;
		size_t m_count;
	};

	export_ template struct refl_ struct_ MUD_OBJ_EXPORT array<float>;
	export_ template struct refl_ struct_ MUD_OBJ_EXPORT array<cstring>;

	export_ template<typename T> class array_2d : public array<T>
	{
	public:
		array_2d(T* pointer, size_t size_x, size_t size_y) : array<T>(pointer, size_x * size_y), m_size_x(size_x), m_size_y(size_y) {}
		size_t m_size_x;
		size_t m_size_y;
	};
}
