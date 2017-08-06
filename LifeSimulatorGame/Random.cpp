#include "Random.h"
#include <stdlib.h>

float FRandom::Range(float range)
{
	return range * (rand() / float(RAND_MAX));
}
