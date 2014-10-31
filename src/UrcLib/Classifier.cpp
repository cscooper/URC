/*
 *  Classifier.cpp - Computation of pathloss in CORNER.
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


#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <cfloat>
#include <climits>
#include <string>
#include <list>
#include <map>

#include "Singleton.h"
#include "VectorMath.h"
#include "Urc.h"
#include "Classifier.h"


using namespace std;
using namespace VectorMath;
using namespace Urc;



Classifier::Classifier() {

	if ( !UrcData::GetSingleton() )
		THROW_EXCEPTION( "Could not find an initialised UrcData Singleton, needed by ComputeState." );

	mPrecomputed = false;

}


Classifier::Classifier( UrcData::Classification c ) {

	if ( !UrcData::GetSingleton() )
		THROW_EXCEPTION( "Could not find an initialised UrcData Singleton, needed by ComputeState." );

	mPrecomputed = true;
	mClassification = c;

}



Classifier::~Classifier() { }

/*
 * Method: Real CalculatePathloss( VectorMath::Vector2D source, VectorMath::Vector2D destination );
 * Description: Calculate the pathloss given the source and destination in mW.
 */
Real Classifier::CalculatePathloss( Vector2D source, Vector2D destination ) {

	UrcData *pUrcData = UrcData::GetSingleton();

	if ( !mPrecomputed )
		ComputeState( source, destination );

	switch( mClassification.mClassification ) {

		case LOS:
			return ( pUrcData->GetLamdaBy4PiSq() / (source-destination).MagnitudeSq() );

		case NLOS1: {
			UrcData::Node *n1 = pUrcData->GetNode( mClassification.mNodeSet[0] );
			Real rm2 = (source - n1->position ).MagnitudeSq();
			Real rm = sqrt(rm2);
			Real rs2 = ( n1->position -  destination ).MagnitudeSq();
			Real rs = sqrt(rs2);
			Real Wm = mClassification.mMainStreetLaneCount * pUrcData->GetLaneWidth();
			Real Ws = mClassification.mSideStreetLaneCount * pUrcData->GetLaneWidth();

			unsigned int Nmin = (unsigned int)floor( 2 * sqrt( ( rm * rs ) / ( Ws * Wm ) ) );
			//calculate PLr
			Real PL = (pUrcData->GetLamdaBy4PiSq() * pow(pUrcData->GetLossPerReflection(), 2 * Nmin)) / pow((rm+rs),2);
			
			//calculate PLd
			if ( rm < rs )
				return (PL + ((pUrcData->GetLamdaBy4PiSq() * pUrcData->GetWavelength()) / (4 * rm * rs2)) );
			else
				return (PL + ((pUrcData->GetLamdaBy4PiSq() * pUrcData->GetWavelength()) / (4 * rs * rm2)) );
		}	
		case NLOS2: {
			UrcData::Node *n1 = pUrcData->GetNode( mClassification.mNodeSet[0] );
			UrcData::Node *n2 = pUrcData->GetNode( mClassification.mNodeSet[1] );
			
			Real rm2 = ( source - n1->position ).MagnitudeSq();
			Real rm = sqrt(rm2);
			Real rs = ( n1->position - n2->position ).Magnitude();
			Real rp2 = ( n2->position - destination ).MagnitudeSq();
			Real rp = sqrt(rp2);
			Real rsp = rs + rp;
			
			Real Wm = mClassification.mMainStreetLaneCount * pUrcData->GetLaneWidth();
			Real Ws = mClassification.mSideStreetLaneCount * pUrcData->GetLaneWidth();
			Real Wp = mClassification.mParaStreetLaneCount * pUrcData->GetLaneWidth();

			Real temp = sqrt( ( rs * Wm * Wp ) / ( Ws * ( rm * Wp + rp * Wm ) ) );
			unsigned int Nmin = (unsigned int)floor((rm * temp) / Wm + rs / (Ws * temp) + (rp * temp) / Wp);
			Real rPow2Nmin = pow(pUrcData->GetLossPerReflection(), 2*Nmin);
			unsigned int N = (unsigned int)floor( rp * rs / ( Wp * Ws ) );

			//calculate PLr
			Real PL = (pUrcData->GetLamdaBy4PiSq() * rPow2Nmin) / pow(rsp+rm, 2);
			
			//calculate PLdd
			if ( rm < rs )
				PL += (pUrcData->GetLamdaBy4PiSq() * pow(pUrcData->GetWavelength(),2)) / (16 * rm * rs * rp2) ; 
			else
				PL += (pUrcData->GetLamdaBy4PiSq() * pow(pUrcData->GetWavelength(),2)) / (16 * rm2 * rp * rs) ; 

			//calculate PLrd
			if ( rs < rp )
				PL += (pUrcData->GetLamdaBy4PiSq() * rPow2Nmin * pUrcData->GetWavelength() * rs) / (4 * pow(rs+rm, 2) * rp2);
			else
				PL += (pUrcData->GetLamdaBy4PiSq() * rPow2Nmin * pUrcData->GetWavelength()) / (4 * pow(rs+rm, 2) * rp);
			
			//calculate PLdr
			if ( rm < rsp )
				return (PL + (pow(pUrcData->GetLossPerReflection(),2*N) * pUrcData->GetLamdaBy4PiSq() * pUrcData->GetWavelength())/(4*rm*rsp*rsp));
			else
				return (PL + (pow(pUrcData->GetLossPerReflection(),2*N) * pUrcData->GetLamdaBy4PiSq() * pUrcData->GetWavelength())/(4*rsp*rm2));
		}
		case OutOfRange:
		default:
			//path-loss 0
			return(0); 
	}
}


