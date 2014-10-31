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

#ifndef URCMACLAYER_H_
#define URCMACLAYER_H_

#include <queue>
#include "Mac80211.h"
#include "MacToNetwControlInfo.h"


class UrcMacToNetwControlInfo : public MacToNetwControlInfo {

protected:
	double mRiceParamA;			/**< Parameter A of the Rice distribution. */
	double mRiceParamSigma;		/**< Parameter Sigma of the Rice distribution. */

public:
	/**
	 * @brief Initializes with the passed last hop address and bit error rate.
	 */
	UrcMacToNetwControlInfo(const LAddress::L2Type& lastHop, double a, double sigma) : MacToNetwControlInfo(lastHop), mRiceParamA(a), mRiceParamSigma(sigma) {}

	double getA() { return mRiceParamA; }

	double getSigma() { return mRiceParamSigma; }

};


class UrcMacLayer: public Mac80211 {
public:
	UrcMacLayer();
	virtual ~UrcMacLayer();

protected:
    /** @brief decapsulate packet */
    cMessage* decapsMsg(Mac80211Pkt *frame);

};

#endif /* URCMACLAYER_H_ */
