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

#ifndef CARSHADOWMODEL_H_
#define CARSHADOWMODEL_H_

#include "MiXiMDefs.h"
#include "AnalogueModel.h"
#include "Mapping.h"


class MIXIM_API CarShadowModel: public AnalogueModel {

protected:
	simtime_t interval;
	double mWavelength;

public:
	CarShadowModel( simtime_t i, double lambda );
	virtual ~CarShadowModel();

	virtual void filterSignal( AirFrame *frame, const Coord& sendersPos, const Coord& receiverPos );
};



class MIXIM_API CarShadowMapping : public SimpleConstMapping {

protected:
	static DimensionSet dimensions;
	double mValue;

public:
	CarShadowMapping( double v, const Argument& start, const Argument& interval, const Argument& end);

	virtual double getValue( const Argument& pos ) const;

	ConstMapping* constClone() const {
		return new CarShadowMapping( *this );
	}



};


#endif /* CORNERMODEL_H_ */
