#include <stdio.h>

#include "FramedWindow.h"
#include "WindowManager.h"

int main(void)
{
	fprintf(stdout, "Hello from zwm!\n");

	if (ZWM::WindowManager::the()->init() != 0)
		return -1;
	ZWM::WindowManager::the()->run_loop();
}
