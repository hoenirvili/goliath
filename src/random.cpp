#include "random.h"
#include <random>
#include <chrono>

using namespace std;

string random_string()
{
	auto time_point = chrono::high_resolution_clock::now();
	auto since = time_point.time_since_epoch();
	// remove the other half, I don't care
	unsigned int seed = (unsigned int)(since.count() & 0xFFFFFFFF);
	mt19937 mt_rand(seed);
	auto random = mt_rand();
	return to_string(random);
}
