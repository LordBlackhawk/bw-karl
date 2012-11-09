#pragma once

template <class T, int N>
class CircleBuffer
{
    protected:
        T buffer[N];
        int index;
    
    public:
        explicit CircleBuffer(const T& t = T())
            : index(0)
        {
            for (int i=0; i<N; ++i)
                buffer[i] = t;
        }
        
        T get() const
        {
            return buffer[index];
        }
        
        void add(const T& t)
        {
            buffer[index] = t;
            index = (index + 1) % N;
        }
};
