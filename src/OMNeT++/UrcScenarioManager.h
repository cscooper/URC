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

#ifndef URCSCENARIOMANAGER_H_
#define URCSCENARIOMANAGER_H_

#include <TraCIScenarioManagerLaunchd.h>

#include "Urc.h"
#include "CarMobility.h"


class UrcScenarioManager: public TraCIScenarioManagerLaunchd {

public:
	typedef std::vector<CarMobility*> GridCell;

	UrcScenarioManager();
	virtual ~UrcScenarioManager();

	virtual void initialize(int stage);
	virtual void finish();

	int getGridSize() const;
	int getGridWidth() const;
	int getGridHeight() const;

	void updateModuleGrid( CarMobility*, Coord, Coord );
	const GridCell& getGridCell( int x, int y ) const;

	std::string commandGetVehicleType(std::string vehicleId);
	std::string commandGetVehicleLaneId(std::string vehicleId);
	bool commandCreateRoute(std::string routeId,std::list<std::string> edgeList);

	std::map<std::string, cModule*> *getManagedHostsPtr();
	Coord ConvertCoords( Coord p, bool fromTraci = false );
	double ConvertAngle( double a, bool fromTraci = false );

	void SetFilenames( std::string linkFile, std::string nodeFile, std::string classFile, std::string linkMappingFile, std::string internalLinkMappingFile, std::string riceFile, std::string carDefinitionFile );


protected:
	Urc::UrcData *mUrcData;
	Urc::Fading *mFading;

	GridCell **mGridLookup;
	int mGridWidth;
	int mGridHeight;
	int mGridSize;

	bool mParametersFromConfig;					/**< If true, this gets the following filenames from the omnetpp.ini file. */
	std::string mLinkFile;
	std::string mNodeFile;
	std::string mClassificationFile;
	std::string mLinkMappingFile;
	std::string mInternalLinkMappingFile;
	std::string mRiceFile;
	std::string mCarDefinitionFile;

};



class UrcScenarioManagerAccess
{
	public:
	UrcScenarioManager* get() {
			return FindModule<UrcScenarioManager*>::findGlobalModule();
		};
};




#endif /* DATACOLLECTORSCENARIOMANAGER_H_ */
