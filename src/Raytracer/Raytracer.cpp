/*
 *  Raytracer.cpp - Raytracer-based K Factor calculation
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

#include <cfloat>
#include <climits>
#include <list>
#include <map>
#include <queue>

#include "Urc.h"
#include "Raytracer.h"
//#define RAYTRACE_DEBUG

#include <iostream>
#include <fstream>

using namespace Urc;
using namespace VectorMath;
using namespace std;



/*
 * Method: void TraceRay( RayPathComponent );
 * Description: This traces a ray through the road network.
 */
void Raytracer::TraceRay( Raytracer::RayPathComponent ray ) {

	UrcData *pUrcData = UrcData::GetSingleton();
	int lastEdge = ray.mLastReflectorIndex;
	int intersectObjectIndex = 0;
	Real incidenceAngle = 0, d, permitivity, scatCoef = 1, angle;
	RayPathComponent newRay;
	LineSegment impactedEdge;
	Vector2D intersectPoint;

	bool bFound = CheckIntersection( ray, &intersectPoint, &incidenceAngle, &impactedEdge, &intersectObjectIndex, &lastEdge );
	lastEdge = intersectObjectIndex;

	// if we didn't find any intersections
	if ( !bFound ) {

		pthread_mutex_lock( &mRaySetMutex );
		mRaySeq.push_back( ray );
		pthread_mutex_unlock( &mRaySetMutex );
		return;

	}

	ray.mLineSegment = LineSegment( ray.mLineSegment.mStart, intersectPoint );
	ray.mDistanceSum += ray.mLineSegment.GetDistance();
	ray.mReflectionCount++;
	pthread_mutex_lock( &mRaySetMutex );
	mRaySeq.push_back( ray );
	pthread_mutex_unlock( &mRaySetMutex );

	permitivity = pUrcData->GetBuilding( intersectObjectIndex )->mPermitivity;

	newRay.mReflectionCoefficient = ray.mReflectionCoefficient * ( sqrt(permitivity - cos(incidenceAngle)*cos(incidenceAngle)) - permitivity*sin(incidenceAngle) ) / ( sqrt(permitivity - cos(incidenceAngle)*cos(incidenceAngle)) + permitivity*sin(incidenceAngle) );
	newRay.mDistanceSum = ray.mDistanceSum;
	d = ray.mReflectionCoefficient * mRayLength - newRay.mDistanceSum;
	if ( d <= 0 )
		return;
	newRay.mLineSegment = LineSegment( intersectPoint, intersectPoint + ray.mLineSegment.GetVector().Reflect( impactedEdge ).Unitise() * d );

	newRay.mLastReflectorIndex = lastEdge;
	pthread_mutex_lock( &mRayQueueMutex );
	mRayQueue.push( newRay );
	pthread_mutex_unlock( &mRayQueueMutex );

}





/*
 * Method: bool CheckIntersection( RayPathComponent, VectorMath::Vector2D*, Real *, int *, int );
 * Description: This traces a ray through the road network.
 */
bool Raytracer::CheckIntersection( RayPathComponent ray, VectorMath::Vector2D *intersectPoint, Real *incidentAngle, VectorMath::LineSegment *impactedLine, int *intersectObjectIndex, int *lastEdge ) {

	UrcData *pUrcData = UrcData::GetSingleton();
	Vector2D temp;
	Real Dmin = DBL_MAX;
	UrcData::LineSet::iterator lineIt;
	UrcData::Bucket::iterator buckIt;

	Rect rayRect = Rect(
		std::min( ray.mLineSegment.mStart.x, ray.mLineSegment.mEnd.x ),
		std::min( ray.mLineSegment.mStart.y, ray.mLineSegment.mEnd.y ),
		fabs( ray.mLineSegment.mStart.x - ray.mLineSegment.mEnd.x ),
		fabs( ray.mLineSegment.mStart.y - ray.mLineSegment.mEnd.y )
	);

	// scan through the list of building outlines
	for ( AllInVector( buckIt, mBucket ) ) {

		// if the current edge we're looking at is the same as the last edge we reflected off, ignore it
		if ( lastEdge && *lastEdge >= 0 )
			if ( *buckIt == *lastEdge )
				continue;

		UrcData::Building *b = pUrcData->GetBuilding( *buckIt );

		// check for intersection and store the intersection point in temp
		for ( AllInVector( lineIt, b->mEdgeSet ) ) {

			if ( !rayRect.LineSegmentWithin( *lineIt ) )
				continue;

			if ( lineIt->IntersectLine( ray.mLineSegment, &temp ) ) {

				// get the distance between the start of the ray and the intersection 
				Real d = ( ray.mLineSegment.mStart - temp ).Magnitude();

				Real dint = pUrcData->GetLaneWidth() / 2;

				// we want to make sure this ray has actually gone somewhere.
				// Also, we want to make sure that the found intersection is the closest one.
				if ( d >= dint && d < Dmin ) {

					Dmin = d;
					*intersectPoint = temp;
					*intersectObjectIndex = *buckIt;
					*incidentAngle = ray.mLineSegment.GetVector().AngleBetween( lineIt->GetNormal() );
					*impactedLine = (*lineIt);

				}

			}

		}

	};

	// if we didn't find any intersections
	if ( Dmin == DBL_MAX )
		return false;

	if ( *incidentAngle > M_PI/2 )
		*incidentAngle = M_PI - *incidentAngle;
	else if ( *incidentAngle < 0 )
		*incidentAngle = M_PI + *incidentAngle;

	return true;

}






