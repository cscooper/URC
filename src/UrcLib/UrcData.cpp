/*
 *  UrcData.cpp - Contains the building geometry and CORNER classifications for a sumo map.
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
#include <string>
#include <list>
#include <map>
#include <climits>

#include "Singleton.h"
#include "VectorMath.h"
#include "UrcData.h"
#include "Classifier.h"

using namespace std;
using namespace VectorMath;
using namespace Urc;


DECLARE_SINGLETON( UrcData );






Real UrcData::GetWavelength() {
	return mWavelength;
}

Real UrcData::GetLamdaBy4PiSq() {
	return mLambdaBy4PiSq;
}

Real UrcData::GetTransmitPower() {
	return mTransmitPower;
}


Real UrcData::GetSystemLoss() {
	return mSystemLoss;
}


Real UrcData::GetReceiverSensitivity() {
	return mSensitivity;
}


Real UrcData::GetFreeSpaceRange() {
	return mFreeSpaceRange;
}


Real UrcData::GetLaneWidth() {
	return mLaneWidth;
}


Real UrcData::GetLossPerReflection() {
	return mLossPerReflection;
}





UrcData::UrcData( VectorMath::Real laneWidth, VectorMath::Real lambda, VectorMath::Real txPower, VectorMath::Real L, VectorMath::Real sensitivity, VectorMath::Real lpr, VectorMath::Real grid ) {

	mLaneWidth = laneWidth;
	mWavelength = lambda;
	mTransmitPower = txPower;
	mSystemLoss = L;
	mSensitivity = sensitivity; 
	mLossPerReflection = lpr;
	mGridSize = grid;
	mBucketSize = grid;
	mLambdaBy4PiSq = pow( mWavelength / (4 * M_PI), 2 );
	mFreeSpaceRange = ( mWavelength / ( 4 * M_PI ) ) * sqrt( mTransmitPower / ( mSystemLoss * mSensitivity ) );

}


/*
 * Constructor Arguments:
 * 		1. linksFile - file name of the CORNER links file
 * 		2. nodesFile - file name of the CORNER nodes file
 * 		3. classFile - file name of the CORNER class file
 * 		4. buildingFile - file name of the CORNER building file
 * 		5. linkMapFile - file name of the CORNER link mapping file
 * 		6. laneWidth - width of one lane in metres
 * 		7. lambda - wavelength of the carrier signal
 * 		8. txPower - transmission power of the signal
 * 		9. L - losses due to the system (signal processing, etc) not related to propagation
 * 		10. sensitivity - the sensitivity of the receiver
 * 		11. lpr - The loss per reflection
 */
UrcData::UrcData(
		const char* linksFile,
		const char* nodesFile,
		const char* classFile,
		const char* buildingFile,
		const char* linkMapFile, 
		VectorMath::Real laneWidth, 
		VectorMath::Real lambda, 
		VectorMath::Real txPower, 
		VectorMath::Real L, 
		VectorMath::Real sensitivity, 
		VectorMath::Real lpr, 
		VectorMath::Real grid ) { 

	mLaneWidth = laneWidth;
	mWavelength = lambda;
	mTransmitPower = txPower;
	mSystemLoss = L; 
	mSensitivity = sensitivity; 
	mLossPerReflection = lpr; 
	mGridSize = grid; 
	mBucketSize = grid; 
	mLambdaBy4PiSq = pow( mWavelength / (4 * M_PI), 2 );
	mFreeSpaceRange = sqrt( mLambdaBy4PiSq * mTransmitPower / ( mSystemLoss * mSensitivity ) );

	LoadNetwork( linksFile, nodesFile, classFile, buildingFile, linkMapFile, NULL, NULL, NULL );
	ComputeSummedLinkSet();
	ComputeBuckets();

}



/*
 * Constructor Arguments:
 * 		1. linksFile - file name of the CORNER links file
 * 		2. nodesFile - file name of the CORNER nodes file
 * 		3. classFile - file name of the CORNER class file
 * 		4. buildingFile - file name of the CORNER building file
 * 		5. linkMapFile - file name of the CORNER link mapping file
 * 		6. intLinkMapFile - file name of the CORNER link mapping file
 * 		7. riceDataFile - file name of the pre-computed K-factor data
 * 		8. carDefFile - file name containing car definitions
 * 		9. laneWidth - width of one lane in metres
 * 		10. lambda - wavelength of the carrier signal
 * 		11. txPower - transmission power of the signal
 * 		12. L - losses due to the system (signal processing, etc) not related to propagation
 * 		13. sensitivity - the sensitivity of the receiver
 * 		14. lpr - The loss per reflection
 */
