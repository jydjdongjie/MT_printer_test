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

//#define PRINT_DELAY_SEC (1)
//#define PRINT_BT_DEV	"hci1"
//#define PRINT_MT_CN	"美团外卖 "
//#define PRINT_G_EN "Gprinter"

//static int bt_print_do(const char *hci_dev, const char *key_str, const char *db_name, const char *db_table);
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
		sleep(PRINT_DELAY_SEC);

		if (bt_print_do(PRINT_BT_DEV, PRINT_G_EN, BT_DB_NAME, G_DB_TABLE) < 0)
		{
			printf("bt_print_task exit with error\n");
			break;//error
		}
		sleep(PRINT_DELAY_SEC);
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


//static int bt_print_do(const char *hci_dev, const char *key_str, const char *db_name, const char *db_table)
int bt_print_do(const char *hci_dev, const char *key_str, const char *db_name, const char *db_table)
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

		//system("rfcomm -i hci1 release /dev/rfcomm0");//try to release the rfcomm0
		//system("rfcomm -i hci0 release /dev/rfcomm0");//try to release the rfcomm0

		//release the dev rfcomm0
		sprintf(tmp_buff, "rfcomm -i %s release /dev/rfcomm0", hci_dev);
		fp_connect = popen(tmp_buff, "r");
		if (fp_connect == NULL)
		{
			printf("do %s ]error\n", tmp_buff);
			return 0;
		}
		pclose(fp_connect);

		//sdptool
		sprintf(tmp_buff, "sdptool -i %s add --channel=1 SP", hci_dev);
		fp_connect = popen(tmp_buff, "r");
		if (fp_connect == NULL)
		{
			printf("do sdptool error fp_connect == NULL\n");
			return 0;
		}

		if ((fgets(read_buff, sizeof(read_buff), fp_connect) == NULL) || (NULL == strstr(read_buff, "Serial Port service registered")))
		{
			printf("sdptool return error\n");
			pclose(fp_connect);
			return 0;
		}
		pclose(fp_connect);

		//bind
		sprintf(tmp_buff, "rfcomm -i %s bind /dev/rfcomm0 %s 1", hci_dev, element.bt_mac);
		fp_connect = popen(tmp_buff, "r");
		if (fp_connect == NULL)
		{
			printf("do rfcomm error\n");
			return 0;//return -1;
		}

		if (fgets(read_buff, sizeof(read_buff), fp_connect))
		{
			printf("error:%s", read_buff);
			pclose(fp_connect);
			update_flag_to_db(-1, &element, db_name, db_table);
			return 0;
		}
		pclose(fp_connect);


		if (open_port("/dev/rfcomm0") < 0)
		{
			sprintf(tmp_buff, "rfcomm -i %s release /dev/rfcomm0", hci_dev);
			system(tmp_buff);
//			system("rfcomm -i hci1 release /dev/rfcomm0");
			update_flag_to_db(-1, &element, db_name, db_table);
			return 0;
		}

		printer_init();			//initialize the printer

		if (strcmp(key_str, PRINT_G_EN) == 0)
		{//Gprinter
			line_feed();
			print_text_bold("饿了么 蓝牙打印帮助\n");
			line_feed();
			print_text("1.将手机蓝牙功能打开，打开饿了么商家版，点击左上角打开菜单，选择【系统设置】\n");
			print_text("2.选择【添加打印机】\n");
			sprintf(tmp_buff, "3.选择【%s %s】，输入配对密码 0000，进行绑定\n", key_str, element.bt_short_mac);
			print_text(tmp_buff);
			print_text("4.绑定成功后，即可测试打印\n");
			line_feed();
			line_feed();
			line_feed();
		}
		else
		{//MT
			print_text_bold("\n欢迎使用美团外卖蓝牙打印机\n");
			line_feed();
			print_text("http://e.waimai.meituan.com");
			line_feed();
			line_feed();
			line_feed();
			print_text_bold("iOS 版本帮助\n");
			line_feed();
			print_text("1.将手机蓝牙功能打开，打开美团外卖商家版，点击右下角【更多】，选择【配置蓝牙打印机】\n");
			sprintf(tmp_buff, "2.打开蓝牙打印，连接【%s %s】打印机\n", key_str, element.bt_short_mac);
			print_text(tmp_buff);
			print_text("3.等待连接完成后，测试打印成功，即代表绑定成功\n");
			print_text("4.在当前页面可以有选择的开启【自动打印】和【打印份数设置】\n");
			line_feed();
			line_feed();
			line_feed();
			print_text_bold("Android 版本帮助\n");
			line_feed();
			print_text("1.将手机蓝牙功能打开，打开美团外卖商家版，点击左上角打开菜单栏，选择【设置/打印机】\n");
			print_text("2.找到蓝牙打印机，选择【打印机】\n");
			print_text("3.点击图标【搜索打印机】\n");
			sprintf(tmp_buff, "4.选择【%s %s】\n", key_str, element.bt_short_mac);
			print_text(tmp_buff);
			print_text("5.输入配对密码 0000 进行绑定\n");
			print_text("5.当正上方显示已经绑定的设备，即代表绑定成功\n");
			print_text("7.返回设置页面测试打印，并选择开启【自动打印】和【打印份数设置】\n");
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

		usleep(200000);
		sprintf(tmp_buff, "rfcomm -i %s release /dev/rfcomm0", hci_dev);
		system(tmp_buff);
//		system("rfcomm -i hci0 release /dev/rfcomm0");
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

int get_local_bt_dev(char *pdev, int cmd)
{
	char read_buff[512];

	FILE *fp_connect = NULL;

	if (pdev  == NULL)
	{
		return -1;
	}

	fp_connect = popen("hcitool dev", "r");
	if (fp_connect == NULL)
	{
		return -1;
	}

	if (fgets(read_buff, sizeof(read_buff), fp_connect) == NULL)
	{

		pclose(fp_connect);

		return -1;
	}

	read_buff[500] = 0;

	if (strstr(read_buff, "hci0"))
	{
		strcpy(pdev, "hci0");
	}
	else
	{
		if (strstr(read_buff, "hci1"))
		{
			strcpy(pdev, "hci1");
		}
	}

	pclose(fp_connect);

	return 0;
}




