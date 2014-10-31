/*
 *  VectorMath.h - Contains vector geometry classes.
 *  Copyright (C) 2012  C. S. Cooper, A. Mukunthan
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *  Contact Details: Cooper - andor734@gmail.com
 */

#pragma once

#include <math.h>
#include <cstdlib>
#include <cstdarg>
#include <vector>
#include <iostream>

    

#ifndef MIN
#define MIN( a, b )		( a < b ? a : b )
#endif

#ifndef MAX
#define MAX( a, b )		( a > b ? a : b )
#endif

#ifndef SPEED_OF_LIGHT
#define SPEED_OF_LIGHT	300000000
#endif

// trig constant
#define SINCOS45		0.707106781		// the square root of 0.5; cos(45) = sin(45)
#define ROOT2			1.4142136

// since i have nowhere else to put this
#define AllInVector( it, vector ) it = vector.begin(); it != vector.end(); it++


namespace VectorMath {

	/**
	 *	Contains a pair of indices. The class ensures that the first is always the lesser of the two.
	 */
	class OrderedIndexPair {

	public:
		int first;		/**< First member of the pair. */
		int second;		/**< Second member of the pair. */

		/** Default constructor */
		OrderedIndexPair( int f = 0, int s = 0 );

		/** Default destructor */
		~OrderedIndexPair();

		/** Less-Than operator overload. */
		bool operator< ( const OrderedIndexPair & ) const;

		/** logical equals operator overload. */
		bool operator== ( const OrderedIndexPair & ) const;

	};

	class Rect;
	class LineSegment;
	typedef double Real;
    
	/*
	 *	Class:		 Vector2D
	 *	Inherits:	 None
	 *	Description: This encapsulates 2D vector arithmetic.
	 */	      
	class Vector2D {
        
	public:
		Real x, y;
        
		Vector2D( Real _x=0, Real _y=0 ) : x(_x), y(_y) {  }
		Vector2D &operator+=(const Vector2D &rhs);
        
		bool operator==(const Vector2D &rhs) const;
        
		bool operator!=(const Vector2D &rhs) const;
        
		Vector2D operator+(const Vector2D &rhs) const;
        
		Vector2D operator-(const Vector2D &rhs) const;
        
		Vector2D operator*(const Real rhs) const;
                
                Real operator*(const Vector2D &rhs) const;
        
		Vector2D operator/(const Real rhs) const;
		
		/*
		 *	Function:	 Real Magnitude();
		 *	Description: Computes the magnitude of the vector.
		 */		
		Real Magnitude() const;
		
		Real MagnitudeSq() const;
		
		/*
		 *	Function:	 Vector2D Unitise();
		 *	Description: Computes the unit form of the vector.
		 */
		Vector2D Unitise() const;
		
		/*
		 *	Function:	 Real Distance( const Vector2D &v ) const;
		 *	Description: Computes the squared distance from this vector to the given vector v.
		 */
		Real DistanceSq( const Vector2D &v ) const;
		
		/*
		 *	Function:	 Real Distance( const Vector2D &v ) const;
		 *	Description: Computes the distance from this vector to the given vector v.
		 */
		Real Distance( const Vector2D &v ) const;

		/*
		 *	Function:	 Real DotProduct( const Vector2D &rhs )
		 *	Description: Computes the dot product between this vector and the given vector
		 */		
		Real DotProduct( const Vector2D &rhs ) const;

		/*
		 *	Function:	 Vector2D Reflect( LineSegment line )
		 *	Description: Reflects this vector off the given line.
		 */
		Vector2D Reflect( LineSegment line ) const;

		/*
		 *	Function:	 Real AngleBetween( Vector2D rhs ) const
		 *	Description: Computes the angle between two vectors.
		 */				
		Real AngleBetween( Vector2D rhs ) const;
	};
   

	/*
	 *	Class:		 Vector3D
	 *	Inherits:	 None
	 *	Description: This encapsulates 3D vector arithmetic.
	 */	
	class Vector3D {
        
	public:
		Real x, y, z;
        
		Vector3D( Real _x=0, Real _y=0, Real _z=0 ) : x(_x), y(_y), z(_z) {  }
		Vector3D &operator+=(const Vector2D &rhs);
        
		Vector3D &operator+=(const Vector3D &rhs);
        
		bool operator==(const Vector3D &rhs);
        
		bool operator!=(const Vector3D &rhs);
        
		Vector3D operator+(const Vector3D &rhs);
        
		Vector3D operator-(const Vector3D &rhs);
        
		Vector3D operator*(const Real rhs);

		Vector3D operator/(const Real rhs);

		/*
		 *	Function:	 Real Magnitude();
		 *	Description: Computes the magnitude of the vector.
		 */		
		Real Magnitude();
		
		/*
		 *	Function:	 Vector3D Unitise();
		 *	Description: Computes the unit form of the vector.
		 */
		Vector3D Unitise();
		
		/*
		 *	Function:	 Real DotProduct( const Vector3D &rhs )
		 *	Description: Computes the dot product between this vector and the given vector
		 */		
		Real DotProduct( const Vector3D &rhs );
        
		
		/*
		 *	Function:	 Real AngleBetween( Vector3D rhs )
		 *	Description: Computes the angle between two vectors.
		 */				
		Real AngleBetween( Vector3D rhs );

	};



	/*
	 *	Class:		 LineSegment
	 *	Inherits:	 None
	 *	Description: Contains a line between two points in space.
	 *				 Used for easy computations of directions and distances between two points
	 */
	
	class LineSegment {