UrcData::UrcData(
		const char* linksFile,
		const char* nodesFile,
		const char* classFile,
		const char* buildingFile,
		const char* linkMapFile, 
		const char* intLinkMapFile, 
		const char* riceDataFile, 
		const char* carDefFile, 
		VectorMath::Real laneWidth, 
		VectorMath::Real lambda, 
		VectorMath::Real txPower, 
		VectorMath::Real L, 
		VectorMath::Real sensitivity, 
		VectorMath::Real lpr, 
		VectorMath::Real grid ) { 

	mLaneWidth = laneWidth;
	mWavelength = lambda;
	mTransmitPower = txPower;
	mSystemLoss = L; 
	mSensitivity = sensitivity; 
	mLossPerReflection = lpr; 
	mGridSize = grid; 
	mBucketSize = grid; 
	mLambdaBy4PiSq = pow( mWavelength / (4 * M_PI), 2 );
	mFreeSpaceRange = sqrt( mLambdaBy4PiSq * mTransmitPower / ( mSystemLoss * mSensitivity ) );

	LoadNetwork( linksFile, nodesFile, classFile, NULL, linkMapFile, intLinkMapFile, riceDataFile, carDefFile );
	ComputeSummedLinkSet();
	ComputeBuckets();

}


UrcData::~UrcData() {

	mNodeSet.clear();
	mLinkSet.clear();
	mSummedLinkSet.clear();
	mClassificationMap.clear();
	mBuildingSet.clear();

}


/*
 * Method: void AddBuilding( Building newBuilding );
 * Description: Manually add a building to the network
 */
void UrcData::AddBuilding( UrcData::Building newBuilding ) {
	mBuildingSet.push_back( newBuilding );
}



/*
 * Method: void CalculateMapRectangle();
 * Description: Calculate map bounds after manually adding buildings.
 */
void UrcData::CalculateMapRectangle() {

	Vector2D topLeft, bottomRight, v1, v2;
	topLeft = Vector2D(DBL_MAX,DBL_MAX);
	bottomRight = Vector2D(DBL_MIN,DBL_MIN) * -1;
	std::vector<Real> dX, dY;

	for ( BuildingSet::iterator it = mBuildingSet.begin(); it != mBuildingSet.end(); it++ ) {

		for ( LineSet::iterator lineIt = it->mEdgeSet.begin(); lineIt != it->mEdgeSet.end(); lineIt++ ) {

			dX.push_back( lineIt->mStart.x );
			dY.push_back( lineIt->mStart.y );
			dX.push_back(   lineIt->mEnd.x );
			dY.push_back(   lineIt->mEnd.y );

		}

		v1.x = *std::min_element( dX.begin(), dX.end() );
		v1.y = *std::min_element( dY.begin(), dY.end() );
		v2.x = *std::max_element( dX.begin(), dX.end() );
		v2.y = *std::max_element( dY.begin(), dY.end() );
		dX.clear();
		dY.clear();

		topLeft.x     = std::min( v1.x, topLeft.x );
		topLeft.y     = std::min( v1.y, topLeft.y );
		bottomRight.x = std::max( v2.x, topLeft.x );
		bottomRight.y = std::max( v2.y, topLeft.y );

	}

	mMapRect = Rect( topLeft, bottomRight - topLeft );

}


/*
 * Method: Rect GetMapRect();
 * Description: Gets the bounds of the road network.
 */
Rect UrcData::GetMapRect() {
	return mMapRect;
}


/*
 * Method: Link *GetLink( int index );
 * Description: Gets a pointer to a link of the given index.
 */
UrcData::Link *UrcData::GetLink( int index ) {
	return &mLinkSet[ index ];
}




/*
 * Method: Node *GetNode( int index );
 * Description: Gets a pointer to a node of the given index.
 */
UrcData::Node *UrcData::GetNode( int index ) {
	return &mNodeSet[ index ];
}




/*
 * Method: Link *GetSummedLink( int index );
 * Description: Gets a pointer to a summed link of the given index.
 */
UrcData::Link *UrcData::GetSummedLink( int index ) {
	return &mSummedLinkSet[ index ];
}




/*
 * Method: Classification GetClassification( int l1, int l2 );
 * Description: Get the CORNER classification between the given links.
 */
