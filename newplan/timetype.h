#pragma once

#include <algorithm>

typedef int TimeType;

class TimeInterval
{
	public:
		typedef TimeType		ctype;
		typedef TimeInterval	ThisType;

	public:
		TimeInterval() : lower(1), upper(0)
		{ }

		TimeInterval(ctype b) : lower(0), upper(b)
		{ }

		TimeInterval(ctype a, ctype b) : lower(a), upper(b)
		{ }

		bool empty() const
		{
			return (upper < lower);
		}

		bool contains(const ctype& value) const
		{
			return (lower <= value) && (value < upper);
		}
		
		bool contains(const ctype& value, bool pushdecs) const
		{
			if (pushdecs) {
				if ((lower == upper) && (upper == value))
					return true;
				return (lower < value) && (value <= upper);
			} else {
				return contains(value);
			}
		}

		ctype length() const
		{
			return std::max(-1, upper - lower);
		}

		ThisType& operator ^= (const ThisType& other)
		{
			lower = std::max(lower, other.lower);
			upper = std::min(upper, other.upper);
			return *this;
		}

		ThisType operator ^ (const ThisType& other) const
		{
			ThisType res(*this);
			res ^= other;
			return res;
		}

		ThisType& operator += (const ThisType& other)
		{
			lower = std::min(lower, other.lower);
			upper = std::max(upper, other.upper);
			return *this;
		}

		ThisType operator + (const ThisType& other) const
		{
			ThisType res(*this);
			res += other;
			return res;
		}

		template <class Stream>
		friend Stream& operator << (Stream& stream, const ThisType& i)
		{
			if (i.empty()) {
				stream << "{}";
			} else if (i.length() == 0) {
				stream << "{" << i.lower << "}";
			} else {
				stream << "[" << i.lower << "," << i.upper << "]";
			}
			return stream;
		}

	public:
		ctype lower, upper;
};

