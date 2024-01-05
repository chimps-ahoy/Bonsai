#include <stdio.h>
#include <stdlib.h>

#include "../lib/stack.h"
#include "../lib/tree.h"
#include "../lib/types.h"
#include "../lib/util.h"

int main(int argc, char **argv)
{
	printf("%s-%s\n", *argv, VERSION);
	return EXIT_SUCCESS;
}