UrcData::Classification UrcData::GetClassification( int l1, int l2 ) {

	OrderedIndexPair linkPair(l1,l2);
	Classification c;

	ClassificationMap::iterator cm = mClassificationMap.find(linkPair);
	if ( cm != mClassificationMap.end() ) {

		c = cm->second;
		c.mFlipped = ( c.mLinkPair.first != l1 );

	} else {

		c.mClassification = Classifier::OutOfRange;
		c.mFullNodeCount = INT_MAX;

	}
	return c;

}


/*
 * Method: Classification GetClassification( std::string link1, std::string link2 );
 * Description: Get the CORNER classification between the given links (by name).
 */
UrcData::Classification UrcData::GetClassification( std::string link1, std::string link2 ) {

	return GetClassification( mLinkIndexMap[link1], mLinkIndexMap[link2] );

}



/**
 *	Get the classification and k factor between the given points.
 */
UrcData::Classification UrcData::GetClassification( std::string txName, std::string rxName, Vector2D txPos, Vector2D rxPos ) {

	int txIndex = 0;
	bool txHasMapping = LinkHasMapping( txName, &txIndex ); 
	int rxIndex = 0;
	bool rxHasMapping = LinkHasMapping( rxName, &rxIndex ); 

	if ( txHasMapping && rxHasMapping ) {
		// We have a mapping for both links
		return GetClassification( txIndex, rxIndex );
	}

	// otherwise, we may have one car on an internal edge
	if ( txHasMapping != rxHasMapping ) {

		if ( !txHasMapping )
			return GetClassificationFromOneInternal( txName, rxIndex, txPos, rxPos );
		if ( !rxHasMapping )
			return GetClassificationFromOneInternal( rxName, txIndex, txPos, rxPos );

	}

	// otherwise we have both cars on internal links.
	return GetClassificationFromInternalLinks( txName, rxName, txPos, rxPos );

}




/**
 *	Refine the given classification based on the position of vehicles.
 */
void UrcData::RefineClassification( UrcData::Classification &cls, VectorMath::Vector2D &s, VectorMath::Vector2D &d ) {

	if ( cls.mClassification == Classifier::LOS ) {

		return;	// Change nothing.

	} else if ( cls.mClassification == Classifier::NLOS1 ) {

		// If we're in NLOS1, then we need to check if either the source or destination are near enough to the common node to be in LOS.
		// Get the position of the common node.
		Vector2D &commonNode = GetNode( cls.mNodeSet[0] )->position;
		Real sDist, dDist;
		if ( cls.mFlipped ) {
			sDist = GetSummedLink( cls.mLinkPair.second )->NumberOfLanes*mLaneWidth*0.5;
			dDist = GetSummedLink( cls.mLinkPair.first  )->NumberOfLanes*mLaneWidth*0.5;
		} else {
			sDist = GetSummedLink( cls.mLinkPair.first  )->NumberOfLanes*mLaneWidth*0.5;
			dDist = GetSummedLink( cls.mLinkPair.second )->NumberOfLanes*mLaneWidth*0.5;
		}

		if ( commonNode.DistanceSq( s ) < sDist*sDist || commonNode.Distance( d ) < dDist*dDist )
			cls.mClassification = Classifier::LOS;

		return;

	} else {

		// We're in NLOS2. We need to be a bit more tricky here.
		// We have to get the common link. Then, if both source and destination are near enough to either of the common link's nodes,
		// we then set them in LOS. If only one is close to the common link's node, then they're in NLOS1.

		Node *n1 = GetNode( cls.mNodeSet[0] );
		Node *n2 = GetNode( cls.mNodeSet[1] );

		// Find the common link between them.
		LinkIndexSet commonLinkSet;
		LinkIndexSet::iterator linkIt = std::set_intersection( n1->mConnectedLinks.begin(), n1->mConnectedLinks.end(), n2->mConnectedLinks.begin(), n2->mConnectedLinks.end(), commonLinkSet.begin() );
		commonLinkSet.resize( linkIt - commonLinkSet.begin() );

#ifdef DEBUG
		if ( commonLinkSet.size() == 0 ) {
			// Found more or less than 1 common link. Bailing out and leaving as is.
			std::cerr << "Found no common link between nodes " << cls.mNodeSet[0] << " and " << cls.mNodeSet[1] << ". Bailing out and leaving as is.\n";
			return;
		}

		if ( commonLinkSet.size() > 1 ) {
			// Found more than 1 common link. Bailing out and leaving as is.
			std::cerr << "Found more than 1 common link between nodes " << cls.mNodeSet[0] << " and " << cls.mNodeSet[1] << ". Bailing out and leaving as is.\n";
			return;
		}

#else // #ifdef DEBUG

		if ( commonLinkSet.size() != 1 ) {
			// Found more or less than 1 common link. Bailing out and leaving as is.
			return;
		}

#endif // #ifdef DEBUG

		// Calculate the minimum distance we need to be within.
		Real dist = pow( GetSummedLink( *linkIt )->NumberOfLanes*mLaneWidth*0.5, 2 );

		// Now work out how far from the nodes we are.
		Real sDist, dDist;
		if ( cls.mFlipped ) {
			sDist = n2->position.DistanceSq( s );
			dDist = n1->position.DistanceSq( d );
		} else {
			sDist = n1->position.DistanceSq( s );
			dDist = n2->position.DistanceSq( d );
		}

		if ( sDist <= dist && dDist <= dist )
			cls.mClassification = Classifier::LOS;			// The two nodes are in the middle of their respective intersections, and are thus in LOS.
		else if ( ( sDist <= dist ) != ( dDist <= dist ) )
			cls.mClassification = Classifier::NLOS1;		// One of the nodes is not in the middle of its intersection, but one is. Thus they are in NLOS1.

		return;

	}

}