/*
 * Method: void ComputeState( VectorMath::Vector2D source, VectorMath::Vector2D destination );
 * Description: Gets the CORNER state given the source and destination.
 */
void Classifier::ComputeState( Vector2D source, Vector2D destination ) {

	/*
	 * Two scenarios here:
	 * If the car is within node.mSize of an intersection, it considers all
	 * links connected to that intersection. If it isn't, the closest link is selected.
	 */
	UrcData *pUrcData = UrcData::GetSingleton();
	UrcData::LinkIndexSet txLinks;
	UrcData::LinkIndexSet rxLinks;

	// get the grid cells in which the transmitter and receiver are located
	UrcData::Grid *gridSource = pUrcData->GetGrid( source );
	UrcData::Grid *gridDestination = pUrcData->GetGrid( destination );
	UrcData::LinkIndexSet::iterator linkIndexIt;

	UrcData::Node *n1, *n2;
	UrcData::Link *pLink = NULL;
	double nodeDist = DBL_MAX, linkDist = DBL_MAX, d;

	UrcData::Node *nearestNodeTx = NULL;
	UrcData::Link *nearestLinkTx = NULL;

	// scan through the nodes
	for ( AllInVector( linkIndexIt, gridSource->linkList ) ) {

		//printf( "Classifer::ComputeState: Transmitter - Current Link: %d\n", *linkIndexIt );
		pLink = pUrcData->GetSummedLink( *linkIndexIt );
		n1 = pUrcData->GetNode( pLink->nodeAindex );
		n2 = pUrcData->GetNode( pLink->nodeBindex );
		LineSegment l( n1->position, n2->position );
		d = l.DistanceFromLine( source );
		if ( d < linkDist && d < pUrcData->GetLaneWidth()*pLink->NumberOfLanes ) {
			linkDist = d;
			nearestLinkTx = pLink;
		}

		if ( nearestNodeTx != n1 && nearestNodeTx != n2 ) {
			d = ( source - n1->position ).Magnitude();
			if ( nodeDist > d && n1->mSize > d ) {
				nearestNodeTx = n1;
				nodeDist = d;
			}
			d = ( source - n2->position ).Magnitude();
			if ( nodeDist > d && n2->mSize > d ) {
				nearestNodeTx = n2;
			}
		}

	}

	txLinks.push_back( nearestLinkTx->index );
	if ( nearestNodeTx )
		txLinks.insert( txLinks.begin(), nearestNodeTx->mConnectedLinks.begin(), nearestNodeTx->mConnectedLinks.end() );

	nodeDist = DBL_MAX;
	linkDist = DBL_MAX;
	UrcData::Node *nearestNodeRx = NULL;
	UrcData::Link *nearestLinkRx = NULL;

	// scan through the nodes
	for ( AllInVector( linkIndexIt, gridDestination->linkList ) ) {

		//printf( "Classifer::ComputeState: Receiver - Current Link: %d\n", *linkIndexIt );
		pLink = pUrcData->GetSummedLink( *linkIndexIt );
		n1 = pUrcData->GetNode( pLink->nodeAindex );
		n2 = pUrcData->GetNode( pLink->nodeBindex );
		LineSegment l( n1->position, n2->position );
		d = l.DistanceFromLine( destination );
		if ( d < linkDist && d < pUrcData->GetLaneWidth()*pLink->NumberOfLanes ) {
			linkDist = d;
			nearestLinkRx = pLink;
		}

		if ( nearestNodeRx != n1 && nearestNodeRx != n2 ) {
			d = ( destination - n1->position ).Magnitude();
			if ( nodeDist > d && n1->mSize > d ) {
				nearestNodeRx = n1;
				nodeDist = d;
			}
			d = ( destination - n2->position ).Magnitude();
			if ( nodeDist > d && n2->mSize > d  ) {
				nearestNodeRx = n2;
			}
		}

	}
	rxLinks.push_back( nearestLinkRx->index );
	if ( nearestNodeRx )
		rxLinks.insert( rxLinks.begin(), nearestNodeRx->mConnectedLinks.begin(), nearestNodeRx->mConnectedLinks.end() );

	UrcData::Classification c;
	UrcData::LinkIndexSet::iterator rxLinkIndexIt, txLinkIndexIt;
	
	mClassification.mClassification = Classifier::OutOfRange;
	mSourceLink = mDestinationLink = 0;
	
	for ( AllInVector( txLinkIndexIt, txLinks ) ) {

		for ( AllInVector( rxLinkIndexIt, rxLinks ) ) {

			c = pUrcData->GetClassification( *txLinkIndexIt, *rxLinkIndexIt );
			if ( c.mClassification < mClassification.mClassification ) {
				mClassification = c;
				mSourceLink = *txLinkIndexIt;
				mDestinationLink = *rxLinkIndexIt;
			}

			if ( mClassification.mClassification == Classifier::LOS )
				return;

		}

	}

}








