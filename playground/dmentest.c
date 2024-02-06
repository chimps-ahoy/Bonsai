#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
	FILE *pipo = popen("dmenu < /dev/null", "r");
	if (!pipo) {
		fprintf(stderr, "farted\n");
		return EXIT_FAILURE;
	}
	char buff[255] = {0};
	fgets(buff, 255, pipo);
	long farted = atoi(buff);
	fprintf(stdout, "2 + 2 = %ld\n", farted *2);
	pclose(pipo);
	return EXIT_SUCCESS;
}
