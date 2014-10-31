//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __URC_CARMOBILITY_H_
#define __URC_CARMOBILITY_H_

#include <omnetpp.h>
#include "TraCIMobility.h"
#include "UrcData.h"

typedef std::list<std::string> Route;


class StatusChangeListener {
public:
	virtual void         LaneChanged(        int newLane ) = 0;
	virtual void CrossedIntersection( std::string roadId ) = 0;
};


/**
 * CarMobility.
 * This class keeps track of the grid cell the car is located in.
 * This helps the shadowing model exclude vehicles not directly located
 * between sender and receiver.
 */
class CarMobility : public TraCIMobility
{

public:
	/** Default constructor */
	CarMobility();
	/** Default destructor */
	virtual ~CarMobility();

	void SetListener( StatusChangeListener *listener ) { mListener = listener; }

	/** Get the current lane. */
	int getLaneId() { return mLaneID; }

	/** Get the TOLIC. */
	simtime_t getTOLIC() { return mTOLIC; }

	/** Get the current grid cell of this car. */
	const Coord getGridCell() const;

	/** Get the name of the car's type. */
	const std::string& getCarType() const;

	/** Get the dimensions of the car. */
	const VectorMath::Vector3D& getCarDimensions() const;

	/** Get the route of this car. */
	const Route& getRoute() const;

	/** Next position of the car. */
	virtual void nextPosition(const Coord& position, std::string road_id = "", double speed = -1, double angle = -1, TraCIScenarioManager::VehicleSignal signals = TraCIScenarioManager::VEH_SIGNAL_UNDEF);

	/** Handle position change. */
	virtual void changePosition();

	/** Initialise car parameters. */
	virtual void initialize( int );

protected:

	/** Fetch the ID of the lane the car is in. */
	void updateLane();

	int mLaneID;							/**< The ID of the lane this car is on. */
	Coord mGridCell;						/**< The grid cell in which this car is located. */
	std::string mCarType;					/**< The type of car. */
	VectorMath::Vector3D mCarDimensions;	/**< The dimensions of the vehicle. */
	Route mThisRoute;						/**< The route of this car. */
	simtime_t mTOLIC;						/**< Time Of Last Intersection Crossing. */

	StatusChangeListener *mListener;		/**< Methods of this class are called when the lane changes or the car crosses an intersection. */

};

#endif