/*
 * Method: VectorMath::Real GetK( LinkPair p, Vector2D srcPos, Vector2D destPos );
 * Description: Get the pre-computed k-factor between the given source and destination.
 */
Real UrcData::GetK( OrderedIndexPair p, Vector2D srcPos, int srcLane, Vector2D destPos, int destLane, bool flipped ) {

	// Get source and destination link IDs
	unsigned int sourceLink = ( flipped ? p.second :  p.first );
	unsigned int destLink   = ( flipped ?  p.first : p.second );

	// Get pointers to the link data structures.
	Link *pSource = GetSummedLink( sourceLink );
	Link *pDest   = GetSummedLink(   destLink );

	// Calculate how far along the links each position is. This rounds to nearest integer.
	unsigned int sourcePos	   = floor( GetNode( pSource->nodeAindex )->position.Distance(  srcPos ) / mLengthIncrement + 0.5 );
	unsigned int destinationPos = floor( GetNode(   pDest->nodeAindex )->position.Distance( destPos ) / mLengthIncrement + 0.5 );

	// TODO: the lane indexing isn't quite right due to the summing of links in both directions.
	// TODO: See if you can think of a way to fix this. Maybe rework the raytracer to consider links in both directions...

	if ( sourceLink >= mRiceFactorData.size() )
		return 0;	// Don't know this link, so assume Rayleigh.

	SourceLocationList &srcLocList = mRiceFactorData[sourceLink];
	if ( sourcePos >= srcLocList.size() )
		return 0;	// Non-indexable position on source link, so assume Rayleigh.

	SourceLaneList &srcLaneList = srcLocList[sourcePos];
	if ( srcLane >= srcLaneList.size() )
		return 0;	// Non-indexable lane on source link, so assume Rayleigh.

	DestinationLookup &destLookup = srcLaneList[srcLane];
	if ( destLookup.find( destLink ) != destLookup.end() )
		return 0;	// No connection between this source and destination, so assume Rayleigh.

	DestinationLocationList &destLocList = destLookup[destLink];
	if ( destinationPos >= destLocList.size() )
		return 0;	// Non-indexable position on destination link, so assume Rayleigh.

	DestinationLaneList &destLaneList = destLocList[destinationPos];
	if ( destLane >= destLaneList.size() )
		return 0;	// Non-indexable lane on destination link, so assume Rayleigh.

	// Now index the lookup.
	return destLaneList[destLane];

}


/*
 * Method: bool LinkIsInternal( std::string linkName, LinkIndexSet **pLinkIndices );
 * Description: Returns true if the given link name is an internal link, and returns a pointer to the parent node's connected links.
 */
bool UrcData::LinkIsInternal( std::string linkName, LinkIndexSet **pLinkIndices ) {

	if ( linkName[0] != ':' )
		return false;

	(*pLinkIndices) = &mNodeSet[ mInternalLinkIndexMap[ linkName ] ].mConnectedLinks;

	return true;

}


/*
 * Method: bool LinkHasMapping( std::string linkName, int *pMapping );
 * Description: Returns true if the given link name is mapped to an index.
 */
