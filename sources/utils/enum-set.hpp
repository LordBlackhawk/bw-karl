#pragma once

#include <initializer_list>
#include "assert-throw.hpp"

template <class T> class EnumSet;

template <class T>
class EnumIterator : public std::iterator<std::input_iterator_tag, T>
{
    public:
        EnumIterator(const EnumIterator<T>& other)
            : bitmask(other.bitmask), current(other.current)
        { }

        EnumIterator<T>& operator ++ ()
        {
            ++current;
            nextValid();
            return *this;
        }

        EnumIterator<T> operator ++ (int)
        {
            EnumIterator<T> tmp(*this);
            operator++();
            return tmp;
        }

        bool operator == (const EnumIterator<T>& other)
        {
            return current == other.current;
        }

        bool operator != (const EnumIterator<T>& other)
        {
            return current != other.current;
        }

        T operator*()
        {
            return (T)current;
        }

    protected:
        friend class EnumSet<T>;

        unsigned int bitmask;
        unsigned int current;

        EnumIterator(unsigned int b, unsigned int c)
            : bitmask(b), current(c)
        {
            nextValid();
        }

        void nextValid()
        {
            while ((current < (unsigned int)T::lastElement) && ((bitmask & (1U << current)) == 0U))
                ++current;
        }
};

template <class T>
class EnumSet
{
    public:
        typedef T BaseType;

        EnumSet(std::initializer_list<BaseType> list)
            : bitmask(0U)
        {
            for (auto it : list)
                set(it);
        }

        EnumSet()
            : bitmask(0U)
        { }

        static EnumSet<T> all()
        {
            return EnumSet<T>(lift(T::lastElement) - 1U);
        }

        EnumIterator<T> begin() const
        {
            return EnumIterator<T>(bitmask, 0U);
        }

        EnumIterator<T> end() const
        {
            return EnumIterator<T>(bitmask, (unsigned int)T::lastElement);
        }

        bool contains(BaseType x) const
        {
            return (bitmask & lift(x)) != 0U;
        }

        bool empty() const
        {
            return bitmask == 0U;
        }

        void set(BaseType x)
        {
            bitmask |= lift(x);
        }

        void clear(BaseType x)
        {
            bitmask &= ~lift(x);
        }

        EnumSet<T> inverse() const
        {
            return EnumSet<T>(all().bitmask & ~bitmask);
        }

    protected:
        unsigned int bitmask;

        EnumSet(unsigned int b)
            : bitmask(b)
        { }

        static unsigned int lift(BaseType x)
        {
            return 1U << (unsigned int)x;
        }
};

template <class E>
struct CastLastElement
{
    enum { lastElement = (unsigned int) E::lastElement };
};

template <class T, class E>
class EnumArray : public std::array<T, CastLastElement<E>::lastElement>
{
    public:
        typedef E                                               BaseType;
        typedef std::array<T, CastLastElement<E>::lastElement>  ParentClass;

        EnumArray()
        { }

        EnumArray(std::initializer_list<T> list)
        {
            assert(ParentClass::size() == list.size());
            unsigned int k = 0U;
            for (auto it : list)
                ParentClass::operator [] (k++) = it;
        }

        typename ParentClass::reference operator [] (BaseType index)
        {
            return ParentClass::operator [] ((unsigned int)index);
        }

        typename ParentClass::const_reference operator [] (BaseType index) const
        {
            return ParentClass::operator [] ((unsigned int)index);
        }
};
