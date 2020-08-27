#include <stdio.h>

#include "FramedWindow.h"
#include "WindowManager.h"

int main(int argc, const char** argv)
{
	fprintf(stdout, "Hello from zwm!\n");

	if (ZWM::WindowManager::the()->init() != 0)
		return -1;
	ZWM::WindowManager::the()->run_loop();
	return 0;
}