	protected:
// 		bool CheckIntersection( LineSegment l2, Vector2D *pVec );
		bool CounterClockWise (Vector2D A, Vector2D B, Vector2D C);

	public:
		LineSegment( Vector2D start = Vector2D(), Vector2D end = Vector2D() ) : mStart( start ), mEnd( end ) {  }
		
		bool operator==(const LineSegment &rhs);
        
		bool operator!=(const LineSegment &rhs);
		
		Vector2D mStart, mEnd;
		
		/*
		 *	Function:	 Vector2D GetVector()
		 *	Description: Computes the free vector from the fixed vector.
		 */				
		Vector2D GetVector();
		
		
		/*
		 *	Function:	 Vector2D GetDirection()
		 *	Description: Computes the direction of the free vector.
		 */
		Vector2D GetDirection();
		
		
		/*
		 *	Function:	 Real GetDistance()
		 *	Description: Computes the distance between the start and end of the vector.
		 */				
		Real GetDistance();

		Vector2D GetNormal();
		
		/*
		 * 	Function:	bool IntersectLine( LineSegment l2, Vector2D *pVec)
		 * 	Description:	Determines if two line segments are intersecting and returns true if they are
		 * 			If argiment pVec is not NULL, it also determines the point of intersection and assigns it to pVec
		 *				This involves more computation
		 */
		bool IntersectLine( LineSegment l2, Vector2D *pVec );

		bool IntersectCircle( Vector2D c, Real r );

		bool IsBounded (LineSegment l2);

		Real DistanceFromLine( Vector2D p );

		Real DistanceAlongLine( Vector2D p );
		
		Real DistanceFromPoint(Vector2D p);
		
		bool PointInCommon( LineSegment l );

		Vector2D Midpoint();

	};

#define ForPointsOnLine( p, l, incr, _l )	Vector2D p; \
										for ( Real _l = 0; _l <= 1; _l+=incr/l.GetDistance(), p=l.mStart+l.GetVector()*_l )


	/*
	 *	Class:		 Rect
	 *	Inherits:	 None
	 *	Description: Describes a rectangle in 2D space.
	 */
	
	class Rect {

	public:
		Rect( Real x=0, Real y=0, Real w=0, Real h=0 ) : location(x, y), size(w, h) {  }
		Rect( Vector2D loc, Vector2D dim ) : location( loc ), size( dim ) {  }
		Rect( LineSegment &l ) {
			location = Vector2D(MIN(l.mStart.x,l.mEnd.x),MIN(l.mStart.y,l.mEnd.y));
			size = Vector2D(MAX(l.mStart.x,l.mEnd.x),MAX(l.mStart.y,l.mEnd.y)) - location;
		}

		Vector2D location;
		Vector2D size;
		
		/*
		 *	Function:	 Rect Scale( Real factor, Vector2D vecPoint )
		 *	Description: Computes a new rectangle that has been scaled by the given factor
		 *				 about the given point within the vector.
		 */				
		Rect Scale( Real factor, Vector2D vecPoint );
		

		/*
		 *	Function:	 Vector2D Centre( bool Absolute );
		 *	Description: Computes the centre point of the rectangle.
		 *				 If Absolute=true, the centre point is given in screen coordinates.
		 */				
		Vector2D Centre( bool Absolute = false );

		
		/*
		 *	Function:	 bool PointWithin( Vector2D point );
		 *	Description: Determines if the given point is within the rect.
		 */		
		bool PointWithin( Vector2D point );
		
		/*
		 *	Function:	 bool IntersectsRect( Rect rect );
		 *	Description: Determines if this rect and the given rect intersect.
		 */
		bool IntersectsRect( Rect rect );
		
		/*
		 * 	Function:	bool LineSegmentWithin (LineSegment line);
		 * 	Description: Determines if any part of this line segment is within the given rectangle
		 */
		bool LineSegmentWithin( LineSegment line);

	};



	/*
	 *	Class:		 Matrix2D
	 *	Inherits:	 None
	 *	Description: Encapsulates 2D matrix arithmetic
	 */
	
	class Matrix2D {
	
	public:
		Matrix2D() : m11(0), m21(0), m12(0), m22(0) {  }
		Matrix2D( Real _11, Real _21, Real _12, Real _22 ) : m11(_11), m21(_21), m12(_12), m22(_22) { }
		~Matrix2D() { }

		Real m11, m21, m12, m22;

		Matrix2D operator*(const Real rhs);

		Matrix2D operator*(const Matrix2D rhs);

		Vector2D operator*(const Vector2D rhs);

		static Matrix2D CreateRotation( const Real theta );

	};
	

	
	class Random {

	public:
		Random() {  }
		~Random() {  }

		static Real Number();

		static Real Range( Real minimum, Real maximum );

	};

	Real ComputeSum( std::vector<Real>& );
	Real ComputeMean( std::vector<Real>& );
	Real ComputeVariance( std::vector<Real>& );
	Real ComputeProduct( std::vector<Real>& );
	Real ComputeCovariance( std::vector<Real>&, std::vector<Real>& );
	Real ComputeMedian( std::vector<Real>& );
	Real GetMax( std::vector<Real>& );
	Real GetMin( std::vector<Real>& );

};




// this is to make it easier to display vectors using cout
std::ostream &operator<<( std::ostream &lhs, const VectorMath::Vector2D &rhs ); 

// this is to make it easier to display line segments using cout
std::ostream &operator<<( std::ostream &lhs, const VectorMath::LineSegment &rhs );

// same for rects
std::ostream &operator<<( std::ostream &lhs, const VectorMath::Rect &rhs );


