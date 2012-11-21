#pragma once

#include <vector>
#include <algorithm>

template <class T>
class ParallelVector
{
    // This implementation of a parallel vector can be used if one thread add continuously
    // items and another thread use the items. Synchronization is done, when update/clear is called.
    // User has to take care that during update call no other methods are used.
    public:
        typedef typename std::vector<T>::iterator           iterator;
        typedef typename std::vector<T>::const_iterator     const_iterator;
        typedef typename std::vector<T>::size_type          size_type;

    protected:
        std::vector<T>      items;
        std::vector<T>      newitems;

    public:
        ParallelVector()
        { }
        
        void update()
        {
            if (!newitems.empty()) {
                items.insert(items.end(), newitems.begin(), newitems.end());
                newitems.clear();
            }
        }
        
        void clear()
        {
            items.clear();
            newitems.clear();
        }

        size_type size() const
        {
            return items.size();
        }
        
        iterator begin()
        {
            return items.begin();
        }
        
        const_iterator begin() const
        {
            return items.begin();
        }
        
        iterator end()
        {
            return items.end();
        }
        
        const_iterator end() const
        {
            return items.end();
        }
        
        void push_back(const T& item)
        {
            newitems.push_back(item);
        }
        
        T& operator [] (int index)
        {
            return items[index];
        }
        
        const T& operator [] (int index) const
        {
            return items[index];
        }
        
        void erase(iterator first, iterator last)
        {
            items.erase(first, last);
        }
};

namespace Containers
{
    template <class T>
    void add(ParallelVector<T>& container, const T& element)
    {
        container.push_back(element);
    }
    
    template <class T>
    void remove(ParallelVector<T>& container, const T& element)
    {
        container.erase(std::remove(container.begin(), container.end(), element), container.end());
    }
    
    template <class T, class F>
    void remove_if(ParallelVector<T>& container, F f)
    {
        container.erase(std::remove_if(container.begin(), container.end(), f), container.end());
    }
}

