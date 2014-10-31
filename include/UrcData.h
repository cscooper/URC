/*
 *  UrcData.h - Contains the building geometry and CORNER classifications for a sumo map.
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

#include "Singleton.h"
#include "VectorMath.h"
#include <list>
#include <map>

namespace Urc {

	/*
	 * Name: UrcData
	 * Inherits: Singleton
	 * Description: Container for data on road network. This includes:
	 * 					-> Nodes and links
	 * 					-> Building Edges
	 * 					-> Intersections
	 * 					-> Edge Index Buckets for easy look-up
	 * 					-> Classifications for CORNER
	 */
	class UrcData : public Singleton<UrcData> {
		
	public:

		/*
		 * Name: Link
		 * Description: Contains the data for one stretch of road
		 */
		struct Link {
			int index;			// Link index
			int nodeAindex;		// index of the start node
			int nodeBindex;		// index of the end node
			int NumberOfLanes;	// number of lanes on this road
			double flow;		// flow rate of traffic			(as yet unused)
			double speed;		// speed of cars on the road	(as yet unused)
		};

		typedef std::vector<int> LinkIndexSet;

		/*
		 * Name: Node
		 * Description: Contains the data for one intersection
		 */
		struct Node {
			int index;						// index of intersection
			VectorMath::Vector2D position;	// position of the intersection
			LinkIndexSet mConnectedLinks;	// set of links which connect to this node.
							// Note: This indexes the summed link set, NOT the other link set.
			VectorMath::Real mSize;			// size of the intersection (treated as a circle).
		};

		typedef std::vector< VectorMath::Vector2D > VectorSet;
		typedef std::vector< VectorMath::LineSegment > LineSet;
                
		struct Classification {
			VectorMath::OrderedIndexPair mLinkPair;	/**< The pair of links between which this classification is valid. */
			int mClassification;					/**< The classification. */
			int mNodeSet[2];						/**< The set of junctions in this classification. */
			int mFullNodeCount;						/**< The full number of nodes that had to be traversed to get this classification. */
			VectorMath::Real mMainStreetLaneCount;	/**< Number of lanes in the sidestreet for NLOS1/2 calculations. */
			VectorMath::Real mSideStreetLaneCount;	/**< Number of lanes in the sidestreet for NLOS1/2 calculations. */
			VectorMath::Real mParaStreetLaneCount;	/**< Number of lanes in the sidestreet for NLOS2 calculations. */
			bool mFlipped;							/**< This flag is set for a particular lookup instance to show whether the source and destination indices are flipped from what we entered. */
		};

		struct Building {
			long mId;
			LineSet mEdgeSet;
			VectorMath::Real mPermitivity;
			VectorMath::Real mMaxHeight;
			VectorMath::Real mHeightStdDev;
		};

		// typedefs
		typedef std::vector<Link> LinkSet;
		typedef std::vector<Node> NodeSet;
		typedef std::pair<int,int> LinkPair;
		typedef std::map< VectorMath::OrderedIndexPair, Classification > ClassificationMap;
		typedef std::vector< Building > BuildingSet;
		typedef std::vector<long> Bucket;
		typedef std::map<std::string,int> LinkIndexMap;
		typedef std::map<std::string,int> InternalLinkIndexMap;

		/*
		 * Name: Grid
		 * Description: Contains the rectangle representing the current grid and the list of links in the grid
		 *
		 */
		struct Grid {
			VectorMath::Rect gridRect;
			LinkIndexSet linkList;
		};

		typedef std::vector<double> DestinationLaneList;
		typedef std::vector<DestinationLaneList> DestinationLocationList;
		typedef std::map<int,DestinationLocationList> DestinationLookup;
		typedef std::vector<DestinationLookup> SourceLaneList;
		typedef std::vector<SourceLaneList> SourceLocationList;
		typedef std::vector<SourceLocationList> RiceFactorMap;

		struct CarDefinition {

			VectorMath::Real mAcceleration;
			VectorMath::Real mDeceleration;
			VectorMath::Real mDriverImperfection;
			VectorMath::Real mLength;
			VectorMath::Real mWidth;
			VectorMath::Real mHeight;
			// TODO: ADD MORE VEHICLE PARAMETERS IF NEEDED

		};

		typedef std::map<std::string,CarDefinition> CarDefinitionMap;

		// getters
		VectorMath::Real GetWavelength();
		VectorMath::Real GetLamdaBy4PiSq();
		VectorMath::Real GetTransmitPower();
		VectorMath::Real GetSystemLoss();
		VectorMath::Real GetReceiverSensitivity();
		VectorMath::Real GetFreeSpaceRange();
		VectorMath::Real GetLaneWidth();
		VectorMath::Real GetLossPerReflection();

		// blank constructor, giving empty UrcData
		UrcData( VectorMath::Real laneWidth, VectorMath::Real lambda, VectorMath::Real txPower, VectorMath::Real L, VectorMath::Real sensitivity, VectorMath::Real lpr, VectorMath::Real grid );

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
		UrcData( const char* linksFile, const char* nodesFile, const char* classFile, const char* buildingFile, const char* linkMapFile, VectorMath::Real laneWidth, VectorMath::Real lambda, VectorMath::Real txPower, VectorMath::Real L, VectorMath::Real sensitivity, VectorMath::Real lpr, VectorMath::Real grid );

		/*
		 * Constructor Arguments:
		 * 		1. linksFile - file name of the CORNER links file
		 * 		2. nodesFile - file name of the CORNER nodes file
		 * 		3commonNode.DistanceSq( s ). classFile - file name of the CORNER class file
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
		UrcData( const char* linksFile, const char* nodesFile, const char* classFile, const char* buildingFile, const char* linkMapFile, const char* intLinkMapFile, const char* riceDataFile, const char *carDefFile, VectorMath::Real laneWidth, VectorMath::Real lambda, VectorMath::Real txPower, VectorMath::Real L, VectorMath::Real sensitivity, VectorMath::Real lpr, VectorMath::Real grid );

		~UrcData();

		/*
		 * Method: void AddBuilding( Building newBuilding );
		 * Description: Manually add a building to the network
		 */
		void AddBuilding( Building newBuilding );
		
		/*
		 * Method: void CalculateMapRectangle();
		 * Description: Calculate map bounds after manually adding buildings.
		 */
		void CalculateMapRectangle();
		
		/*
		 * Method: VectorMath::Rect GetMapRect();
		 * Description: Gets the bounds of the road network.
		 */
		VectorMath::Rect GetMapRect();

		/*
		 * Method: Link *GetLink( int index );
		 * Description: Gets a pointer to a link of the given index.
		 */
		Link *GetLink( int index );

		/*
		 * Method: Node *GetNode( int index );
		 * Description: Gets a pointer to a node of the given index.
		 */
		Node *GetNode( int index );

		/*
		 * Method: Link *GetSummedLink( int index );
		 * Description: Gets a pointer to a summed link of the given index.
		 */
		Link *GetSummedLink( int index );

		/*
		 * Method: int GetSummedLinkCount();
		 * Description: Return the number of summed links.
		 */
		int GetSummedLinkCount() { return mSummedLinkSet.size(); }

		/*
		 * Method: void GetGrid(Vector2D position) {
		 * Description: Gets the grid associated with the specified position
		 */
		Grid* GetGrid(VectorMath::Vector2D position);

		/*
		 * Method: Classification GetClassification( int l1, int l2 );
		 * Description: Get the CORNER classification between the given links.
		 */
		Classification GetClassification( int l1, int l2 );

		/*
		 * Method: Classification GetClassification( std::string link1, std::string link2 );
		 * Description: Get the CORNER classification between the given links (by names).
		 */
		Classification GetClassification( std::string link1, std::string link2 );

		/**
		 *	Get the classification between the given points.
		 */
		Classification GetClassification( std::string txName, std::string rxName, VectorMath::Vector2D, VectorMath::Vector2D );

		/**
		 *	Refine the given classification based on the position of vehicles.
		 */
		void RefineClassification( Classification &cls, VectorMath::Vector2D &s, VectorMath::Vector2D &d );

		/*
		 * Method: VectorMath::Real GetK( LinkPair p, Vector2D srcPos, Vector2D destPos );
		 * Description: Get the pre-computed k-factor between the given source and destination.
		 */
		VectorMath::Real GetK( VectorMath::OrderedIndexPair p, VectorMath::Vector2D srcPos, int srcLane, VectorMath::Vector2D destPos, int destLane, bool flipped = false );

		/*
		 * Method: bool LinkIsInternal( std::string linkName, LinkIndexSet **pLinkIndices );
		 * Description: Returns true if the given link name is an internal link, and returns a pointer to the parent node's connected links.
		 */
		bool LinkIsInternal( std::string linkName, LinkIndexSet **pLinkIndices );

		/*
		 * Method: bool LinkHasMapping( std::string linkName, int *pMapping );
		 * Description: Returns true if the given link name is mapped to an index.
		 */
		bool LinkHasMapping( std::string linkName, int *pMapping );

		/*
		 * Method: Building *GetBuilding( int index );
		 * Description: Get the building at the given index.
		 */
		Building *GetBuilding( int index );
		
		/*
		 * Method: int GetBuildingCount();
		 * Description: Get the number of buildings.
		 */
		int GetBuildingCount() { return mBuildingSet.size(); }
		
		void CollectBucketsInRange( VectorMath::Real r, VectorMath::Vector2D p, Bucket* );

		/*
		 * Method: void LoadNetwork( char* linksFile, char* nodesFile, const char* classFile, const char* buildingFile, const char* linkMapFile, const char* intLinkMapFile, const char* riceDataFile, const char* carDefFile );
		 * Description: Loads the data from the links, nodes, classification, buildings, link map, internal link map, rice data files, and car definitions.
		 */
		void LoadNetwork( const char* linksFile, const char* nodesFile, const char* classFile, const char* buildingFile, const char* linkMapFile, const char* intLinkMapFile, const char* riceDataFile, const char* carDefFile );
		
		/*
		 * Method: void ComputeSummedLinkSet();
		 * Description: Takes the link set from the file, and calculates a reduced set
		 * 				comprised of links that are the only connections between node pairs.
		 */
		void ComputeSummedLinkSet();

		/*
		 * Method: void ComputeBuckets();
		 * Description: Fills the buckets with indices of building edges.
		 */
		void ComputeBuckets();

		/*
		 * Method: VectorMath::Vector3D GetVehicleTypeDimensions( std::string );
		 * Description: Get the width (x), length (y), and height (z) of vehicles of the given class.
		 */
		VectorMath::Vector3D GetVehicleTypeDimensions( std::string );

	protected:

		/**
		 * Get the classification between two positions when one link is internal.
		 */
		Classification GetClassificationFromOneInternal( std::string internalName, int otherIndex, VectorMath::Vector2D, VectorMath::Vector2D );

		/**
		 * Get the classification between two positions when both links are internal.
		 */
		Classification GetClassificationFromInternalLinks( std::string txName, std::string rxName, VectorMath::Vector2D, VectorMath::Vector2D );


		Bucket **m_ppBuckets;
		unsigned int mBucketX;
		unsigned int mBucketY;
		VectorMath::Vector2D mCentroid;
		VectorMath::Real mBucketSize;

		LinkSet mLinkSet;									// set of links loaded from file
		NodeSet mNodeSet;									// set of nodes loaded from file
		LinkSet mSummedLinkSet;								// link set calculated by summing lane counts of links sharing nodes
		LinkIndexMap mLinkIndexMap;							// mapping between link indices and link names
		InternalLinkIndexMap mInternalLinkIndexMap;			// mapping between internal link names and parent node indices

		VectorMath::Real mWavelength;						// wavelength of carrier signal
		VectorMath::Real mTransmitPower;					// transmission power
		VectorMath::Real mSystemLoss;						// sum of system losses (L)
		VectorMath::Real mSensitivity;						// receiver sensitivity

		VectorMath::Real mLossPerReflection;				// Loss of power per reflection

		VectorMath::Real mFreeSpaceRange;					// Free Space Transmission Range

		VectorMath::Vector2D mFirstCentroid;				// the centroid of the first bucket
		
		VectorMath::Real mLambdaBy4PiSq;					// pre-calculated lambda/4pi^2 since it is used a lot in classifer

		VectorMath::Real mGridSize;							//preferred size of the grid (assuming square)
		Grid **mGridList;									//list of grids in the map
		unsigned int mGridRowCount;
		unsigned int mGridColumnCount;

		VectorMath::Real mLaneWidth;						// Width of each lane
		VectorMath::Rect mMapRect;							// Rectangle showing bounds of the map network

		ClassificationMap mClassificationMap;				// classifications
		BuildingSet mBuildingSet;

		RiceFactorMap mRiceFactorData;						// map of pre-computed K-factors
		int mLengthIncrement;								// Increment between K-Factor calculations along the links.

		CarDefinitionMap mCarDefinitions;					// map of car definitions

	};


};
