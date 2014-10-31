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

#include "UrcMacLayer.h"
#include "UrcPhyLayer.h"

using namespace std;

Define_Module( UrcMacLayer );

UrcMacLayer::UrcMacLayer() {
	// TODO Auto-generated constructor stub

}

UrcMacLayer::~UrcMacLayer() {
	// TODO Auto-generated destructor stub
}


/** @brief decapsulate packet */
cMessage* UrcMacLayer::decapsMsg(Mac80211Pkt *frame) {

	cMessage *m = frame->decapsulate();
	UrcPhyToMacControlInfo *ctrlInfo = dynamic_cast<UrcPhyToMacControlInfo*>(frame->getControlInfo());
	m->setControlInfo( new UrcMacToNetwControlInfo( frame->getSrcAddr(), ctrlInfo->getA(), ctrlInfo->getSigma() ) );
	debugEV << " message decapsulated " << endl;
	return m;

}







