#include <stdio.h>

#include "FramedWindow.h"
#include "WindowManager.h"

// int x_error_handler(Display *d, XErrorEvent * e)
// {
// 	fprintf(stderr, "X ERROR: %x - %x\n", e->error_code, e->minor_code);
// 	fprintf(stderr, "\ttype: 0x%x\n", e->type);
// 	fprintf(stderr, "\tserial: 0x%lx\n", e->serial);
// 	fprintf(stderr, "\tresourceid: 0x%lx\n", e->resourceid);
// 	fprintf(stderr, "\trequest_code: 0x%x\n", e->request_code);
// 	return 0;
// }

int main(int argc, const char **argv)
{
	fprintf(stdout, "Hello from zwm!\n");

	// XSetErrorHandler(x_error_handler);

	if (ZWM::WindowManager::the()->init() != 0)
		return -1;
	ZWM::WindowManager::the()->run_loop();
	return 0;
}
