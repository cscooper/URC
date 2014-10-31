/*
 *  VectorMath.cpp - Contains vector geometry classes.
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

#include <algorithm>
#include <iostream>
#include <cfloat>

#include <math.h>
#include <cstdlib>
#include <climits>
#include <cstdarg>
#include <vector>

#include "VectorMath.h"

using namespace VectorMath;


/** Default constructor */
OrderedIndexPair::OrderedIndexPair( int f, int s ) {

	first = MIN( f, s );
	second = ( first == f ? s : f );

}

/** Default destructor */
OrderedIndexPair::~OrderedIndexPair() {  }

/** Less-Than operator overload. */
bool OrderedIndexPair::operator< ( const OrderedIndexPair &p ) const {

	return first < p.first || ( first == p.first && second < p.second );

}

/** logical equals operator overload. */
bool OrderedIndexPair::operator== ( const OrderedIndexPair &p ) const {

	return first == p.first && second == p.second;

}



Vector2D &Vector2D::operator+=(const Vector2D &rhs) {
	this->x+=rhs.x;
	this->y+=rhs.y;
	return *this;
}

bool Vector2D::operator==(const Vector2D& rhs) const {
	return ( this->x == rhs.x && this->y == rhs.y );
}

bool Vector2D::operator!=(const Vector2D& rhs) const {
	return ( this->x != rhs.x || this->y != rhs.y );
}

Vector2D Vector2D::operator+(const Vector2D& rhs) const {
	return Vector2D( this->x+rhs.x, this->y+rhs.y );
}

Vector2D Vector2D::operator-(const Vector2D& rhs) const {
	return Vector2D( this->x-rhs.x, this->y-rhs.y );
}

Vector2D Vector2D::operator*(const Real rhs) const {
	return Vector2D( this->x*rhs, this->y*rhs );
}

Real Vector2D::operator*(const Vector2D& rhs) const {
        return (this->x*rhs.y - this->y*rhs.x);
}

Vector2D Vector2D::operator/(const Real rhs) const {
	return Vector2D( this->x/rhs, this->y/rhs );
}


/*
*	Function:	 Real Magnitude();
*	Description: Computes the magnitude of the vector.
*/		
Real Vector2D::Magnitude() const {
	return sqrt( this->x*this->x + this->y*this->y );
}

Real Vector2D::MagnitudeSq() const {
	return (this->x*this->x + this->y*this->y);
}

/*
*	Function:	 Vector2D Unitise();
*	Description: Computes the unit form of the vector.
*/
Vector2D Vector2D::Unitise() const {

	return (*this) / Magnitude();

}



/*
 *	Function:	 Real DistanceSq( const Vector2D &v ) const;
 *	Description: Computes the square distance from this vector to the given vector v.
 */
Real Vector2D::DistanceSq( const Vector2D &v ) const {

	Real x = v.x - this->x, y = v.y - this->y;
	return x*x + y*y;

}


/*
 *	Function:	 Real Distance( const Vector2D &v ) const;
 *	Description: Computes the distance from this vector to the given vector v.
 */
Real Vector2D::Distance( const Vector2D &v ) const {

	return sqrt( DistanceSq( v ) );

}

/*
*	Function:	 Real DotProduct( const Vector2D &rhs )
*	Description: Computes the dot product between this vector and the given vector
*/
Real Vector2D::DotProduct( const Vector2D &rhs ) const {
	return ( this->x*rhs.x + this->y*rhs.y );
}

/*
*	Function:	 Vector2D Reflect( LineSegment line )
*	Description: Reflects this vector off the given line.
*/
Vector2D Vector2D::Reflect( LineSegment line ) const {

	Vector2D n = line.GetNormal().Unitise();
	return (*this) - n * ( DotProduct( n ) ) * 2.0;

}

/*
*	Function:	 Real AngleBetween( Vector2D rhs ) const
*	Description: Computes the angle between two vectors.
*/
Real Vector2D::AngleBetween( Vector2D rhs ) const {
	
	Real dotProduct = this->DotProduct( rhs );
	Real magRHS = rhs.Magnitude();
	Real magLHS = this->Magnitude();

	return acos( dotProduct / ( magRHS * magLHS ) );
	
}

std::ostream &operator<<( std::ostream &lhs, const Vector2D &rhs ) {

	lhs << "(" << rhs.x << "," << rhs.y << ")";
	return lhs;

}



Vector3D &Vector3D::operator+=(const Vector2D &rhs) {
	this->x+=rhs.x;
	this->y+=rhs.y;
	this->z=0;
	return *this;
}

Vector3D &Vector3D::operator+=(const Vector3D &rhs) {
	this->x+=rhs.x;
	this->y+=rhs.y;
	this->z+=rhs.z;
	return *this;
}

