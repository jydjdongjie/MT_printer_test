/*
 * bt_scan.c
 *
 *  Created on: Oct 16, 2014
 *      Author: jack
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "db_handle.h"
#include "bt_scan.h"
#include <unistd.h>

//#define DEBUG_BT_SCAN
#define SCAN_BT_DEV 		"hci0"
#define SEARCH_KEY_STR		"美团外卖 "
#define SEATCH_KEY_STR_G	"Gprinter "
#define SCAN_DELAY_SEC		(5)

static int get_dev_list(const char *hci_dev, const char *key_str, char *recv[]);
static int free_malloc_mem(char *p_tab[], int count);
static char * blank_process(char *str);
static int bt_scan_do(const char *hci_dev, const char *key_str, const char *table);


void *task_bt_scan(void *parg)
{
	printf("Into task_bt_scan()\n");
	while (1)
	{
		//scan the bluetooth devices named "美团外卖 "
		if (bt_scan_do(SCAN_BT_DEV, SEARCH_KEY_STR, MT_DB_TABLE) < 0)
		{
			printf("task_bt_scan %s return error!\n", MT_DB_TABLE);
			break;
		}
		sleep(5);
		if (bt_scan_do(SCAN_BT_DEV, SEATCH_KEY_STR_G, G_DB_TABLE) < 0)
		{
			printf("task_bt_scan %s return error!\n", G_DB_TABLE);
			break;
		}

		sleep(SCAN_DELAY_SEC);
	}

	return NULL;
}



static int bt_scan_do(const char *hci_dev, const char *key_str, const char *table)
{
	//1.Get the result of hcitool scan
	char *str_tab[128];
	int  dev_num = 0;
	int ret = 0;

	printf("into bt_scan_main\n");
	memset((void*)&str_tab[0], 0, 128*sizeof(str_tab[0]));

	if (!(hci_dev && key_str))
	{
		printf("Param error in bt_scan_main\n");
		return -1;
	}

	dev_num = get_dev_list(hci_dev, key_str, str_tab);
	if (dev_num > 0)
	{
		//insert db
		ret = insert_new_to_db(BT_DB_NAME, table, str_tab, dev_num);

		free_malloc_mem(str_tab, dev_num);
	}

	return ret;
}


static char * blank_process(char *str)
{
	char *tmp = str;

	if (str == NULL)
	{
		return NULL;
	}
	while (*tmp != 0)
	{
		if (*tmp == '\t')
		{
			*tmp = ' ';
		}

		tmp++;
	}

	while (*str == ' ')
	{
		str++;
	}

	return str;
}


static int get_dev_list(const char *hci_dev, const char *key_str, char *recv[])
{
	int count = 0;
	FILE *fp_scan = NULL;
	char tmpstr[64];
	char read_buff[1024];

	memset(read_buff, 0, sizeof(read_buff));
	memset(tmpstr, 0, sizeof(tmpstr));

	if (!(hci_dev && key_str && recv))
	{
		return -1;
	}

	sprintf(tmpstr, "hcitool -i %s scan", hci_dev);
	printf("in get_dev_list() %s\n", tmpstr);
	fp_scan = popen(tmpstr, "r");

	if (fp_scan == NULL)
	{
		printf("popen error:fp_scan == NULL in get_dev_list\n");
		return -1;
	}

	while (fgets(read_buff, sizeof(read_buff), fp_scan))
	{
		read_buff[1023] = 0;

		if (strstr(read_buff, key_str) != NULL)
		{
			char *p = blank_process(read_buff);
			int len = strlen(p);

			recv[count] = (char*)malloc(len+1);
			if (recv[count] != NULL)
			{
				strcpy(recv[count],p);
				count++;
				if (count >= 127)
				{
					break;
				}
			}

		}

		memset(read_buff, 0, sizeof(read_buff));
	}

	return count;
}



static int free_malloc_mem(char *p_tab[], int count)
{
	int i ;

	for (i = 0; i < count; i++)
	{
		if (p_tab[i] != NULL)
		{
			free (p_tab[i]);
			p_tab[i] = NULL;
		}
	}

	return 0;
}




