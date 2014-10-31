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
#include <queue>

#include "UrcScenarioManager.h"
#include "CarShadowModel.h"
#include "BaseWorldUtility.h"
#include "AirFrame_m.h"
#include "ChannelAccess.h"

#include "UrcData.h"
#include "RsuMobility.h"
#include "CarMobility.h"


DimensionSet CarShadowMapping::dimensions(Dimension::time,Dimension::frequency);


using namespace VectorMath;
using namespace Urc;

CarShadowMapping::CarShadowMapping( double v, const Argument& start, const Argument& interval, const Argument& end) :
					   SimpleConstMapping( dimensions, start, end, interval ), mValue(v) {
}


double CarShadowMapping::getValue( const Argument& pos ) const {

	if ( mValue < -1 )
		return 1;

	double PL = 0;

	/*
	 *	These are approximations derived by W.C.Y. Lee
	 *	See Rappaport, Wireless Communications Principles and Practice, Chapter 4, p.131
	 **/

	if ( -1 <= mValue && mValue <= 0 )
		PL = 0.5-0.62*mValue;
	else if ( 0 < mValue && mValue <= 1 )
		PL = 0.5*exp(-0.95*mValue);
	else if ( 1 < mValue && mValue <= 2.4 )
		PL = 0.4 - sqrt( 0.1184 - ( 0.38 - 0.1 * mValue ) * ( 0.38 - 0.1 * mValue ) );
	else if ( mValue > 2.4 )
		PL = 0.225 / mValue;

	return PL;

}


CarShadowModel::CarShadowModel( simtime_t i, double mLambda ) {
	interval = i;
	mWavelength = mLambda;
}

CarShadowModel::~CarShadowModel() {
	// TODO Auto-generated destructor stub
}