bool Vector3D::operator==(const Vector3D &rhs) {
	return ( this->x == rhs.x && this->y == rhs.y && this->z == rhs.z );
}

bool Vector3D::operator!=(const Vector3D &rhs) {
	return ( this->x != rhs.x || this->y != rhs.y || this->z != rhs.z );
}

Vector3D Vector3D::operator+(const Vector3D &rhs) {
	return Vector3D( this->x+rhs.x, this->y+rhs.y, this->z+rhs.z );
}

Vector3D Vector3D::operator-(const Vector3D &rhs) {
	return Vector3D( this->x-rhs.x, this->y-rhs.y, this->z-rhs.z );
}

Vector3D Vector3D::operator*(const Real rhs) {
	return Vector3D( this->x*rhs, this->y*rhs, this->z*rhs );
}

Vector3D Vector3D::operator/(const Real rhs) {
	return Vector3D( this->x/rhs, this->y/rhs, this->z/rhs );
}

/*
*	Function:	 Real Magnitude();
*	Description: Computes the magnitude of the vector.
*/		
Real Vector3D::Magnitude() {
	return sqrt( this->x*this->x + this->y*this->y + this->z*this->z ); 
}

/*
*	Function:	 Vector3D Unitise();
*	Description: Computes the unit form of the vector.
*/
Vector3D Vector3D::Unitise() {
	return (*this) / Magnitude();
}

/*
*	Function:	 Real DotProduct( const Vector3D &rhs )
*	Description: Computes the dot product between this vector and the given vector
*/		
Real Vector3D::DotProduct( const Vector3D &rhs ) {
	return ( this->x*rhs.x + this->y*rhs.y + this->z*rhs.z );
}


/*
*	Function:	 Real AngleBetween( Vector3D rhs )
*	Description: Computes the angle between two vectors.
*/				
Real Vector3D::AngleBetween( Vector3D rhs ) {
	
	Real dotProduct = this->DotProduct( rhs );
	Real magRHS = rhs.Magnitude();
	Real magLHS = this->Magnitude();
	return acos( dotProduct / ( magRHS * magLHS ) );
	
}

//private helper function to determine if three vectors are counter-clockwise to each other
bool LineSegment::CounterClockWise (Vector2D A, Vector2D B, Vector2D C) {
	return ((C.y-A.y)*(B.x-A.x) > (B.y-A.y)*(C.x-A.x));
}


// private helper function to check for an intersection
// bool LineSegment::CheckIntersection( LineSegment l2, Vector2D *pVec ) {

	// Line 1 is (*this), Line 2 is l2
	// Got this from a forum post on gamedev.net
	// Required some modifications though.

	//mStart A
	//mEnd B
	//l2.mStart C
	//l2.mEnd D

	
// 	Vector2D u =    mStart -    mEnd;
// 	Vector2D v = l2.mStart - l2.mEnd;
// 	Real D = u * (v*-1);
// 
// 	if ( abs(D) < DBL_EPSILON )
// 		return false; //parallel test
// 
// 	Vector2D w = mStart - l2.mStart;
// 
// 	Real s = v * w;
// 	Real t = u * w;
// 	if ( ( s < 0 || s > D ) || ( t < 0 || t > D ) )
// 		return false;
// 
// 	if ( pVec ) {
// 		*pVec = l2.mStart + l2.GetVector() * (t/D);
// 	}
// 
// 	return true;

// }

bool LineSegment::IntersectLine( LineSegment l2, Vector2D *pVec ) {
	if ( CounterClockWise(mStart,l2.mStart,l2.mEnd) != CounterClockWise(mEnd,l2.mStart,l2.mEnd) && 
		CounterClockWise(mStart,mEnd,l2.mStart) != CounterClockWise(mStart,mEnd,l2.mEnd) ) {
		
		if (pVec) { //should work for all cases, but recheck for a larger test set
			Vector2D u =    mStart -    mEnd; //vector of self (AB)
			Vector2D v = l2.mEnd - l2.mStart; //vector of -l2 (DC)
			Vector2D w = mStart - l2.mStart; //vector of AC
			*pVec = l2.mStart + v * ((u * w)/(u * v)); //position of start of l2 + direction Vector * length of line to intersection
		}
		return true;
	} else {
		return false;
	}
}

bool LineSegment::IntersectCircle( Vector2D cent, Real r ) {

	Real a = GetDistance();
	Real b = LineSegment( mStart, cent ).GetDistance();
	Real c = LineSegment( mEnd, cent ).GetDistance();
	Real s = ( a + b + c ) / 2;
	return ( 2 / a ) * sqrt( s*(s-a)*(s-b)*(s-c) ) < r;

}

