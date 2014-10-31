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

#include <cfloat>
#include <list>
#include <map>
#include <TraCIMobility.h>

#include <queue>

#include <time.h>

#include "Urc.h"
#include "UrcPhyLayer.h"
#include "CORNERModel.h"
#include "CarShadowModel.h"

using namespace std;

Define_Module( UrcPhyLayer );

UrcPhyLayer::UrcPhyLayer() {
	// TODO Auto-generated constructor stub

}

UrcPhyLayer::~UrcPhyLayer() {
	// TODO Auto-generated destructor stub
}


/*
void computeKandOmega( Signal& signal, double *k, double *omega ) {

	double gm = 0;
	*k = *omega = 0;

	Argument timeLoc(DimensionSet::timeFreqDomain);
	timeLoc.setArgValue(Dimension::frequency,3e8 / Urc::UrcData::GetSingleton()->GetWavelength());
	simtime_t currTime = signal.getReceptionStart();
	simtime_t timeIncr = (signal.getReceptionEnd() - currTime) / 30;
	for ( ; currTime < signal.getReceptionEnd(); currTime += timeIncr ) {
		timeLoc.setTime( currTime );
		double rx = signal.getReceivingPower()->getValue( timeLoc );
		gm += rx;
		(*omega) += rx * rx;
	}
	gm /= 30;
	*omega /= 30;

	double d = 2*gm*gm - *omega;
	if ( d < 0 )
		return;

	double v2 = sqrt(d);
	double s2 = gm - v2;

	*k = v2/s2;

}
*/


/**
 * @brief Called to send an AirFrame with DeciderResult to the MACLayer
 *
 * Send the AirFrame up, with a different control info.
 *
 */
void UrcPhyLayer::sendUp(AirFrame* packet, DeciderResult* result) {

	coreEV << "Decapsulating MacPacket from Airframe with ID " << packet->getId() << " and sending it up to MAC." << endl;

	cMessage* pkt = packet->decapsulate();
	assert(pkt);

	Signal& signal = packet->getSignal();

	double k, omega;

	// Get K
	const Signal::ConstMappingList &attenuation = signal.getAttenuation();
	Signal::ConstMappingList::const_iterator it;
	for ( it = attenuation.begin(); it != attenuation.end(); it++ ) {
		CORNERMapping *c = dynamic_cast<CORNERMapping*>(*it);
		if ( c ) {
			k = c->GetK();
			break;
		}
	}

	// Get Omega
	Argument timeLoc(DimensionSet::timeFreqDomain);
	timeLoc.setArgValue(Dimension::frequency,3e8 / Urc::UrcData::GetSingleton()->GetWavelength());
	timeLoc.setTime( signal.getReceptionStart() );
	omega = signal.getReceivingPower()->getValue( timeLoc );

	// Now compute the K-factor and Omega.
	//computeKandOmega( signal, &k, &omega );

//	std::cerr << "K = " << k << "; O = " << omega << "\n";
	// Compute the statistics 
	double a = sqrt( k*omega / ( 1+k ) );
	double sigma = sqrt( omega / ( 2 * ( 1+k ) ) );

	// Add the control info.
	pkt->setControlInfo( new UrcPhyToMacControlInfo( a, sigma, result ) );

	// Send the packet up.
	sendMacPktUp( pkt );

}



AnalogueModel* UrcPhyLayer::getAnalogueModelFromName( std::string name, ParameterMap& params ) {

	if ( name == "CORNER" ) {

		simtime_t interval = params["interval"].doubleValue();
		double k = -1;
		if ( params.find("k") != params.end() )
			k = params["k"].doubleValue();

		return new CORNERModel( interval, k );

	} else if ( name == "CarShadow" ) {

		simtime_t interval = params["interval"].doubleValue();
		double wavelength = params["wavelength"].doubleValue();

		return new CarShadowModel( interval, wavelength );

	}


	return PhyLayer::getAnalogueModelFromName( name, params );

}











