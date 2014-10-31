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

#ifndef URCPHYLAYER_H_
#define URCPHYLAYER_H_

#include <queue>
#include "PhyLayer80211p.h"
#include "PhyToMacControlInfo.h"


class UrcPhyToMacControlInfo : public PhyToMacControlInfo {

protected:
	double mRiceParamA;			/**< Parameter A of the Rice distribution. */
	double mRiceParamSigma;		/**< Parameter Sigma of the Rice distribution. */

public:
	/**
	 * @brief Initializes the PhyToMacControlInfo with the passed DeciderResult.
	 *
	 * NOTE: PhyToMacControlInfo takes ownership of the passed DeciderResult!
	 */
	UrcPhyToMacControlInfo( double a, double sigma, DeciderResult *result ) : PhyToMacControlInfo(result), mRiceParamA(a), mRiceParamSigma(sigma) {}

	double getA() { return mRiceParamA; }
	double getSigma() { return mRiceParamSigma; }

};


class UrcPhyLayer: public PhyLayer {
public:
	UrcPhyLayer();
	virtual ~UrcPhyLayer();

	/**
	 * @brief Called to send an AirFrame with DeciderResult to the MACLayer
	 *
	 * Send the AirFrame up, with a different control info.
	 *
	 */
	void sendUp(AirFrame* packet, DeciderResult* result);

protected:
	virtual AnalogueModel* getAnalogueModelFromName( std::string name, ParameterMap& params );

};

#endif /* URCPHYLAYER_H_ */
