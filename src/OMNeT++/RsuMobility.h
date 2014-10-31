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

#ifndef __URC_RSUMOBILITY_H_
#define __URC_RSUMOBILITY_H_

#include <omnetpp.h>
#include "ConstSpeedMobility.h"

/**
 * RsuMobility.
 * This class contains a height and road ID for an RSU.
 */
class RsuMobility : public ConstSpeedMobility
{

public:
	double getHeight() { return mHeight; }
	std::string getRoadId() { return mRoadId; }
	int getLaneId() { return 0; }

protected:
    virtual void initialize(int);

    double mHeight;			/**< The height in metres of this RSU off the ground. */
    std::string mRoadId;	/**< The id of the road in Sumo this RSU is on. */

};

#endif