void CarShadowModel::filterSignal( AirFrame *frame, const Coord& sendersPos, const Coord& receiverPos ) {

	Signal& signal = frame->getSignal();

	UrcScenarioManager *pManager = UrcScenarioManagerAccess().get();

	LineSegment l;
	l.mStart = Vector2D(  sendersPos.x,  sendersPos.y );
	l.mEnd   = Vector2D( receiverPos.x, receiverPos.y );
	Rect lineRect(l);


	CarMobility *pSenderMob = dynamic_cast<CarMobility*>(dynamic_cast<ChannelAccess *const>(frame->getSenderModule())->getMobilityModule());
	CarMobility *pReceiverMob = dynamic_cast<CarMobility*>(dynamic_cast<ChannelAccess *const>(frame->getArrivalModule())->getMobilityModule());

	std::string txRoadId, rxRoadId;
	UrcData::Classification c;

	double txHeight;
	double rxHeight;
	int xStartTmp, xEndTmp;
	int yStartTmp, yEndTmp;
	if ( !pSenderMob ) {
		RsuMobility *txRsuMob = dynamic_cast<RsuMobility*>(dynamic_cast<ChannelAccess *const>(frame->getSenderModule())->getMobilityModule());
		if ( !txRsuMob )
			opp_error( "Invalid node for transmission." );
		txHeight = txRsuMob->getHeight();
		xStartTmp = txRsuMob->getCurrentPosition().x / pManager->getGridSize();
		yStartTmp = txRsuMob->getCurrentPosition().y / pManager->getGridSize();
		txRoadId = txRsuMob->getRoadId();
	} else {
		txHeight = pSenderMob->getCarDimensions().z;
		xStartTmp = MAX( pSenderMob->getGridCell().x, 0 );
		yStartTmp = MAX( pSenderMob->getGridCell().y, 0 );
		txRoadId = pSenderMob->getRoadId();
	}

	if ( !pReceiverMob ) {
		RsuMobility *rxRsuMob = dynamic_cast<RsuMobility*>(dynamic_cast<ChannelAccess *const>(frame->getArrivalModule())->getMobilityModule());
		if ( !rxRsuMob )
			opp_error( "Invalid node for reception." );
		rxHeight = rxRsuMob->getHeight();
		xEndTmp = rxRsuMob->getCurrentPosition().x / pManager->getGridSize();
		yEndTmp = rxRsuMob->getCurrentPosition().y / pManager->getGridSize();
		rxRoadId = rxRsuMob->getRoadId();
	} else {
		rxHeight = pReceiverMob->getCarDimensions().z;
		xEndTmp = MAX( pReceiverMob->getGridCell().x, 0);
		yEndTmp = MAX( pReceiverMob->getGridCell().y, 0);
		rxRoadId = pReceiverMob->getRoadId();
	}

	Coord posT = pManager->ConvertCoords( sendersPos );
	Coord posR = pManager->ConvertCoords( receiverPos );

	c = UrcData::GetSingleton()->GetClassification( txRoadId, rxRoadId, Vector2D(posT.x,posT.y), Vector2D(posR.x,posR.y) );

	if ( c.mClassification != Classifier::LOS )
		return;	// we don't care about this model if they're not in LOS

	bool bFound = false;

	// find the vehicles on the line between Tx and Rx
	Real gammaT[3], gammaR[3], hts[3], hsr[3], Dts[3], Dsr[3], Dtr;
	gammaT[0] = gammaT[1] = gammaT[2] = 0;
	gammaR[0] = gammaR[1] = gammaR[2] = 0;

	Dtr = l.GetDistance();

	int xStart = MIN( xStartTmp, xEndTmp );
	int yStart = MIN( yStartTmp, yEndTmp );
	int xEnd   = MAX( xStartTmp, xEndTmp );
	int yEnd   = MAX( yStartTmp, yEndTmp );

	for ( int x = xStart; x <= xEnd; x++ ) {

		for ( int y = yStart; y <= yEnd; y++ ) {

			const UrcScenarioManager::GridCell &pCell = pManager->getGridCell( x, y );
			UrcScenarioManager::GridCell::const_iterator it;

			for ( AllInVector( it, pCell ) ) {

				CarMobility *pMob = (*it);

				Coord vPos = pMob->getCurrentPosition();

				if ( pMob == pSenderMob || pMob == pReceiverMob )
					continue;

				double angle = pMob->getAngleRad();

				Vector2D v1, v2, p, heading1, heading2;
				p = Vector2D( vPos.x, vPos.y );
				heading1 = Vector2D( cos(angle), sin(angle) );
				heading2 = Vector2D( heading1.y, -heading1.x );
				LineSegment d1, d2;

				Vector3D currDims = pMob->getCarDimensions();
				d1 = LineSegment( p+heading1*currDims.y/2+heading2*currDims.x/2, p-heading1*currDims.y/2-heading2*currDims.x/2 );
				d2 = LineSegment( p+heading1*currDims.y/2-heading2*currDims.x/2, p-heading1*currDims.y/2+heading2*currDims.x/2 );

				bool d1Int = d1.IntersectLine( l, &v1 ), d2Int = d2.IntersectLine( l, &v2 );

				if ( d1Int || d2Int ) {

					Real h[3], dTx, dRx, tmp;
					h[0] = currDims.z;
					if ( d1Int && !d2Int ) {
						h[1] = (d1.mStart-v1).Magnitude();
						h[2] = (  d1.mEnd-v1).Magnitude();
						dTx = (l.mStart-v1).Magnitude();
						dRx = (  l.mEnd-v1).Magnitude();
					} else if ( !d1Int && d2Int ) {
						h[1] = (d2.mStart-v2).Magnitude();
						h[2] = (  d2.mEnd-v2).Magnitude();
						dTx = (l.mStart-v2).Magnitude();
						dRx = (  l.mEnd-v2).Magnitude();
					} else {
						h[1] = MAX( (d2.mStart-v2).Magnitude(), (d1.mStart-v1).Magnitude() );
						h[2] = MAX( (  d2.mEnd-v2).Magnitude(), (  d1.mEnd-v1).Magnitude() );
						dTx = (l.mStart-v2).Magnitude();
						dRx = (  l.mEnd-v2).Magnitude();
					}

					for ( int i = 0; i < 3; i++ ) {

						tmp = ( h[i] - ( i == 0 ? txHeight : 0 ) ) / dTx;
						if ( gammaT[i] <= tmp ) {

							gammaT[i] = tmp;
							hts[i] = h[i];
							Dts[i] = dTx;

						}

						tmp = ( h[i] - ( i == 0 ? rxHeight : 0 ) ) / dRx;
						if ( gammaR[i] <= tmp ) {

							gammaR[i] = tmp;
							hsr[i] = h[i];
							Dsr[i] = dRx;

						}

					}

					bFound = true;

				} else {

					continue;

				}

			}

		}

	}

	if ( !bFound )
		return;

	Real d1, d2, h, v;

	for ( int i = 0; i < 3; i++ ) {

		if ( gammaT[i] <= 0 && gammaR[i] > 0 ) {

			h = hsr[i];
			d1 = Dts[i];
			d2 = Dsr[i];

		} else if ( gammaT[i] > 0 && gammaR[i] <= 0 ) {

			h = hts[i];
			d1 = Dsr[i];
			d2 = Dts[i];

		} else if ( gammaT[i] == 0 && gammaT[i] == gammaR[i] ) {

			h = 0;

		} else {

			d1 = ( rxHeight - txHeight + gammaR[i] * Dtr ) / ( gammaT[i] + gammaR[i] );
			d2 = Dtr - d1;
			h = gammaT[i] * d1 + txHeight;

		}

		if ( h != 0 ) {
			v = ( atan2( h - ( i == 0 ? txHeight : 0 ), d1 ) + atan2( h - ( i == 0 ? rxHeight : 0 ), d2 ) ) * sqrt( 2 * (d1 + d2) / ( mWavelength * d1 * d2 ) );
		} else {
			v = 0;
		}

		signal.addAttenuation(
				new CarShadowMapping(
						v,
						Argument(signal.getReceptionStart()),
						Argument(interval),
						Argument(signal.getReceptionEnd())
						)
		);

	}

}




