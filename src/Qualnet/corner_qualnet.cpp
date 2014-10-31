#include "corner_qualnet.h"
#include "Singleton.h"
#include "VectorMath.h"
#include "BuildingSolver.h"
#include "Fading.h"
#include "Classifier.h"
#include "partition.h"
#include "phy.h"
#include "phy_802_11.h"
#include <iostream>
#include <fstream>
QNPartitionMutex sMutex;

using namespace std;
using namespace VectorMath;


double PathlossCorner(PropPathProfile *pathProfile, PropProfile *propProfile, Node* node)
{
	Vector2D tx, rx;
	tx.x = pathProfile->fromPosition.cartesian.x;
	tx.y = pathProfile->fromPosition.cartesian.y;
	rx.x = pathProfile->toPosition.cartesian.x;
	rx.y = pathProfile->toPosition.cartesian.y;
        
	//calculate the path-loss
	double PL = propProfile->cornerClassifier->CalculatePathloss(tx, rx);
	
	if (PL == 0) { //just a stupidly large path loss to let the nodes know that they cannot see each other
		return(10000);
	} else {
		if (propProfile->cornerFading) {
			PL = -IN_DB(PL) + Corner::Fading::GetSingleton()->CalculateFading(propProfile->cornerClassifier->GetClassification(), propProfile->cornerK);
			return (PL);
		}
		return(-IN_DB(PL));
	}
}

void CornerInitialize( Node* node, PropProfile* propProfile, const NodeInput* nodeInput )
{
	BOOL retVal;
        //Initialise BuildingSolver and Fading classes once using Mutex locks
        QNPartitionLock *initLock = new QNPartitionLock(&sMutex);
        if (!Corner::BuildingSolver::GetSingleton()) { //uninitialised?
                try { 
                        char linksFile[MAX_STRING_LENGTH], nodesFile[MAX_STRING_LENGTH], classFile[MAX_STRING_LENGTH];
                        double txPower, rxSensitivity, laneWidth, lpr, gain, loss, grid;
                        
                        //read in the corner links file
                        IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput, "PROPAGATION-CORNER-LINKS-FILE", &retVal, linksFile);
                        ERROR_Assert(retVal, "CORNER requires the links file to be specified with the PROPAGATION-CORNER-LINKS-FILE parameter in the Qualnet configuration file\n");
                        
			//read in the corner nodes file
                        IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput, "PROPAGATION-CORNER-NODES-FILE", &retVal, nodesFile);
                        ERROR_Assert(retVal, "CORNER requires the nodes file to be specified with the PROPAGATION-CORNER-NODES-FILE parameter in the Qualnet configuration file\n");
			
			//read in the corner classifications file
			IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput, "PROPAGATION-CORNER-CLASSIFICATIONS-FILE", &retVal, classFile);
			ERROR_Assert(retVal, "CORNER requires the classifications file to be specified with the PROPAGATION-CORNER-CLASSIFICATIONS-FILE parameter in the Qualnet configuration file\n");
			
			//read in the grid size to use for location determination
			IO_ReadDouble(ANY_NODEID, ANY_ADDRESS, nodeInput, "PROPAGATION-CORNER-LOCATION-GRID-SIZE", &retVal, &grid);
			if (!retVal) {
				grid = DEFAULT_GRID_SIZE;
			}
		
                        //read in the lane width
                        IO_ReadDouble(ANY_NODEID, ANY_ADDRESS, nodeInput, "PROPAGATION-CORNER-LANE-WIDTH", &retVal, &laneWidth);
                        if (!retVal) {
                                laneWidth = DEFAULT_LANE_WIDTH;
                        }
                        
                        //read in the loss per reflection
                        IO_ReadDouble(ANY_NODEID, ANY_ADDRESS, nodeInput, "PROPAGATION-CORNER-LOSS-PER-REFLECTION", &retVal, &lpr);
                        if (!retVal) {
                                lpr = DEFAULT_LPR;
                        }
                        
                        //get the txPower, rxSensitivity, gains and losses
                        ERROR_Assert((node->phyData[0]->antennaData->antennaModelType == ANTENNA_OMNIDIRECTIONAL), "BuildingSolver is designed only for omnidirectional antennas\n");
                        gain = NON_DB(((AntennaOmnidirectional*) node->phyData[0]->antennaData->antennaVar)->antennaGain_dB);
                        
			//loss is multiplied by 2 to represent losses on both transmitter and receiver
                        loss = (NON_DB(node->phyData[0]->systemLoss_dB))*2;
                        
                        txPower = NON_DB(((PhyData802_11*) node->phyData[0]->phyVar)->txDefaultPower_dBm[((PhyData802_11*)node->phyData[0]->phyVar)->lowestDataRateType]);
                        rxSensitivity = ((PhyData802_11*) node->phyData[0]->phyVar)->rxSensitivity_mW[((PhyData802_11*)node->phyData[0]->phyVar)->lowestDataRateType];
			
			cerr<<txPower<<" "<<rxSensitivity<<" "<<loss;
			
                        //Create building solver. Gain is not used for now. Loss is not representative of losses due to the atmospheric noise/interference but rather an idealised Loss due to cabling and other system losses.
                        new Corner::BuildingSolver(linksFile, nodesFile, classFile, laneWidth, propProfile->wavelength, txPower, loss, rxSensitivity, lpr, grid);
                } catch (Exception &e) {
                        ERROR_Assert(0, e.What().c_str());
                }
        }
        
	//check if we use fading
        IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput, "PROPAGATION-CORNER-USE-FADING", &retVal, &propProfile->cornerFading);
        if (!retVal) {
                propProfile->cornerFading = FALSE;
        } else {
		if (!Corner::Fading::GetSingleton()) {
			try {
				char compFile[MAX_STRING_LENGTH];
				
				//read in the rician K-factor
				IO_ReadDouble(ANY_NODEID, ANY_ADDRESS, nodeInput, "PROPAGATION-CORNER-RICEAN-K-FACTOR", &retVal, &(propProfile->cornerK));
				ERROR_Assert(retVal, "PROPAGATION-CORNER-RICEAN-K-FACTOR must be specified if using PROPAGATION-CORNER-USE-FADING\n");

				//read in the gaussian components file
				IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput, "PROPAGATION-FADING-GAUSSIAN-COMPONENTS-FILE", &retVal, compFile);
				ERROR_Assert(retVal, "PROPAGATION-FADING-GAUSSIAN-COMPONENTS-FILE must be specified if using PROPAGATION-CORNER-USE-FADING\n");
				
				//initialise the Fading singleton
				new Corner::Fading(compFile, node->globalSeed);
			} catch (Exception &e) {
				ERROR_Assert(0, e.What().c_str());
			}
		}
	}
        
        delete initLock;
        
        propProfile->cornerClassifier = new Corner::Classifier();
}

void CornerFinalize(PropProfile *propProfile)
{
        QNPartitionLock *finalLock = new QNPartitionLock(&sMutex);
	if (Corner::BuildingSolver::GetSingleton()) {
		delete Corner::BuildingSolver::GetSingleton();
	}
	if (Corner::Fading::GetSingleton()) {
		delete Corner::Fading::GetSingleton();
	}
	delete finalLock;
        if (propProfile->cornerClassifier) {
                delete propProfile->cornerClassifier;
        }
        
}