bool UrcData::LinkHasMapping( std::string linkName, int *pMapping ) {

	bool hasMapping = mLinkIndexMap.find(linkName) != mLinkIndexMap.end();
	if ( hasMapping && pMapping )
		*pMapping = mLinkIndexMap[linkName];
	return hasMapping;

}


/*
 * Method: Building *GetBuilding( int index );
 * Description: Get the building at the given index.
 */
UrcData::Building *UrcData::GetBuilding( int index ) {
	return &mBuildingSet[index];
}


/*
 * Method: void GetGrid(Vector2D position) {
 * Description: Gets the grid associated with the specified position
 */
UrcData::Grid* UrcData::GetGrid(Vector2D position) {
	return( &(mGridList[int(position.y/mGridSize)][int(position.x/mGridSize)]) );
	
}

/*
 * Method: void LoadNetwork( char* linksFile, char* nodesFile, const char* classFile, const char* buildingFile, const char* linkMapFile, const char* intLinkMapFile, const char* riceDataFile, const char* carDefFile );
 * Description: Loads the data from the links and nodes files.
 */
void UrcData::LoadNetwork( const char* linksFile, const char* nodesFile, const char* classFile, const char* buildingFile, const char* linkMapFile, const char* intLinkMapFile, const char* riceDataFile, const char* carDefFile ) {
	ifstream stream;
	char buffer[20];

	int numNodesInFile, numLinksInFile, numClassInFile, numBuildingsInFile, numLinkMappings, numRice, numCars;
	Vector2D topLeft, bottomRight;
	UrcData::Node tempNode;
	UrcData::Link tempLink;

	// read the nodes file
	if ( nodesFile ) { 

		stream.open( nodesFile );
		if ( stream.fail() ) {
			THROW_EXCEPTION( "Cannot open nodes file: %s", nodesFile );
		}

		stream >> dec >> numNodesInFile;

		mNodeSet.reserve(numNodesInFile);

		for(int n = 0; n < numNodesInFile; n++) {
			stream >> tempNode.index >> tempNode.position.x >> tempNode.position.y;
			if ( topLeft.x > tempNode.position.x )
				topLeft.x = tempNode.position.x;
			if ( bottomRight.x < tempNode.position.x )
				bottomRight.x = tempNode.position.x;
			if ( topLeft.y > tempNode.position.y )
				topLeft.y = tempNode.position.y;
			if ( bottomRight.y < tempNode.position.y )
				bottomRight.y = tempNode.position.y;
			mNodeSet.push_back(tempNode);
		}
	
		stream.close();

	}

	// read the links file
	if ( linksFile ) {

		stream.open( linksFile );
		if ( stream.fail() ) {
			THROW_EXCEPTION( "Cannot open links file: %s", linksFile );
		}

		stream >> dec >> numLinksInFile;

		mLinkSet.reserve(numLinksInFile);
		for(int l = 0; l < numLinksInFile; l++)
		{
			stream >> tempLink.index
				>> tempLink.nodeAindex
				>> tempLink.nodeBindex
				>> tempLink.NumberOfLanes
				>> buffer // Boarder segment is not in use
				>> tempLink.flow
				>> tempLink.speed;
			mLinkSet.push_back(tempLink);
		}

		stream.close();

	}

	// read the classification files
	if ( classFile ) {

		stream.open( classFile );
		if ( stream.fail() ) {
			THROW_EXCEPTION( "Cannot open classification file: %s", classFile );
		}

		stream >> dec >> numClassInFile;
		int link1, link2;

		Classification tempClass;
		for(int c = 0; c < numClassInFile; c++ ) {

			stream >> link1 >> link2 >> tempClass.mClassification >> tempClass.mFullNodeCount;

			if ( tempClass.mClassification == Classifier::NLOS1 || tempClass.mClassification == Classifier::NLOS2 ) {
				stream >> tempClass.mMainStreetLaneCount;
				stream >> tempClass.mSideStreetLaneCount;
				if ( tempClass.mClassification == Classifier::NLOS2 )
					stream >> tempClass.mParaStreetLaneCount;
			}

			if ( tempClass.mClassification != Classifier::LOS ) {

				for ( int n = 0; n < tempClass.mClassification; n++ ) {

					stream >> tempClass.mNodeSet[ n ];

				}

			}

			tempClass.mLinkPair = VectorMath::OrderedIndexPair(link1,link2);
			mClassificationMap[ tempClass.mLinkPair ] = tempClass;

		}

		stream.close();

	}

	// read the building files
	if ( buildingFile ) {

		stream.open( buildingFile );
		if ( stream.fail() ) {
			THROW_EXCEPTION( "Cannot open building file: %s", buildingFile );
		}

		stream >> dec >> numBuildingsInFile;
		int vertexCount, tmp;
		Vector2D v1, v2, v3;

		Building tempBuilding;
		for(int c = 0; c < numBuildingsInFile; c++ ) {

			tempBuilding.mId = c;
			stream >> tmp >> tempBuilding.mPermitivity >> tempBuilding.mMaxHeight >> tempBuilding.mHeightStdDev >> vertexCount >> v1.x >> v1.y;
			v3 = v1;
			for ( int v = 0; v < vertexCount-1; v++ ) {

				if ( topLeft.x > tempNode.position.x )
					topLeft.x = tempNode.position.x;
				if ( bottomRight.x < tempNode.position.x )
					bottomRight.x = tempNode.position.x;
				if ( topLeft.y > tempNode.position.y )
					topLeft.y = tempNode.position.y;
				if ( bottomRight.y < tempNode.position.y )
					bottomRight.y = tempNode.position.y;

				stream >> v2.x >> v2.y;
				tempBuilding.mEdgeSet.push_back( LineSegment( v1, v2 ) );
				v1 = v2;

			}

			tempBuilding.mEdgeSet.push_back( LineSegment( v1, v3 ) );
			mBuildingSet.push_back( tempBuilding );
			tempBuilding.mEdgeSet.clear();

		}

		stream.close();

	}

	// read the link mappings
	if ( linkMapFile ) {

		stream.open( linkMapFile );
		if ( stream.fail() ) {
			THROW_EXCEPTION( "Cannot open link mapping file: %s", linkMapFile );
		}

		stream >> dec >> numLinkMappings;
		for ( int c = 0; c < numLinkMappings; c++ ) {

			std::string strTmp;
			int index;
			stream >> strTmp >> index;
			mLinkIndexMap[ strTmp ] = index;

		}

		stream.close();

	}

	
	// read the internal link mappings
	if ( intLinkMapFile ) {

		stream.open( intLinkMapFile );
		if ( stream.fail() ) {
			THROW_EXCEPTION( "Cannot open internal link mapping file: %s", intLinkMapFile );
		}

		stream >> dec >> numLinkMappings;
		for ( int c = 0; c < numLinkMappings; c++ ) {

			std::string strTmp;
			int index;
			stream >> strTmp >> index;
			mInternalLinkIndexMap[ strTmp ] = index;

		}

		stream.close();

	}

	// read the car definitions
	if ( carDefFile ) {

		stream.open( carDefFile );
		if ( stream.fail() ) {
			THROW_EXCEPTION( "Cannot open car definitions file: %s", carDefFile );
		}

		stream >> dec >> numCars;
		for ( int c = 0; c < numCars; c++ ) {

			std::string strName, strCol;
			Real w;
			stream >> strName;
			stream >> mCarDefinitions[strName].mAcceleration
				   >> mCarDefinitions[strName].mDeceleration
				   >> mCarDefinitions[strName].mDriverImperfection
				   >> mCarDefinitions[strName].mLength
				   >> strCol
				   >> mCarDefinitions[strName].mWidth
				   >> mCarDefinitions[strName].mHeight
				   >> w;

		}

		stream.close();

	}


	// read the pre-computed K-factors
	if ( riceDataFile ) {

		stream.open( riceDataFile );
		if ( stream.fail() ) {
			THROW_EXCEPTION( "Cannot open Rice datafile: %s", riceDataFile );
		}

		stream >> dec >> mLengthIncrement;
		stream >> dec >> numRice;

		for ( int r = 0; r < numRice; r++ ) {

			// Read the index of the source link and number of locations.
			SourceLocationList srcLocList;
			int srcId, srcLocCount;
			stream >> srcId >> srcLocCount;
			for ( int srcLoc = 0; srcLoc < srcLocCount; srcLoc++ ) {

				// Read the number of source lanes.
				SourceLaneList srcLaneList;
				int srcLaneCount;
				stream >> srcLaneCount;
				for ( int srcLane = 0; srcLane < srcLaneCount; srcLane++ ) {

					// Read the number of destination links.
					DestinationLookup destLookup;
					int destLinkCount;
					stream >> destLinkCount;
					for ( int destLink = 0; destLink < destLinkCount; destLink++ ) {

						// Read the index of the destination link and the number of locations therein.
						DestinationLocationList destLocList;
						int destId, destLocCount;
						stream >> destId >> destLocCount;
						for ( int destLoc = 0; destLoc < destLocCount; destLoc++ ) {

							// Read the number of destination lanes.
							DestinationLaneList newDestLane;
							int destLaneCount;
							stream >> destLaneCount;
							for ( int destLane = 0; destLane < destLaneCount; destLane++ ) {

								// Read the K-Factor
								std::string kStr;
								stream >> kStr;
								if ( "inf" == kStr )
									newDestLane.push_back( DBL_MAX );
								else
									newDestLane.push_back( atof( kStr.c_str() ) );

							}

							destLocList.push_back( newDestLane );

						}

						destLookup[destId] = destLocList;

					}

					srcLaneList.push_back( destLookup );

				}

				srcLocList.push_back( srcLaneList );

			}

			mRiceFactorData.push_back( srcLocList );

		}

    }

   
	mMapRect.location = topLeft;
	mMapRect.size = bottomRight - topLeft;

}





