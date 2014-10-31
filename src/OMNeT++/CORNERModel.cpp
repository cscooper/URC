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

#include <TraCIMobility.h>

#include "UrcScenarioManager.h"
#include "RsuMobility.h"
#include "CarMobility.h"
#include "CORNERModel.h"

#include "BaseWorldUtility.h"
#include "AirFrame_m.h"
#include "ChannelAccess.h"

#include <queue>
#include <fstream>



DimensionSet CORNERModel::dimensions(Dimension::time,Dimension::frequency);
DimensionSet CORNERMapping::dimensions(Dimension::time,Dimension::frequency);


using namespace VectorMath;
using namespace Urc;

CORNERMapping::CORNERMapping( Coord tPos, Coord rPos, Urc::UrcData::Classification c, double k, const Argument& start, const Argument& interval, const Argument& end) :
					   SimpleConstMapping( dimensions, start, end, interval ), txPos( tPos ), rxPos( rPos ), mClassification( c ), kFactor(k) {
}


double CORNERMapping::getValue( const Argument& pos ) const {
    Classifier classifier(mClassification);
    return classifier.CalculatePathloss( Vector2D( txPos.x, txPos.y ), Vector2D( rxPos.x, rxPos.y ) ) / UrcData::GetSingleton()->GetSystemLoss();
}


CORNERModel::CORNERModel( simtime_t i, double k ) {
	interval = i;
	staticK = k;
}

CORNERModel::~CORNERModel() {
	// TODO Auto-generated destructor stub
}


void CORNERModel::filterSignal( AirFrame *frame, const Coord& sendersPos, const Coord& receiverPos ) {

	Signal& signal = frame->getSignal();
	double kFactor = 0;

	UrcScenarioManager *pManager = UrcScenarioManagerAccess().get();

	std::string txRoadId, rxRoadId;
	int txLaneId, rxLaneId;
	CarMobility *pMobTx = dynamic_cast<CarMobility*>(dynamic_cast<ChannelAccess *const>( frame->getSenderModule())->getMobilityModule());
	CarMobility *pMobRx = dynamic_cast<CarMobility*>(dynamic_cast<ChannelAccess *const>(frame->getArrivalModule())->getMobilityModule());
	RsuMobility *pRsuTx = dynamic_cast<RsuMobility*>(dynamic_cast<ChannelAccess *const>( frame->getSenderModule())->getMobilityModule());
	RsuMobility *pRsuRx = dynamic_cast<RsuMobility*>(dynamic_cast<ChannelAccess *const>(frame->getArrivalModule())->getMobilityModule());
	UrcData::Classification c;

	if ( pMobTx ) {
		txRoadId = pMobTx->getRoadId();
		txLaneId = pMobTx->getLaneId();
	} else if ( pRsuTx ) {
		txRoadId = pRsuTx->getRoadId();
		txLaneId = pRsuTx->getLaneId();
	}
	Coord posT = pManager->ConvertCoords( sendersPos );
	Vector2D posTv = Vector2D(posT.x,posT.y);

	if ( pMobRx ) {
		rxRoadId = pMobRx->getRoadId();
		rxLaneId = pMobRx->getLaneId();
	} else if ( pRsuRx ) {
		rxRoadId = pRsuRx->getRoadId();
		rxLaneId = pRsuRx->getLaneId();
	}
	Coord posR = pManager->ConvertCoords( receiverPos );
	Vector2D posRv = Vector2D(posR.x,posR.y);

	c = UrcData::GetSingleton()->GetClassification( txRoadId, rxRoadId, posTv, posRv );
	UrcData::GetSingleton()->RefineClassification( c, posTv, posRv );
	if ( staticK == -1 ) {
		// There has been no static K factor specified. Get one from our index.
		if ( c.mClassification == Classifier::LOS )
			kFactor = UrcData::GetSingleton()->GetK( c.mLinkPair, posTv, txLaneId, posRv, rxLaneId, c.mFlipped );
	} else {
		kFactor = staticK;
	}

	signal.addAttenuation(
			new CORNERMapping(
					posT,
					posR,
					c,
					kFactor,
					Argument(signal.getReceptionStart()),
					Argument(interval),
					Argument(signal.getReceptionEnd())
					)
	);


	if ( kFactor != DBL_MAX ) {

		// we have fading at this point
		Mapping *att = MappingUtils::createMapping( dimensions, Mapping::LINEAR );
		Argument pos;

		for ( simtime_t t = signal.getReceptionStart(); t <= signal.getReceptionEnd(); t += interval ) {

			pos.setTime( t );
			double f = Fading::GetSingleton()->CalculateFading( c.mClassification, kFactor );
			att->appendValue( pos, f );

		}
		signal.addAttenuation( att );

	}

}




