#include <lcommon/unittest.h>
#include "../lanarts_net.h"

int main(int argc, char** argv) {
//	lanarts_net_init(true);
	int exit_code =  run_unittests();
//	lanarts_net_quit();
	return exit_code;
}
