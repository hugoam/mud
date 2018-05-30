//  Copyright (c) 2018 Hugo Amiard hugo.amiard@laposte.net
//  This software is provided 'as-is' under the zlib License, see the LICENSE.txt file.
//  This notice and the license may not be removed or altered from any source distribution.

#pragma once

#include <math/Generated/Forward.h>

#include <cfloat>
#include <limits>

namespace mud
{
	namespace detail
	{
		template <class T>
		inline T min(T a, T b) { return (b < a) ? b : a; }

		template <class T>
		inline T max(T a, T b) { return (a < b) ? b : a; }
	}

	template <class T>
	class StatDef
	{
	public:
		StatDef(T min = std::numeric_limits<T>::lowest(), T max = std::numeric_limits<T>::max(), T step = T(1))
			: m_min(min)
			, m_max(max)
			, m_step(step)
		{}

		T m_min;
		T m_max;
		T m_step;

		T rincrement(T& value, T amount) const { T diff = detail::min(m_max - value, amount); value += diff; update(value); return diff; }
		T rdecrement(T& value, T amount) const { T diff = detail::max(-m_min + value, amount); value -= diff; update(value); return diff; }
		void increment(T& value, T amount) const { value += amount; update(value); }
		void decrement(T& value, T amount) const { value -= amount; update(value); }

		void increment(T& value) const { value += m_step; update(value); }
		void decrement(T& value) const { value -= m_step; update(value); }

		void multiply(T& value, T& base, T multiplier) const { T diff = value - base; base *= multiplier; value = base + diff; update(value); }
		
		void modify(T& value, T& base, T val) const { value += val - base; base = value; update(value); }
		void modify(T& value, T val) const { value = val; update(value); }

		void update(T& value) const
		{
			if(value < m_min)
				value = m_min;
			if(value > m_max)
				value = m_max;
		}
	};

	template <class T>
	class Stat
	{
	public:
		Stat(T base = T())
			: m_base(base)
			, m_value(base)
			//, m_updateHandlers()
		{}

		operator T() const { return m_value; }

		inline T base() const { return m_base; }
		inline T value() const { return m_value; }

		virtual const StatDef<T>& vdef() const = 0;

	protected:
		T m_base;
		T m_value;
	};

	template <class T, class T_Def>
	class DefStat : public Stat<T>
	{
	public:
		DefStat(T base = T())
			: Stat<T>(base)
		{}

		const T_Def* self() const { return static_cast<const T_Def*>(this); }

		const StatDef<T>& vdef() const { return self()->def(); }

		inline T min() const { return self()->def().m_min; }
		inline T max() const { return self()->def().m_max; }
		inline T step() const { return self()->def().m_step; }

		inline void modify(T value) { self()->def().modify(this->m_value, this->m_base, value); }

		inline T rincrement(T amount) { return self()->def().rincrement(this->m_value, amount); }
		inline T rdecrement(T amount) { return self()->def().rdecrement(this->m_value, amount); }
		inline void increment(T amount) { self()->def().increment(this->m_value, amount); }
		inline void decrement(T amount) { self()->def().decrement(this->m_value, amount); }

		inline void increment() { self()->def().increment(this->m_value); }
		inline void decrement() { self()->def().decrement(this->m_value); }

		inline void multiply(T multiplier) { self()->def().multiply(this->m_value, this->m_base, multiplier); }
	};

	template <class T>
	class _refl_ AutoStat
	{
	public:
		AutoStat(T value = T(), T min = std::numeric_limits<T>::lowest(), T max = std::numeric_limits<T>::max(), T step = T(1))
			: m_value(value)
			, m_valueRef(m_value)
			, m_def(min, max, step)
		{}

		AutoStat(T& value, StatDef<T> def)
			: m_value()
			, m_valueRef(value)
			, m_def(def)
		{}

		AutoStat(const AutoStat& other)
			: m_value(other.m_value)
			, m_valueRef(&other.m_value == &other.m_valueRef ? m_value : other.m_valueRef)
			, m_def(other.m_def)
		{}

		AutoStat& operator=(const AutoStat& other)
		{
			m_value = other.m_value;
			m_valueRef = other.m_valueRef;
			m_def = other.m_def;
			return *this;
		}

		operator T() const { return m_valueRef; }

		inline T value() const { return m_valueRef; }
		inline T& ref() const { return m_valueRef; }

		inline T min() const { return m_def.m_min; }
		inline T max() const { return m_def.m_max; }
		inline T step() const { return m_def.m_step; }

		inline void modify(T value) { m_def.modify(m_valueRef, value); }

		inline void increment() { m_def.increment(m_valueRef); }
		inline void decrement() { m_def.decrement(m_valueRef); }

		const StatDef<T>& vdef() const { return m_def; }

	protected:
		T m_value;
		T& m_valueRef;
		StatDef<T> m_def;
	};

	template class _refl_ MUD_MATH_EXPORT AutoStat<int>;
	template class _refl_ MUD_MATH_EXPORT AutoStat<float>;

	struct _refl_ MUD_MATH_EXPORT Ratio : public DefStat<float, Ratio>
	{
	public:
		_constr_ Ratio(float value = 0.f) : DefStat<float, Ratio>(value) {}
		Ratio(const Ratio&) = default;
		Ratio& operator=(const Ratio&) = default;

		_attr_ _mut_ float value() const { return DefStat<float, Ratio>::value(); }
		void setValue(float value) { DefStat<float, Ratio>::modify(value); }

		const StatDef<float>& def() const { static StatDef<float> df(0.f, 1.f, 0.01f); return df; }
	};

	struct _refl_ MUD_MATH_EXPORT Gauge : public DefStat<float, Gauge>
	{
	public:
		_constr_ Gauge(float value = 0.f) : DefStat<float, Gauge>(value) {}
		Gauge(const Gauge&) = default;
		Gauge& operator=(const Gauge&) = default;

		_attr_ _mut_ float value() const { return DefStat<float, Gauge>::value(); }
		void setValue(float value) { DefStat<float, Gauge>::modify(value); }

		const StatDef<float>& def() const { static StatDef<float> df(0.f, FLT_MAX, 1.f); return df; }
	};

#if 0
	template <class T>
	struct StringConverter<AutoStat<T>>
	{
		static inline void to(const AutoStat<T>& stat, string& str) { to_string<T>(stat.value(), str); }
		static inline void from(const string& str, AutoStat<T>& stat) { stat.modify(from_string<T>(str)); }
	};
#endif

}
