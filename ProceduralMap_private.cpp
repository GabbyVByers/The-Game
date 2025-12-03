
#include "ProceduralMap.h"

ProceduralMap::State& ProceduralMap::GetState() {
	static State state;
	return state;
}

