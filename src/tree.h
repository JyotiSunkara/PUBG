#pragma once

#include "object.h"

// this really doesn't have to exist, but I'm trying to maintain some consistency
// between the DroneManager and the TreeManager; this class is also used so we
// can add ray-cast-collidable objects to the game world (and we've kept the Object
// class pure virtual (abstract) for design reasons)
class Tree : public Object { };
