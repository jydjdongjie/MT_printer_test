/*
 * main.c
 *
 *  Created on: Oct 16, 2014
 *      Author: jack
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#include "bt_scan.h"
#include "bt_print.h"
#include "db_handle.h"

#define SEC_REPRINT_DEF 120

long int print_interval_sec = SEC_REPRINT_DEF;

int main (int argc, const char *argv[])
{
	pthread_t tid_scan, tid_print;
	int ret = 0;

	if (argv[1] != NULL)
	{
		print_interval_sec = atoi(argv[1]);
	}

	if (print_interval_sec < 10)
	{
		print_interval_sec = SEC_REPRINT_DEF;
	}

	printf("printer_test V0.2 - 2014.10.23pm\n"
			"print_interval_sec = %ld\n", print_interval_sec);

	ret = pthread_create(&tid_scan, NULL, task_bt_scan, NULL);
	if (ret != 0)
	{
		printf("Create the scan task failed!\n");
		exit(1);
	}

	printf("scan task create sucess!\n");

	ret = pthread_create(&tid_print, NULL, task_bt_print, NULL);
	if (ret != 0)
	{
		printf("Create the print task failed!\n");
		exit(1);
	}

	printf("scan task create sucess!\n");

	while (1)
	{
		print_db_all (BT_DB_NAME, MT_DB_TABLE);
		printf("now UTC:%ld\n", time(NULL));
		sleep(1);
		print_db_all (BT_DB_NAME, G_DB_TABLE);
		sleep(1);
	}

	return 0;
}


