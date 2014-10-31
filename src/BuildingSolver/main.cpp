/*
 *  main.cpp - Building Solver Algorithm
 *  Copyright (C) 2014  C. S. Cooper, A. Mukunthan
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
#include <cfloat>

#include "Urc.h"

#define SIGN(x) (x<0?-1:1)

using namespace Urc;
using namespace VectorMath;


struct NodeIDVectorPair {
	int mDestNodeID;					/**< ID of the destination node. */
	VectorMath::Vector2D mDirection;	/**< Direction to the target node. */
	double mAngle;						/**< Angle of the vector from positive +y. */
	double mRoadWidth;					/**< Half the width of the link between this node and the destination. */
};




// Sorting predicate for the node destination pairs.
bool VectorSortPredicate( const NodeIDVectorPair &n1, const NodeIDVectorPair &n2 ) {
	return n1.mAngle < n2.mAngle;
}


typedef std::vector< NodeIDVectorPair > VectorList;
typedef VectorList::iterator VectorIterator;
typedef std::map< int, VectorList > NodeVectorLookup;
typedef NodeVectorLookup::iterator NodeIterator;
typedef std::vector< Vector2D > VertexList;
typedef std::map< int, VertexList > NodeVertexLookup;
typedef std::vector< LineSegment > LineList;


struct Building {
	long mId;
	LineList mEdgeSet;
	VectorMath::Real mPermitivity;
	VectorMath::Real mMaxHeight;
	VectorMath::Real mHeightStdDev;
};

typedef std::vector< Building > BuildingList;


