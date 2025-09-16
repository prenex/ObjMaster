#include <stdio.h>
#include <unistd.h>
// emcc hello_world.c -o hello.html

int main() {
	for(int i = 0; i < 5; ++i) {
		puts("Hello World");
		sleep(1);
		fflush(stdout);
	}
}
