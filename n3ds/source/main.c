#include <stdio.h>
#include <string.h>
#include <3ds.h>

int main(int argc, char* argv[])
{
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);
	printf("Hello, world!\n");

	while (aptMainLoop()) {
		gspWaitForVBlank();
		gfxSwapBuffers();
	}

	return 0;
}
