#include "../lib/czmut/src/czmut/czmut.h"

int main()
{
	return cz::mut::runAll(F("[example]")) ? 0 : 1;
}