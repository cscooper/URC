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


#include "CarMobility.h"
#include "UrcScenarioManager.h"



Define_Module(CarMobility);


CarMobility::CarMobility() {

	mGridCell.x = -1;
	mGridCell.y = -1;

}


CarMobility::~CarMobility() {

	UrcScenarioManagerAccess().get()->updateModuleGrid( this, mGridCell, Coord(-1,-1,0) );

}


const Coord CarMobility::getGridCell() const {

	return mGridCell;

}


const std::string& CarMobility::getCarType() const {

	return mCarType;

}



const VectorMath::Vector3D& CarMobility::getCarDimensions() const {

	return mCarDimensions;

}



/** Get the route of this car. */
const Route& CarMobility::getRoute() const {

	return mThisRoute;

}



void CarMobility::nextPosition(const Coord& position, std::string road_id, double speed, double angle, TraCIScenarioManager::VehicleSignal signals ) {

	if ( this->road_id != road_id ) {
		updateLane();
		// Remove the old road ID from the route.
		mThisRoute.pop_front();
		mTOLIC = simTime();
		if ( mListener )
			mListener->CrossedIntersection( mThisRoute.front() );
	}

	TraCIMobility::nextPosition( position, road_id, speed, angle, signals );

}



void CarMobility::changePosition() {

	TraCIMobility::changePosition();
	UrcScenarioManager *pManager = UrcScenarioManagerAccess().get();

	double dim = pManager->getGridSize();
	Coord lastPos = mGridCell;
	mGridCell.x = (int)(fabs(move.getStartPos().x) / dim);
	mGridCell.y = (int)(fabs(move.getStartPos().y) / dim);

	if ( mGridCell.x >= pManager->getGridWidth() )
		mGridCell.x = pManager->getGridWidth()-1;

	if ( mGridCell.y >= pManager->getGridHeight() )
		mGridCell.y = pManager->getGridHeight()-1;

	if ( lastPos != mGridCell )
		UrcScenarioManagerAccess().get()->updateModuleGrid( this, lastPos, mGridCell );
	else
		mGridCell = lastPos;

}



void CarMobility::initialize( int stage ) {

	TraCIMobility::initialize( stage );

	if ( stage == 0 ) {

		mCarType = UrcScenarioManagerAccess().get()->commandGetVehicleType( getExternalId() );
		mCarDimensions = Urc::UrcData::GetSingleton()->GetVehicleTypeDimensions( mCarType );

        // Get the route of this car.
		std::string myRouteId = UrcScenarioManagerAccess().get()->commandGetRouteId( getExternalId() );
		mThisRoute = UrcScenarioManagerAccess().get()->commandGetRouteEdgeIds( myRouteId );
		this->road_id = mThisRoute.front();
		mTOLIC = simTime();
		mListener = NULL;

	}

}




/** Fetch the ID of the lane the car is in. */
void CarMobility::updateLane() {

	std::string s = UrcScenarioManagerAccess().get()->commandGetLaneId( getExternalId() );
	int i = s.find("_");
	mLaneID = atoi( s.substr( i+1 ).c_str() );
	if ( mListener )
		mListener->LaneChanged( mLaneID );

}





