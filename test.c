#include <stdio.h>
#include <stdlib.h> //exit
#include <task.h>
#include <timer.h>
#include <native/task.h>

#define TASK_PRIO 99 // 99 is Highest RT priority, 0 is Lowest
#define TASK_MODE 0 // No flags
#define TASK_STKSZ 0 // default Stack size
#define TASK_PERIOD 1000000 // 0.5= 50000000 ns
RT_TASK tA;


int main (int argc, char *argv[]) {
	int e1, e2, e3;
	
	return 0;
}