/*
 * Method: void ComputeSummedLinkSet();
 * Description: Takes the link set from the file, and calculates a reduced set
 * 				comprised of links that are the only connections between node pairs.
 */
void UrcData::ComputeSummedLinkSet() {

	// Note: Not sure if this is necessary now, since the Corner python class performs the link reduction.
	
	LinkSet::iterator linkIt;
	std::map< std::pair<int,int>, int > nodePairMapLinkIndex;
	std::pair<int,int> n1, n2;
	int lIndex = 0;
	Link newLink;

	for ( AllInVector( linkIt, mLinkSet ) ) {

		n1.first = n2.second = linkIt->nodeAindex;
		n2.first = n1.second = linkIt->nodeBindex;

		if ( nodePairMapLinkIndex.find( n1 ) != nodePairMapLinkIndex.end() ) {
			mSummedLinkSet[ nodePairMapLinkIndex[ n1 ] ].NumberOfLanes += linkIt->NumberOfLanes;
		} else if ( nodePairMapLinkIndex.find( n2 ) != nodePairMapLinkIndex.end() ) {
			mSummedLinkSet[ nodePairMapLinkIndex[ n2 ] ].NumberOfLanes += linkIt->NumberOfLanes;
		} else {
			newLink.index = lIndex;
			newLink.NumberOfLanes = linkIt->NumberOfLanes;
			newLink.nodeAindex = linkIt->nodeAindex;
			newLink.nodeBindex = linkIt->nodeBindex;
			mSummedLinkSet.push_back( newLink );
			mNodeSet[ n1.first  ].mConnectedLinks.push_back( lIndex );
			mNodeSet[ n1.second ].mConnectedLinks.push_back( lIndex );
			nodePairMapLinkIndex[ n1 ] = lIndex;
			lIndex++;
		}

	}

	nodePairMapLinkIndex.clear();

}



