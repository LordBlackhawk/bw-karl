#pragma once
#include <BWAPI.h>
#include <vector>
namespace BWTA
{
  class Polygon
  {
	public:
		typedef std::vector<BWAPI::Position>::iterator				iterator;
		typedef std::vector<BWAPI::Position>::const_iterator		const_iterator;
		typedef std::vector<BWAPI::Position>::reference				reference;
		typedef std::vector<BWAPI::Position>::const_reference		const_reference;
		typedef std::vector<BWAPI::Position>::size_type				size_type;
		typedef std::vector<BWAPI::Position>::value_type			value_type;
		
    public:
      Polygon();
      Polygon(const Polygon& b);
      ~Polygon();
      double getArea() const;
      double getPerimeter() const;
      BWAPI::Position getCenter() const;
      bool isInside(const BWAPI::Position& p) const;
      BWAPI::Position getNearestPoint(BWAPI::Position p) const;
      const std::vector<Polygon>& getHoles() const;
	  
	  iterator begin ()
	  {
		return points.begin();
	  }
	  
	  const_iterator begin () const
	  {
		return points.begin();
	  }
	  
	  iterator end ()
	  {
		return points.end();
	  }
	  
	  const_iterator end () const
	  {
		return points.end();
	  }
	  
	  size_type size() const
	  {
		return points.size();
	  }
	  
	  reference operator[] ( size_type n )
	  {
		return points[n];
	  }
	  
	  const_reference operator[] ( size_type n ) const
	  {
		return points[n];
	  }
	  
	  reference front()
	  {
		return points.front();
	  }
	  
      const_reference front() const
	  {
	    return points.front();
	  }
	  
	  reference back()
	  {
		return points.back();
	  }
	  
      const_reference back() const
	  {
	    return points.back();
	  }
	  
	  void push_back ( const value_type& x )
	  {
		points.push_back(x);
	  }
	  
	  iterator erase ( iterator position )
	  {
		return points.erase(position);
	  }
	  
	  iterator erase ( iterator first, iterator last )
	  {
		return points.erase(first, last);
	  }

	//private:
      struct PolygonImpl;

      std::vector<BWAPI::Position>  points;
      std::vector<Polygon>          holes;
      mutable PolygonImpl*          impl;
  };
}