int main( int argc, char **ppArgv ) {

	if ( argc < 4 ) {
		std::cerr << "Usage: BuildingSolver <basefile> <lanewidth> <footPathWidth>\n";
		return -1;
	}

	UrcData *pUrc = NULL;
	std::string basename = ppArgv[1];
	int laneWidth = atoi(ppArgv[2]);
	double footPathWidth = atof(ppArgv[3]);

	// Initialise the URC singleton.
	try
	{

		pUrc = new UrcData(
			(basename+".corner.lnk").c_str(),
			(basename+".corner.int").c_str(),
			(basename+".corner.cls").c_str(),
			(basename+".corner.bld").c_str(),
			(basename+".corner.lnm").c_str(),
			15, 0.124378109, 1.1666, 1142.9, pow(10,-11), 0.25, 200
		);

	} catch( Exception &e ) {

		std::cerr << "Exception occurred: " << e.What() << "\n";
		return -1;

	}


	// First pass: Iterate through each link and compute vectors between it's nodes, and assign them to these nodes.
	NodeVectorLookup nodeLookup;
	NodeVertexLookup vertexLookup;
	int linkCount = pUrc->GetSummedLinkCount();
	int currLink = 0;

	for ( ; currLink < linkCount; currLink++ ) {

		UrcData::Link *pLink = pUrc->GetSummedLink( currLink );
		int nodeA = pLink->nodeAindex;
		int nodeB = pLink->nodeBindex;
		UrcData::Node *pNodeA = pUrc->GetNode( nodeA );
		UrcData::Node *pNodeB = pUrc->GetNode( nodeB );

		NodeIDVectorPair p;
		p.mRoadWidth = pLink->NumberOfLanes * laneWidth * 0.5 + footPathWidth;

		p.mDestNodeID = nodeB;
		p.mDirection = ( pNodeB->position - pNodeA->position ).Unitise();
		p.mAngle = p.mDirection.AngleBetween( Vector2D(0,1) );
		if ( p.mDirection.x < 0 )
			p.mAngle = 2*M_PI - p.mAngle;
		nodeLookup[nodeA].push_back( p );

		p.mDestNodeID = nodeA;
		p.mDirection = ( pNodeA->position - pNodeB->position ).Unitise();
		p.mAngle = p.mDirection.AngleBetween( Vector2D(0,1) );
		if ( p.mDirection.x < 0 )
			p.mAngle = 2*M_PI - p.mAngle;
		nodeLookup[nodeB].push_back( p );

	}

	// Second pass: Compute the location of building apexes.
	for ( NodeIterator it = nodeLookup.begin(); it != nodeLookup.end(); it++ ) {

		// Now, compute the vertices of the buildings at this intersection.
		UrcData::Node *currNode = pUrc->GetNode( it->first );
		if ( it->second.size() == 1 ) {

			// This is a dead end road.
			Vector2D v1 = it->second[0].mDirection;	// get the vector
			Vector2D v2 = Vector2D( v1.y, -v1.x );	// copy and rotate the copy by PI/2
			Vector2D v3 = v1 * -1;
			Vector2D c1 = currNode->position + ( v2 - v1 ).Unitise() * sqrt(2.0) * it->second[0].mRoadWidth;	// first corner point is on one side of the road
			Vector2D c2 = currNode->position + ( v3 - v1 ).Unitise() * sqrt(2.0) * it->second[0].mRoadWidth;	// the second is opposite it.
			vertexLookup[it->first].push_back( c1 );
			vertexLookup[it->first].push_back( c2 );

		} else {

			// Sort the vectors for the node.
			std::sort( it->second.begin(), it->second.end(), VectorSortPredicate );

			// Iterate over the vectors
			for ( VectorIterator vecIt = it->second.begin(); vecIt != it->second.end(); vecIt++ ) {

				// Compute the resolution between this vector and the next.
				Vector2D currVec = vecIt->mDirection;
				Vector2D nextVec;
				if ( vecIt+1 == it->second.end() )
					nextVec = it->second[0].mDirection;
				else
					nextVec = (vecIt+1)->mDirection;

				Real angle = currVec.AngleBetween( nextVec );

				/*
				 * Here's where it gets tricky.
				 * If the angle between them is just PI, no problem. We just rotate the first
				 * vector clockwise by PI/2 to get a perpendicular vector and calculate the point
				 * using that.
				 * However, if the angle is less than PI, we need to make sure we put the node in
				 * the right place. If we just did the what we did above, we'd get two corner
				 * points in the same place.
				 */
				Vector2D c;
				if ( angle == M_PI ) {

					// This is a straight line corner.
					c = currNode->position + ( Vector2D( currVec.y, -currVec.x ) ).Unitise() * it->second[0].mRoadWidth * sqrt(2.0);

				} else {

					// Calculate the resolved vector.
					Vector2D vRes = ( currVec + nextVec ) * it->second[0].mRoadWidth * sqrt(2.0);

					// Now we rotate the next vector by the angle between the current vector and the +y axis.
					Vector2D vTrans = Matrix2D::CreateRotation( vecIt->mAngle ) * nextVec;

					/*
					 * If the rotated vector is on the left-hand side of +y, then we need to change the direction
					 * of the translation to put the point on the opposite corner.
					 */
					if ( vTrans.x < 0 )
						c = currNode->position - vRes; // note the subtraction?
					else
						c = currNode->position + vRes;

				}

				vertexLookup[it->first].push_back( c );

			}

		}

	}

	// Third pass: Compute the building lines.
	LineList buildingEdges;
	for ( NodeIterator it = nodeLookup.begin(); it != nodeLookup.end(); it++ ) {

		 // Go through the connections between this node and others again. 
		 for ( int v = 0; v < it->second.size(); v++ ) {

			// We get the corresponding vertex from our list.
			Vector2D v1 = vertexLookup[it->first][v];

			// We then get the matching one from the destination one.
			// This is the vertex corresponding to the vector AFTER our complement.
			Vector2D v2;
			int complementIndex = -100;
			for ( VectorIterator vecIt = nodeLookup[it->second[v].mDestNodeID].begin(); vecIt != nodeLookup[it->second[v].mDestNodeID].end(); vecIt++ ) {
				if ( vecIt->mDestNodeID == it->first ) {
					complementIndex = vecIt - nodeLookup[it->second[v].mDestNodeID].begin() - 1;
					break;
				}
			}

			if ( complementIndex == -1 )
				complementIndex = nodeLookup[it->second[v].mDestNodeID].size()-1;

			v2 = vertexLookup[it->second[v].mDestNodeID][complementIndex];

			// Create a new line segment and add it to the list of building edges.
			buildingEdges.push_back( LineSegment(v1,v2) );

		}

	}

	// Fourth pass: Group the lines into buildings.
	BuildingList buildingList;
	Building newBuilding;

	// Prepare the first building from the first edge.
	newBuilding.mId = 0;
	newBuilding.mEdgeSet.push_back(buildingEdges[0]);
	buildingEdges.erase( buildingEdges.begin() );
	newBuilding.mPermitivity = 3.743;
	newBuilding.mMaxHeight = 0;
	newBuilding.mHeightStdDev = 0;

	// Now cycle until the list of edges is empty.
	while ( !buildingEdges.empty() ) {

		bool addBuilding = true;

		// Look through all unsorted edges.
		for ( LineList::iterator lineIt = buildingEdges.begin(); lineIt != buildingEdges.end(); lineIt++ ) {

			// Find if this edge is connected to the one at the back of the current building's edge set.
			if ( lineIt->PointInCommon( newBuilding.mEdgeSet.back() ) ) {

				// We have a common point, so add this to the building edge set.
				newBuilding.mEdgeSet.push_back( *lineIt );

				// Remove this from the list because it has been sorted now.
				buildingEdges.erase( lineIt );

				// Break the loop, telling the code to return to the start.
				addBuilding = false;
				break;

			}

		}

		if ( !addBuilding )
			continue;	// We added an edge to the building, so see if there's 

		// We found no more edges, so add this building to the list.
		buildingList.push_back( newBuilding );

		// Prepare the next building to go.
		newBuilding.mId++;
		newBuilding.mEdgeSet.clear();
		newBuilding.mEdgeSet.push_back(buildingEdges[0]);
		buildingEdges.erase( buildingEdges.begin() );

	}

	// Add the last building to the list.
	buildingList.push_back( newBuilding );

	// Save the  building list.
	std::ofstream outputStream;
	outputStream.open( (basename+".corner.bld").c_str() );
	outputStream << buildingList.size() << "\n";
	for ( BuildingList::iterator buildingIt = buildingList.begin(); buildingIt != buildingList.end(); buildingIt++ ) {

		outputStream << buildingIt->mId << " " << buildingIt->mPermitivity << " " << buildingIt->mMaxHeight << " " << buildingIt->mHeightStdDev << " " << buildingIt->mEdgeSet.size();
		for ( LineList::iterator lineIt = buildingIt->mEdgeSet.begin(); lineIt != buildingIt->mEdgeSet.end(); lineIt++ )
			outputStream << " " << lineIt->mStart.x << " " << lineIt->mStart.y;
		outputStream << "\n";

	}
	outputStream.close();

// 	// Let's see the data.
// 	al_init();
// 	al_init_primitives_addon();
// 	al_init_image_addon();
// 	al_init_font_addon();
// 	al_init_ttf_addon();
// 
// 	Rect r = pUrc->GetMapRect();
// 
// 	std::cerr << "Generating image...\n";
// 	ALLEGRO_BITMAP *bmp = al_create_bitmap( r.size.x, r.size.y );
// 	ALLEGRO_FONT *f = al_load_font( "ARIALUNI.TTF", 40, 0 );
// 	al_set_target_bitmap(bmp);
// 	double d = 100;
// 	double dw = ( r.size.x - 2*d ) / r.size.x;
// 	double dh = ( r.size.y - 2*d ) / r.size.y;
// 
// 	for ( BuildingList::iterator buildingIt = buildingList.begin(); buildingIt != buildingList.end(); buildingIt++ ) {
// 		double ax=0, ay=0;
// 		for ( LineList::iterator lineIt = buildingIt->mEdgeSet.begin(); lineIt != buildingIt->mEdgeSet.end(); lineIt++ ) {
// 			al_draw_line(d+lineIt->mStart.x*dw,d+lineIt->mStart.y*dh,d+lineIt->mEnd.x*dw,d+lineIt->mEnd.y*dh,al_map_rgb_f(1,1,1),5);
// 			ax += 2*d+(lineIt->mStart.x+lineIt->mEnd.x)*dw;
// 			ay += 2*d+(lineIt->mStart.y+lineIt->mEnd.y)*dh;
// 		}
// 		al_draw_textf( f, al_map_rgb_f(1,1,1), ax*0.5/buildingIt->mEdgeSet.size(), ay*0.5/buildingIt->mEdgeSet.size(), 0, "%d", buildingIt->mId );
// 	}


// 	ALLEGRO_COLOR cols[4];
// 	cols[0] = al_map_rgb_f(1,0,0); // Red
// 	cols[1] = al_map_rgb_f(1,1,0); // Yellow
// 	cols[2] = al_map_rgb_f(0,1,0); // Green
// 	cols[3] = al_map_rgb_f(0,1,1); // Cyan
// 
// 	for ( NodeVertexLookup::iterator nvIt = vertexLookup.begin(); nvIt != vertexLookup.end(); nvIt++ ) {
// 
// 		Vector2D p = pUrc->GetNode( nvIt->first )->position;
// 
// 		for ( VertexList::iterator vIt = nvIt->second.begin(); vIt != nvIt->second.end(); vIt++ ) {
// 
// 			int col = vIt-nvIt->second.begin();
// 			Vector2D v = p + nodeLookup[nvIt->first][col].mDirection*60;
// 			al_draw_filled_circle( d+vIt->x/dw, d+vIt->y/dh, 10, cols[col] );
// 			al_draw_line( d+p.x/dw, d+p.y/dh, d+v.x/dw, d+v.y/dh, cols[col], 15 );
// 
// 		}
// 
// 		al_draw_circle( d+p.x/dw, d+p.y/dh, 10, al_map_rgb_f(1,1,1), 2 );
// 
// 	}
	
// 	std::cerr << "Saving image...\n";
// 	if ( !al_save_bitmap( "building.bmp", bmp ) )
// 		std::cerr << "FAILED!\n";
// 	al_destroy_bitmap( bmp );
// 	al_destroy_font( f );
	
	delete pUrc;
	
	return 0;

}