/*
 * Method: void ComputeBuckets();
 * Description: Fills the buckets with indices of building edges.
 */
void UrcData::ComputeBuckets() {

	unsigned int i, j;

	if ( mMapRect.size.x > mBucketSize * SINCOS45 )
		mBucketX = ceil( mMapRect.size.x / mBucketSize - SINCOS45 );
	else
		mBucketX = 1;

	if ( mMapRect.size.y > mBucketSize * SINCOS45 )
		mBucketY = ceil( mMapRect.size.y / mBucketSize - SINCOS45 );
	else
		mBucketY = 1;

	// compute buckets
	m_ppBuckets = new Bucket*[mBucketX];
	for ( i = 0; i < mBucketX; i++ )
		m_ppBuckets[i] = new Bucket[mBucketY];

	mCentroid = ( mMapRect.size - Vector2D( mBucketX-1, mBucketY-1 ) * mBucketSize ) * 0.5;

	for ( i = 0; i < mBucketX; i++ ) {
		for ( j = 0; j < mBucketY; j++ ) {
			Vector2D c = mCentroid + Vector2D( i, j ) * mBucketSize;
			for ( BuildingSet::iterator it = mBuildingSet.begin(); it != mBuildingSet.end(); it++ ) {
				for ( LineSet::iterator edgeIt = it->mEdgeSet.begin(); edgeIt != it->mEdgeSet.end(); edgeIt++ ) {
					if ( edgeIt->IntersectCircle( c, mBucketSize ) ) {
						m_ppBuckets[i][j].push_back( it->mId );
						break;
					}
				}
			}
		}
	}

	Real temp = mMapRect.size.y / mGridSize;
	if (temp <= 0) {
		mGridRowCount = 1;
	} else {
		mGridRowCount = (unsigned int)ceil(temp);
	}
	
	temp = mMapRect.size.x / mGridSize;
	if (temp <= 0) {
		mGridColumnCount = 1;
	} else {
		mGridColumnCount = (unsigned int)ceil(temp);
	}
	
	mGridList = new Grid*[mGridRowCount];
	if (!mGridList) {
		THROW_EXCEPTION ("Could not allocate memory for microBuckets!");
	}
	for (i=0; i<mGridRowCount; i++) {
		mGridList[i] = new Grid[mGridColumnCount];
		for (j=0; j<mGridColumnCount; j++) {
			mGridList[i][j].gridRect = Rect(j*mGridSize, i*mGridSize, mGridSize, mGridSize);
		}
	}
	
	LinkSet::iterator linkIt;
	VectorMath::Rect largerRect;
	for(i=0; i<mGridRowCount; i++) {
		for (j=0; j<mGridColumnCount; j++) {
			for (AllInVector(linkIt, mSummedLinkSet)) {
				//if the link lies within the grid rectangle, add it to the grid
				if (mGridList[i][j].gridRect.LineSegmentWithin(LineSegment(mNodeSet[linkIt->nodeAindex].position, mNodeSet[linkIt->nodeBindex].position))) {
					mGridList[i][j].linkList.push_back(linkIt->index);
				}
			}
		}
	}
}



