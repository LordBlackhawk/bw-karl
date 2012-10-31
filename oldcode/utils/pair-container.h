#pragma once

#include <boost/operators.hpp>
#include <algorithm>

template <class Container>
class PairContainer
{
	protected:
		typedef typename Container::iterator iterator_internal;
		typedef std::pair<iterator_internal, iterator_internal> PairType;

	public:
		class iterator : public boost::forward_iterator_helper<iterator, PairType, std::ptrdiff_t, const PairType*, const PairType&>
		{
			friend class PairContainer;

			public:
				iterator& operator ++ ()
				{
					values.first = values.second;
					++values.second;
					return *this;
				}

				bool operator < (const iterator& other) const
				{
					return (values.first < other.values.first);
				}

				bool operator == (const iterator& other) const
				{
					return (values.first == other.values.first);
				}

				const PairType& operator * () const
				{
					return values;
				}

			protected:
				PairType values;

				iterator(const iterator_internal& i) : values(std::make_pair(i,i))
				{
					++(*this);
				}
		};

		typedef typename Container::size_type size_type;

	public:
		PairContainer(Container c) : container(c)
		{ }
		
		iterator begin()
		{
			return iterator(container.begin());
		}

		iterator end()
		{
			if (container.size() < 1)
				return iterator(container.end());
			else
				return iterator(container.end()-1);
		}

		size_type size() const
		{
			return (container.size() > 1) ? container.size() - 1 : 0;
		}

	protected:
		Container container;
};

template <class Container>
PairContainer<Container> pairsOf(Container container)
{
	return PairContainer<Container>(container);
}