#ifndef CORNER_QUALNET_H
#define CORNER_QUALNET_H

#include "propagation.h"
#include "api.h"
#include "qualnet_mutex.h"

#define DEFAULT_LANE_WIDTH 5
#define DEFAULT_LPR 0.01
#define DEFAULT_GRID_SIZE 100

double PathlossCorner(PropPathProfile *pathProfile, PropProfile *propProfile, Node* node);
void CornerInitialize(Node* node, PropProfile* propProfile, const NodeInput* nodeInput);
//void CornerNodeInitialize(Node* node, PropProfile *propProfile);
void CornerFinalize(PropProfile *propProfile);


#endif
