#include "../lib/czmut/src/czmut/czmut.h"

int main()
{
	return cz::mut::run(F("[example]")) ? 0 : 1;
}