bool LineSegment::IsBounded( LineSegment l2) {
	//x-axis and y-axis bound checker as suggested by Nalin
  	if ((MAX(mStart.x,mEnd.x) < MIN(l2.mStart.x,l2.mEnd.x) || MAX(mStart.y,mEnd.y) < MIN(l2.mStart.y,l2.mEnd.y)))  {
  		return false;
  	}
  	return true;
}


bool LineSegment::operator==(const LineSegment &rhs) {
	return ( this->mStart == rhs.mStart ) && ( this->mEnd == rhs.mEnd );
}

bool LineSegment::operator!=(const LineSegment &rhs) {
	return ( this->mStart != rhs.mStart ) || ( this->mEnd != rhs.mEnd );
}
	
/*
 *	Function:	 Vector2D GetVector()
 *	Description: Computes the free vector from the fixed vector.
 */				
Vector2D LineSegment::GetVector() { return ( mEnd - mStart ); }


/*
 *	Function:	 Vector2D GetDirection()
 *	Description: Computes the direction of the free vector.
 */
Vector2D LineSegment::GetDirection() {
	Vector2D vec = GetVector();
	return vec / vec.Magnitude();
}


/*
 *	Function:	 Real GetDistance()
 *	Description: Computes the distance between the start and end of the vector.
 */				
Real LineSegment::GetDistance() {
	Vector2D vec = GetVector();
	return vec.Magnitude();
}


Vector2D LineSegment::GetNormal() {
	return Vector2D( mEnd.y - mStart.y, mStart.x - mEnd.x ).Unitise();
}

Real LineSegment::DistanceFromLine( Vector2D p ) {

	Vector2D n = GetNormal().Unitise();
	Vector2D r = mStart - p;
	return abs( n.DotProduct( r ) );

}

Real LineSegment::DistanceFromPoint(Vector2D p) {
	//source: http://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
	Real t = (p-mStart).DotProduct(mEnd-mStart) / ((mEnd.x-mStart.x)*(mEnd.x-mStart.x) + (mEnd.y-mStart.y)*(mEnd.y-mStart.y)) ;
	//basically, if its less than 0, then the projected point is before the start point, > 1, after the end point
	if (t < 0.0) {
		return (p-mStart).Magnitude();
	} else if (t > 1.0) {
		return (p-mEnd).Magnitude();
	}
	return (p-(mStart + (mEnd-mStart)*t)).Magnitude();
}

Real LineSegment::DistanceAlongLine( Vector2D p ) {

	Vector2D n = GetVector().Unitise();
	Vector2D r = mStart - p;
	return abs( n.DotProduct( r ) );

}



bool LineSegment::PointInCommon( LineSegment l ) {

	return  mStart == l.mStart || mEnd == l.mStart || l.mEnd == mStart || mEnd == l.mEnd;

}


Vector2D LineSegment::Midpoint() {

	return ( mEnd + mStart ) / 2;

}



// this is to make it easier to display line segments using cout
std::ostream &operator<<( std::ostream &lhs, const LineSegment &rhs ) {

	lhs << "[" << rhs.mStart << "," << rhs.mEnd << "]";
	return lhs;

}

std::ostream &operator<<( std::ostream &lhs, const Rect &rhs) {
	lhs << "[" << rhs.location << "," <<rhs.size << "]";
	return lhs;
	
}




/*
 *	Function:	 Rect Scale( Real factor, Vector2D vecPoint )
 *	Description: Computes a new rectangle that has been scaled by the given factor
 *				 about the given point within the vector.
 */				
Rect Rect::Scale( Real factor, Vector2D vecPoint ) {
	Rect newRect = Rect( vecPoint * -factor, size * factor );
	newRect.location = newRect.location + location + size / 2;
	return newRect;
}


/*
 *	Function:	 Vector2D Centre( bool Absolute );
 *	Description: Computes the centre point of the rectangle.
 *				 If Absolute=true, the centre point is given in screen coordinates.
 */				
Vector2D Rect::Centre( bool Absolute ) {
	return size/2 + ( Absolute ? location : 0 );
}


/*
 *	Function:	 bool PointWithin( Vector2D point );
 *	Description: Determines if the given point is within the rect.
 */		
bool Rect::PointWithin( Vector2D point ) {
//	return ( point.x > location.x && point.y > location.y && point.x < ( location.x + size.x ) && point.y < ( location.y + size.y ) );
	Vector2D v = point - location;
	return ( v.x >= 0 && v.x <= size.x && v.y >= 0 && v.y <= size.y );
}

/*
 *	Function:	 bool IntersectsRect( Rect rect );
 *	Description: Determines if this rect and the given rect intersect.
 */
