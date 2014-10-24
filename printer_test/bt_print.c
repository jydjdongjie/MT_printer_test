/*
 * bt_print.c
 *
 *  Created on: Oct 16, 2014
 *      Author: jack
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "bt_print.h"
#include "esc_deal.h"
#include "db_handle.h"

#define PRINT_DELAY_SEC (1)
#define PRINT_BT_DEV	"hci1"
#define PRINT_MT_CN	"�������� "
#define PRINT_G_EN "Gprinter"

static int bt_print_do(const char *hci_dev, const char *key_str, const char *db_name, const char *db_table);
static void mac_process(char *des_mac, const char *src_mac);

void *task_bt_print(void *parg)
{
	sleep(2);
	while (1)
	{
		if (bt_print_do(PRINT_BT_DEV, PRINT_MT_CN, BT_DB_NAME, MT_DB_TABLE) < 0)
		{
			printf("bt_print_task exit with error\n");
			break;//error
		}
		//sleep(PRINT_DELAY_SEC);

		if (bt_print_do(PRINT_BT_DEV, PRINT_G_EN, BT_DB_NAME, G_DB_TABLE) < 0)
		{
			printf("bt_print_task exit with error\n");
			break;//error
		}
		//sleep(PRINT_DELAY_SEC);
	}

	return NULL;
}


static void mac_process(char *des_mac, const char *src_mac)
{
	while (*src_mac != 0)
	{
		if (*src_mac != ':')
		{
			*des_mac = *src_mac;
			des_mac++;
		}
		src_mac++;
	}

	*des_mac = 0;
}


static int bt_print_do(const char *hci_dev, const char *key_str, const char *db_name, const char *db_table)
{
	int ret = 0;
	bt_dev_element_t element;
	int num ;
	char tmp_buff[512];
	char pure_mac[128];

	num = get_new_from_db(0, &element, db_name, db_table);

	switch (num)
	{
	case -1://error
		ret = -1;
		break;
	case 0://
		ret = 0;
		break;
	case 1://print
	{
		FILE *fp_connect = NULL;
		char read_buff[256];

		memset(read_buff, 0, sizeof(read_buff));
		sprintf(tmp_buff, "sdptool -i %s add --channel=1 SP", PRINT_BT_DEV);
		fp_connect = popen(tmp_buff, "r");
		if (fp_connect == NULL)
		{
			printf("do sdptool error fp_connect == NULL\n");
			return 0;
		}

		if ((fgets(read_buff, sizeof(read_buff), fp_connect) == NULL) || (NULL == strstr(read_buff, "Serial Port service registered")))
		{
			printf("sdptool return error\n");
			return 0;
		}

		system("rfcomm -i hci1 release /dev/rfcomm0");
		sprintf(tmp_buff, "rfcomm -i %s bind /dev/rfcomm0 %s 1", PRINT_BT_DEV, element.bt_mac);
		fp_connect = popen(tmp_buff, "r");
		if (fp_connect == NULL)
		{
			printf("do rfcomm error\n");
			return -1;
		}

		if (fgets(read_buff, sizeof(read_buff), fp_connect))
		{
			printf("error:%s", read_buff);
			update_flag_to_db(-1, &element, db_name, db_table);
			return 0;
		}

		if (open_port("/dev/rfcomm0") < 0)
		{
			system("rfcomm -i hci1 release /dev/rfcomm0");
			update_flag_to_db(-1, &element, db_name, db_table);
			return 0;
		}

		printer_init();			//initialize the printer

		if (strcmp(key_str, PRINT_G_EN) == 0)
		{//Gprinter
			line_feed();
			print_text_bold("����ô ������ӡ����\n");
			line_feed();
			print_text("1.���ֻ��������ܴ򿪣��򿪶���ô�̼Ұ棬������ϽǴ򿪲˵���ѡ��ϵͳ���á�\n");
			print_text("2.ѡ����Ӵ�ӡ����\n");
			sprintf(tmp_buff, "3.ѡ��%s %s��������������� 0000�����а�\n", key_str, element.bt_short_mac);
			print_text(tmp_buff);
			print_text("4.�󶨳ɹ��󣬼��ɲ��Դ�ӡ\n");
			line_feed();
			line_feed();
			line_feed();
		}
		else
		{//MT
			print_text_bold("\n��ӭʹ����������������ӡ��\n");
			line_feed();
			print_text("http://e.waimai.meituan.com");
			line_feed();
			line_feed();
			line_feed();
			print_text_bold("iOS �汾����\n");
			line_feed();
			print_text("1.���ֻ��������ܴ򿪣������������̼Ұ棬������½ǡ����ࡿ��ѡ������������ӡ����\n");
			sprintf(tmp_buff, "2.��������ӡ�����ӡ�%s %s����ӡ��\n", key_str, element.bt_short_mac);
			print_text(tmp_buff);
			print_text("3.�ȴ�������ɺ󣬲��Դ�ӡ�ɹ���������󶨳ɹ�\n");
			print_text("4.�ڵ�ǰҳ�������ѡ��Ŀ������Զ���ӡ���͡���ӡ�������á�\n");
			line_feed();
			line_feed();
			line_feed();
			print_text_bold("Android �汾����\n");
			line_feed();
			print_text("1.���ֻ��������ܴ򿪣������������̼Ұ棬������ϽǴ򿪲˵�����ѡ������/��ӡ����\n");
			print_text("2.�ҵ�������ӡ����ѡ�񡾴�ӡ����\n");
			print_text("3.���ͼ�꡾������ӡ����\n");
			sprintf(tmp_buff, "4.ѡ��%s %s��\n", key_str, element.bt_short_mac);
			print_text(tmp_buff);
			print_text("5.����������� 0000 ���а�\n");
			print_text("5.�����Ϸ���ʾ�Ѿ��󶨵��豸��������󶨳ɹ�\n");
			print_text("7.��������ҳ����Դ�ӡ����ѡ�������Զ���ӡ���͡���ӡ�������á�\n");
			line_feed();
			line_feed();
			line_feed();
		}

		mac_process(pure_mac, element.bt_mac);
		print_barcode(pure_mac);
		line_feed();
		sprintf(tmp_buff, "Mac: %s\n", pure_mac);
		print_text(tmp_buff);
		line_feed();
		line_feed();
		line_feed();
		line_feed();
		close_port();

		system("rfcomm release /dev/rfcomm0");
		update_flag_to_db(1, &element, db_name, db_table);
		ret = 0;
		usleep(500000);
	}break;
	default://never
		ret = -1;
		break;
	}

	sleep(1);
	return ret;
}