/*
 * Method: static void *WorkerThread(void *pRT);
 * Description: Traces the rays through the network. Multiple worker threads can work in parallel.
 */
void *Raytracer::WorkerThread(void *pRT) {

	Raytracer *raytracer = (Raytracer*)pRT;
	if ( !raytracer ) {
		THROW_EXCEPTION( "Invalid pointer passed to worker thread!" );
	}

	bool bDone = false;
	while( !bDone ) {
		bDone = raytracer->RunTrace();
	}

	return NULL;

}





/*
 * Constructor arguments:
 * 		1. Transmitter Position - location of the transmitter in the network
 * 		2. Ray Count - number of rays to simulate
 * 		4. Number of Workers - Specify the number of worker threads
 */
Raytracer::Raytracer( Vector2D tx, unsigned int N, unsigned int nWorkers ) {

	mRaySeq.clear();

	mPositionTX = tx;
	mRayCount = N;
	mStartAngle = M_PI * (Real)rand() / ( 2 * (Real)RAND_MAX );

	// check for a UrcData singleton, used by GetLineSet()
	UrcData *pUrcData = UrcData::GetSingleton();
	if ( pUrcData == NULL )
		THROW_EXCEPTION("Raytracer requires an initialised UrcData Singleton. Found none!");

	mExecuted = false;

	mRayLength = pUrcData->GetFreeSpaceRange();

	mNumberOfWorkers = nWorkers;
	mRayQueueMutex = PTHREAD_MUTEX_INITIALIZER;
	mRaySetMutex = PTHREAD_MUTEX_INITIALIZER;
	mWorkerThreads = new pthread_t[mNumberOfWorkers];

}



Raytracer::~Raytracer() {
	mRaySeq.clear();
}



/*
 * Method: RayPathComponentSet *GetRaySet();
 * Description: Get a pointer to the trace
 */
const Raytracer::RayPathComponentSet *Raytracer::GetRaySet() const {

	return &mRaySeq;

}




/*
 * Method: void RunTrace();
 * Description: Run the trace in the worker thread. Returns true if done.
 */
bool Raytracer::RunTrace() {

	Raytracer::RayPathComponent ray;

	bool isEmpty = false;
	pthread_mutex_lock( &mRayQueueMutex );
	isEmpty = mRayQueue.empty();
	if ( !isEmpty ) {
		ray = mRayQueue.front();
		mRayQueue.pop();
	}
	pthread_mutex_unlock( &mRayQueueMutex );

	if ( !isEmpty ) {
		TraceRay( ray );
	}
	return isEmpty;

}



/*
 * Method: void Execute();
 * Description: Run the trace.
 */
