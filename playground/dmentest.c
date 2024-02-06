#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
	FILE *pipo = popen("dmenu < /dev/null | lua 2>&1", "r");
	if (!pipo) {
		fprintf(stderr, "farted\n");
		return EXIT_FAILURE;
	}
	char buff[255] = {0};
	fgets(buff, 255, pipo);
	fprintf(stdout, "lua says:\n\t %s", buff);
	pclose(pipo);
	return EXIT_SUCCESS;
}
