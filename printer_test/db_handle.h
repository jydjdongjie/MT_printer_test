/*
 * db_handle.h
 *
 *  Created on: Oct 17, 2014
 *      Author: jack
 */

#ifndef DB_HANDLE_H_
#define DB_HANDLE_H_

#include <time.h>

#define BT_DB_NAME 	"/home/pi/bt_db/MT.db"
#define MT_DB_TABLE	"mt_table"
#define G_DB_TABLE	"Gprinter_table"

typedef struct
{
	char bt_mac[18];
	char bt_name[20];
	char bt_short_mac[7];
	int bt_falg;
	time_t last_time;
}bt_dev_element_t;


int insert_new_to_db(const char *db_name, const char *list_name, char *p_tab[], int count);
int get_new_from_db(int cmd, bt_dev_element_t *p_element, const char *db_name, const char *db_table);
int update_flag_to_db(int flag, bt_dev_element_t *p_element, const char *db_name, const char *db_table);
int print_db_all(const char *db_name, const char *table);
void mac_format(char *des_mac, const char *src_mac);

#endif /* DB_HANDLE_H_ */


