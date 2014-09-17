#pragma once

#include "assert-throw.hpp"
#include <BWAPI.h>
#include <vector>

template <class T>
class Array2d
{
    typedef Array2d<T> ThisType;

    public:
        class Row
        {
            friend class Array2d<T>;
            public:
                typename std::vector<T>::reference operator [] (int index)
                {
                    assert( (0 <= index) && (index < size) );
                    return owner.elements[offset + index * factor];
                }

            protected:
                Row(ThisType& o, int off, int fac, int s) : owner(o), offset(off), factor(fac), size(s)
                { }

                ThisType& owner;
                int offset, factor, size;
        };

        class ConstRow
        {
            friend class Array2d<T>;
            public:
                T operator [] (int index) const
                {
                    assert( (0 <= index) && (index < size) );
                    return owner.elements[offset + index * factor];
                }

            protected:
                ConstRow(const ThisType& o, int off, int fac, int s) : owner(o), offset(off), factor(fac), size(s)
                { }

                const ThisType& owner;
                int offset, factor, size;
        };

        Array2d() : sizex(0), sizey(0)
        { }

        Array2d(int sx, int sy, const T& t = T()) : sizex(sx), sizey(sy), elements(sizex*sizey, t)
        { }

        void resize(int sx, int sy, const T& t = T())
        {
            sizex    = sx;
            sizey    = sy;
            elements = std::vector<T>(sizex*sizey, t);
        }

        Row operator [] (int index)
        {
            assert( (0 <= index) && (index < sizex) );
            return Row(*this, index*sizey, 1, sizey);
        }

        ConstRow operator [] (int index) const
        {
            assert( (0 <= index) && (index < sizex) );
            return ConstRow(*this, index*sizey, 1, sizey);
        }

        T& operator [] (const BWAPI::TilePosition& pos)
        {
            return (*this)[pos.x()][pos.y()];
        }

        T operator [] (const BWAPI::TilePosition& pos) const
        {
            return (*this)[pos.x()][pos.y()];
        }

        bool isValid(const BWAPI::TilePosition& pos) const
        {
            return (0 <= pos.x()) && (pos.x() < sizex) && (0 <= pos.y()) && (pos.y() < sizey);
        }

        int getWidth() const
        {
            return sizex;
        }

        int getHeight() const
        {
            return sizey;
        }

    protected:
        int sizex, sizey;
        std::vector<T> elements;
};