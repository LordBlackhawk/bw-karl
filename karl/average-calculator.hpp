#pragma once

#include <algorithm>

template <int N, class CT, class RT>
class AverageCalculator
{
    protected:
        CT data[N];
        CT sum;
        int n;
        bool full;
        RT factor;
        CT min_;
        CT max_;
    
    public:
        AverageCalculator()
            : sum(0), n(0), full(false), factor(1.0), min_(0), max_(0)
        {
            for (int k=0; k<N; ++k)
                data[k] = 0;
        }
        
        void setFactor(const RT& f)
        {
            factor = f;
        }
        
        void push(const CT& value)
        {
            if ((n == 0) && !full) {
                min_ = value;
                max_ = value;
            } else {
                min_ = std::min(min_, value);
                max_ = std::max(max_, value);
            }
            sum += value - data[n];
            data[n] = value;
            ++n;
            if (n >= N) {
                n    = 0;
                full = true;
            }
        }
        
        RT average() const
        {
            if (full)
                return factor * RT(sum) / RT(N);
            else
                return factor * RT(sum) / RT(n);
        }
        
        RT min() const
        {
            return factor * RT(min_);
        }
        
        RT max() const
        {
            return factor * RT(max_);
        }
};