bool Rect::IntersectsRect( Rect rect ) {
	return PointWithin( rect.location ) || PointWithin( Vector2D( rect.location.x + rect.size.x, rect.location.y ) )
		|| PointWithin( Vector2D( rect.location.x, rect.location.y + rect.size.y ) )
		|| PointWithin( Vector2D( rect.location.x + rect.size.x, rect.location.y + rect.size.y ) );
}

bool Rect::LineSegmentWithin(LineSegment line) {
	//check if either the start point or the end point are within the rectangle
	if (PointWithin(line.mStart) || PointWithin(line.mEnd)) {
		return true;
	}
	
	Vector2D addWidth = Vector2D(size.x,0);
	Vector2D addHeight = Vector2D(0, size.y);
	LineSegment leftLine(location, location+addHeight);
	LineSegment bottomLine(location, location+addWidth);
	LineSegment rightLine(location+addWidth, location+size);
	LineSegment topLine(location+addHeight, location+size);
	
	//if the line segment intersects any part of the rectangle, then its within the rectangle
	if (line.IntersectLine(leftLine, NULL) || line.IntersectLine(rightLine, NULL) || line.IntersectLine(bottomLine, NULL) || line.IntersectLine(topLine, NULL)) {
		return true;
	}
	
	return false;
}


Matrix2D Matrix2D::operator*(const Real rhs) {
	return Matrix2D( this->m11*rhs, this->m21*rhs, this->m12*rhs, this->m22*rhs );
}

Matrix2D Matrix2D::operator*(const Matrix2D rhs) {

	Real _11, _21, _12, _22;
	_11 = this->m11 * rhs.m11 + this->m12 * rhs.m21;
	_12 = this->m11 * rhs.m12 + this->m12 * rhs.m22;
	_21 = this->m21 * rhs.m11 + this->m22 * rhs.m21;
	_22 = this->m21 * rhs.m12 + this->m22 * rhs.m22;
	return Matrix2D( _11, _21, _12, _22 );

}


Vector2D Matrix2D::operator*(const Vector2D rhs) {

	return Vector2D( rhs.x*m11 + rhs.y*m21, rhs.x*m12 + rhs.y*m22 );

}


Matrix2D Matrix2D::CreateRotation( const Real theta ) {
	return Matrix2D( cos(theta), -sin(theta), sin(theta), cos(theta) );
}



Real Random::Number() {
	return (Real)rand() / (Real)RAND_MAX;
}

Real Random::Range( Real minimum, Real maximum ) {
	return Number() * ( maximum - minimum ) + minimum;
}




Real VectorMath::ComputeSum( std::vector<Real>& dataSet ) {

	double sum = 0;
	std::vector<Real>::iterator it;
	for( AllInVector( it, dataSet ) ) {
		sum += *it;
	}
	return sum;

}




Real VectorMath::ComputeMean( std::vector<Real>& dataSet ) {

	return ComputeSum( dataSet ) / dataSet.size();

}





Real VectorMath::ComputeVariance( std::vector<Real>& dataSet ) {

	double sum = 0;
	std::vector<Real>::iterator it;
	for( AllInVector( it, dataSet ) ) {
		sum += pow( *it, 2 );
	}
	return ( sum / dataSet.size() ) - pow( ComputeMean( dataSet ), 2 );

}





Real VectorMath::ComputeProduct( std::vector<Real>& dataSet ) {

	double sum = 0;
	std::vector<Real>::iterator it;
	for( AllInVector( it, dataSet ) ) {
		sum += *it;
	}
	return sum;

}




Real VectorMath::ComputeCovariance( std::vector<Real>& X, std::vector<Real>& Y ) {

	if ( X.size() != Y.size() )
		return DBL_MAX;

	double sum = 0, meanX, meanY;
	unsigned int i = 0;

	meanX = ComputeMean( X );
	meanY = ComputeMean( Y );

	for( ; i < X.size(); i++ ) {
		sum += ( X[i] - meanX ) * ( Y[i] - meanY );
	}
	return sum / X.size();

}



Real VectorMath::ComputeMedian( std::vector<Real>& dataSet ) {

	// sort the dataset
	std::sort( dataSet.begin(), dataSet.end() );
	if ( ( dataSet.size() % 2 ) == 1 )
		return dataSet[ (dataSet.size() + 1)/2 ];
	else
		return ( dataSet[ dataSet.size()/2 ] + dataSet[ dataSet.size()/2 + 1 ] ) / 2;

}


Real VectorMath::GetMax( std::vector<Real>& dataSet ) {

	// sort the dataset
	std::sort( dataSet.begin(), dataSet.end() );
	return dataSet.back();

}


Real VectorMath::GetMin( std::vector<Real>& dataSet ) {

	// sort the dataset
	std::sort( dataSet.begin(), dataSet.end() );
	return dataSet.front();

}