/*
 * Method: VectorMath::Vector3D GetVehicleTypeDimensions( std::string );
 * Description: Get the width (x), length (y), and height (z) of vehicles of the given class.
 */
Vector3D UrcData::GetVehicleTypeDimensions( std::string strName ) {

	return Vector3D( mCarDefinitions[strName].mWidth, mCarDefinitions[strName].mLength, mCarDefinitions[strName].mHeight );

}




void UrcData::CollectBucketsInRange( VectorMath::Real r, VectorMath::Vector2D p, Bucket *pBucket ) {

	unsigned int i, j;
	for ( i = 0; i < mBucketX; i++ ) {
		for ( j = 0; j < mBucketY; j++ ) {
			Vector2D c = mCentroid + Vector2D( i, j ) * mBucketSize;
			Real dist = ( p - c ).Magnitude();
			if ( dist < mBucketSize )
				pBucket->insert( pBucket->end(), m_ppBuckets[i][j].begin(), m_ppBuckets[i][j].end() );
		}
	}

}




UrcData::Classification UrcData::GetClassificationFromOneInternal( std::string internalName, int otherIndex, Vector2D txPos, Vector2D rxPos ) {

	LinkIndexSet *pSet;
	bool isInternal = LinkIsInternal( internalName, &pSet );

	LinkIndexSet::iterator it;
	Classification bestClass;
	int internalIndex = -1;

	bestClass.mClassification = Classifier::OutOfRange;
	for ( AllInVector( it, (*pSet) ) ) {

		Classification c = GetClassification( *it, otherIndex );
		if ( c.mClassification <= bestClass.mClassification ) {

			bestClass = c;
			internalIndex = *it;

		}

	}

	return bestClass;

}


UrcData::Classification UrcData::GetClassificationFromInternalLinks( std::string txName, std::string rxName, Vector2D txPos, Vector2D rxPos ) {

	LinkIndexSet *pTxSet, *pRxSet;
	bool txIsInternal = LinkIsInternal( txName, &pTxSet );
	bool rxIsInternal = LinkIsInternal( rxName, &pRxSet );

	LinkIndexSet::iterator txIt, rxIt;
	Classification bestClass;
	int txIndex = -1, rxIndex = -1;

	bestClass.mClassification = Classifier::OutOfRange;

	for ( AllInVector( txIt, (*pTxSet) ) ) {

		for ( AllInVector( rxIt, (*pRxSet) ) ) {

			Classification c = GetClassification( *txIt, *rxIt );
			if ( c.mClassification <= bestClass.mClassification ) {

				bestClass = c;
				txIndex = *txIt;
				rxIndex = *rxIt;

			}

		}

	}

	return bestClass;

}