void Raytracer::Execute() {

	if ( mExecuted )
		THROW_EXCEPTION( "Trace has already been executed." );

	UrcData::GetSingleton()->CollectBucketsInRange( 2*mRayLength, mPositionTX, &mBucket );

	for ( unsigned int r = 0; r < mRayCount; r++ ) {
		Real alpha = mStartAngle + 2*M_PI*r/mRayCount;
		RayPathComponent newComponent;
		newComponent.mDistanceSum = 0;
		newComponent.mLineSegment = LineSegment( mPositionTX, mPositionTX+Vector2D(cos(alpha),sin(alpha))*mRayLength );
		newComponent.mReflectionCoefficient = 1;
		newComponent.mReflectionCount = 0;
		newComponent.mLastReflectorIndex = -1;
		mRayQueue.push( newComponent );
	}


	unsigned int i;
	for ( i = 0; i < mNumberOfWorkers; i++ ) {
		if ( pthread_create( &mWorkerThreads[i], NULL, &Raytracer::WorkerThread, this ) ) {
			THROW_EXCEPTION( "Could not create worker threads for Raytracer." );
		}
	}

	for ( i = 0; i < mNumberOfWorkers; i++ ) {
		pthread_join( mWorkerThreads[i], NULL );
	}

	delete[] mWorkerThreads;

	mExecuted = true;

}



/*
 * Method: TraceReport ComputeK( VectorMath::Vector2D receiverPosition, VectorMath::Real gain );
 * Description: This computes the K factor for the receiver given its position and speed.
 */
Raytracer::TraceReport Raytracer::ComputeK( VectorMath::Vector2D rx, VectorMath::Real gain ) {

	UrcData *pUrcData = UrcData::GetSingleton();
	RayPathComponentSet::iterator componentIt;
	Real r = sqrt(gain) * pUrcData->GetWavelength() / (2 * M_PI);
	unsigned int minRefl=UINT_MAX;

	vector< InterceptedRay > interceptedRays;

	for ( AllInVector( componentIt, mRaySeq ) ) {

		Real d = componentIt->mLineSegment.DistanceAlongLine( rx );
		if ( componentIt->mLineSegment.DistanceFromLine( rx ) < r && d > 0 && d < componentIt->mLineSegment.GetDistance() ) {

			InterceptedRay ray;
			ray.mDistance = d;
			ray.m_pComponent = &(*componentIt);
			interceptedRays.push_back( ray );
			if ( minRefl > componentIt->mReflectionCount )
				minRefl = componentIt->mReflectionCount;

		}

	}

	vector< InterceptedRay >::iterator interceptIt;
	TraceReport t;
	t.mSpecularPower = t.mDiffusePower = 0;
	t.mFactorK = -1;
	t.mSpecularRayCount = t.mDiffuseRayCount = 0;

	t.mTransmitterPosition = mPositionTX;
	t.mReceiverPosition = rx;

	if ( interceptedRays.size() == 0 )
		return t;	// if we got no intercepted rays

	t.mFactorK = 0;
	t.mDiffuseRayCount = interceptedRays.size();
	if ( minRefl > 0 )
		return t;	// got no LOS rays, so assume rayleigh

	t.mDiffuseRayCount = 0;
	for ( AllInVector( interceptIt, interceptedRays ) ) {

		double phi = ( 2 * ( interceptIt->mDistance + interceptIt->m_pComponent->mDistanceSum ) / pUrcData->GetWavelength() + interceptIt->m_pComponent->mReflectionCount ) * 2 * M_PI;
		double p = interceptIt->m_pComponent->mReflectionCoefficient * interceptIt->m_pComponent->mReflectionCoefficient * ( 0.5 + sin( phi ) / M_PI );
		if ( interceptIt->m_pComponent->mReflectionCount == minRefl ) {
			t.mSpecularPower += p;
			t.mSpecularRayCount++;
		} else {
			t.mDiffusePower += p;
			t.mDiffuseRayCount++;
		}

		t.mRayPowers.push_back( p );

	}

	t.mRayPowerMean = ComputeMean( t.mRayPowers );
	t.mRayPowerVariance = ComputeVariance( t.mRayPowers );
	t.mRayPowerMedian = ComputeMedian( t.mRayPowers );

// 	Real m = GetMax( t.mRayPowers ) - 2*sqrt(t.mRayPowerVariance);
// 
// 	std::vector<Real>::iterator powerIt;
// 	for ( AllInVector( powerIt, t.mRayPowers ) ) {
// 		if ( (*powerIt) < m ) {
// 			t.mDiffusePower += (*powerIt);
// 			t.mDiffuseRayCount++;
// 		} else {
// 			t.mSpecularPower += (*powerIt);
// 			t.mSpecularRayCount++;
// 		}
// 	}

	if ( t.mDiffusePower == 0 )
		t.mFactorK = DBL_MAX;	// best stand-in for infinity I can think of.
	else
		t.mFactorK = t.mSpecularPower / t.mDiffusePower;

	return t;

}









