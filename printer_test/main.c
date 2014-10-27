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


#define __SINGLE_MAIN

long int print_interval_sec = SEC_REPRINT_DEF;

char valid_dev_name[32];

void *task_single_main(void *parg)
{
	while (1)
	{
		//scan
		if (bt_scan_do(valid_dev_name, SEARCH_KEY_STR, MT_DB_TABLE) < 0)
		{
			printf("task_bt_scan %s return error!\n", MT_DB_TABLE);
			break;
		}

		sleep(1);

		if (bt_scan_do(valid_dev_name, SEATCH_KEY_STR_G, G_DB_TABLE) < 0)
		{
			printf("task_bt_scan %s return error!\n", G_DB_TABLE);
			break;
		}

		sleep(1);

		//print
		if (bt_print_do(valid_dev_name, PRINT_MT_CN, BT_DB_NAME, MT_DB_TABLE) < 0)
		{
			printf("bt_print_task exit with error\n");
			break;//error
		}
		sleep(1);

		if (bt_print_do(valid_dev_name, PRINT_G_EN, BT_DB_NAME, G_DB_TABLE) < 0)
		{
			printf("bt_print_task exit with error\n");
			break;//error
		}
		sleep(1);
	}

	return NULL;
}



int main (int argc, const char *argv[])
{
//#ifdef __SINGLE_MAIN
//	pthread_t tid_single;
//#else
//	pthread_t tid_scan, tid_print;
//#endif
//
//	int ret = 0;
//
//	if (argv[1] != NULL)
//	{
//		print_interval_sec = atoi(argv[1]);
//	}
//
//	if (print_interval_sec < 10)
//	{
//		print_interval_sec = SEC_REPRINT_DEF;
//	}
//
//	printf("printer_test V0.2 - 2014.10.24pm\n"
//			"print_interval_sec = %ld\n", print_interval_sec);
//
//#ifdef __SINGLE_MAIN
//	printf("Running single_main\n");
//	ret = pthread_create(&tid_single, NULL, task_single_main, NULL);
//	if (ret != 0)
//	{
//		printf("Create the single task failed! tid_scan = %d\n", (int)tid_single);
//		exit(1);
//	}
//#else
//
//	ret = pthread_create(&tid_scan, NULL, task_bt_scan, NULL);
//	if (ret != 0)
//	{
//		printf("Create the scan task failed! tid_scan = %d\n", (int)tid_scan);
//		exit(1);
//	}
//
//	printf("scan task create sucess!\n");
//
//	ret = pthread_create(&tid_print, NULL, task_bt_print, NULL);
//	if (ret != 0)
//	{
//		printf("Create the print task failed! tid_print = %d\n", (int)tid_print);
//		exit(1);
//	}
//
//	printf("scan task create sucess!\n");
//#endif
//
//	while (1)
//	{
//		print_db_all (BT_DB_NAME, MT_DB_TABLE);
//		printf("now UTC:%ld\n", time(NULL));
//		sleep(1);
//		print_db_all (BT_DB_NAME, G_DB_TABLE);
//		sleep(1);
//	}
	memset(&valid_dev_name[0], 0, sizeof(valid_dev_name));

	get_local_bt_dev(&valid_dev_name[0], 0);

	if (strlen(&valid_dev_name[0]) == 4)
	{
		task_single_main(NULL);
	}


	return 0;
}


