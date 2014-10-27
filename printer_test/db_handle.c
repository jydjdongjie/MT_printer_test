/*
 * db_handle.c
 *
 *  Created on: Oct 17, 2014
 *      Author: jack
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <pthread.h>
#include "db_handle.h"

extern long int print_interval_sec;

int db_open(const char *db_name, sqlite3 **ppDb);
int db_close(sqlite3* pDb);

pthread_mutex_t db_access_mutex = PTHREAD_MUTEX_INITIALIZER;

int insert_new_to_db(const char *db_name, const char *list_name, char *p_tab[], int count)
{
	int i, rc;
	char sql_buf[128];
	sqlite3 *db;
	bt_dev_element_t element;
	char tmp_buf[64];
	char *p = &tmp_buf[0];
	char *p1 = NULL;
	int ret_sq = 0;
	char *zErrMsg = NULL;

	int nRow, nColumn;
	char **dbResult;

	if (!(db_name && list_name))
	{
		return -1;
	}

	ret_sq = db_open(db_name, &db);
	if (ret_sq  != SQLITE_OK)
	{
		db_close(db);
		printf("Error in insert_to_db at sqlite3_open\n");
		return 0;//-1;
	}

	memset(sql_buf, '\0', 128);
	memset((void*)&element, 0, sizeof(element));
	sprintf(sql_buf, "create table %s(smac TEXT PRIMARY KEY, mac TEXT, name TEXT, flag INTEGER, utc INTEGER)", list_name);

	rc = sqlite3_exec(db, sql_buf, 0, 0, &zErrMsg);

	if (rc != SQLITE_OK && (strstr(zErrMsg, "already exists") == NULL))
	{
		printf("Sqlite3 Error [%d],Create table return :%s\n", rc, zErrMsg);
		db_close(db);
		return 0;//exit(1);
	}

	for (i = 0; i < count; i++)
	{
		memset(&tmp_buf[0], 0, sizeof(tmp_buf));

		strcpy(tmp_buf, p_tab[i]);
		p = &tmp_buf[0];
		while (*p != ' ' )
		{
			p++;
		}
		*p = 0;

		strcpy(element.bt_mac, tmp_buf);
		p++;
		p1 = p;

		while (*p != '\n')
		{
			p++;
		}
		*p = 0;
		strcpy(element.bt_name, p1);
		//
		{
			char mac_mem_tmp[32];
			mac_format(mac_mem_tmp, element.bt_mac);
			strcpy(element.bt_short_mac, &mac_mem_tmp[6]);
		}

		//
		sprintf(sql_buf, "select * from %s where smac=\'%s\';", list_name, element.bt_short_mac);
		rc = sqlite3_get_table( db, sql_buf, &dbResult, &nRow, &nColumn, &zErrMsg);

		if (rc != SQLITE_OK)
		{
			printf("Error rc = %d zErrMsg=%s\n", rc, zErrMsg);
		}
		else
		{
			if (nRow == 0)
			{
				element.bt_falg = 0;
				element.last_time = 0;

				sprintf(sql_buf, "insert into %s values(\'%s\', \'%s\', \'%s\', %d, %ld);",
						list_name, element.bt_short_mac, element.bt_mac, element.bt_name, element.bt_falg, element.last_time);

				sqlite3_exec(db, sql_buf, 0, 0, &zErrMsg);
			}
			else if (nRow == 1)
			{
				if (atoi(dbResult[nColumn+3]) < 0)
				{
					sprintf(sql_buf, "update %s set flag=%d where smac=\'%s\';", list_name, 0, element.bt_short_mac);
					sqlite3_exec(db, sql_buf, 0, 0, &zErrMsg);
				}
			}
		}
		sqlite3_free_table( dbResult);
	}

	db_close(db);

	return 0;

}


int get_new_from_db(int cmd, bt_dev_element_t *p_element, const char *db_name, const char *db_table)
{
	//select * from film limit 10;
	int rc, ret = 0;
	char sql_buf[128];
	sqlite3 *db;
	int ret_sq = 0;
	char *zErrMsg = NULL;
	int nRow, nColumn;
	char **dbResult;

	if (p_element == NULL)
	{
		return -1;
	}

	ret_sq = db_open(db_name, &db);
	if (ret_sq  != SQLITE_OK)
	{
		db_close(db);
		printf("Error in get_new_from_db at sqlite3_open\n");
		return 0;//return -1;
	}

	memset(sql_buf, '\0', 128);

	//select * from film limit 1;
	sprintf(sql_buf, "select * from %s where flag=%d limit 1;", db_table, cmd);
	rc = sqlite3_get_table( db, sql_buf, &dbResult, &nRow, &nColumn, &zErrMsg);

	if (rc == SQLITE_OK)
	{
		int index;

		if (nRow == 1)
		{
			index = nColumn;
			strcpy(p_element->bt_short_mac, dbResult[index+0]);
			strcpy(p_element->bt_mac, dbResult[index+1]);
			strcpy(p_element->bt_name, dbResult[index+2]);
			p_element->bt_falg = atoi(dbResult[index+3]);
		}
		else if(nRow == 0)
		{
			time_t tmp_t/*, tmp_t1*/ = time(NULL);

			if (tmp_t > print_interval_sec)
			{
				tmp_t = tmp_t - print_interval_sec;

				sprintf(sql_buf, "select * from %s where flag > %d and utc < %ld limit 1;", db_table, cmd, tmp_t);
				sqlite3_free_table( dbResult);
				rc = sqlite3_get_table( db, sql_buf, &dbResult, &nRow, &nColumn, &zErrMsg);

				if (rc == SQLITE_OK && nRow > 0)
				{
					index = nColumn;

					strcpy(p_element->bt_short_mac, dbResult[index+0]);
					strcpy(p_element->bt_mac, dbResult[index+1]);
					strcpy(p_element->bt_name, dbResult[index+2]);
					p_element->bt_falg = atoi(dbResult[index+3]);
					p_element->last_time = atoi(dbResult[index+4]);
				}
			}
		}

		ret = nRow;
	}

	sqlite3_free_table( dbResult);

	db_close(db);

	return ret;
}


int update_flag_to_db(int flag, bt_dev_element_t *p_element, const char *db_name, const char *db_table)
{
	int rc;
	char sql_buf[128];
	sqlite3 *db;
	int ret_sq = 0;
	char *zErrMsg = NULL;

	if (p_element == NULL)
	{
		return -1;
	}

	ret_sq = db_open(db_name, &db);
	if (ret_sq  != SQLITE_OK)
	{
		db_close(db);
		printf("Error in get_new_from_db at sqlite3_open\n");
		return -1;
	}

	memset(sql_buf, '\0', 128);

	p_element->last_time = time(NULL);

	sprintf(sql_buf, "update %s set flag=%d, utc=%ld where smac=\'%s\';", db_table, flag, p_element->last_time, p_element->bt_short_mac);
	rc = sqlite3_exec(db, sql_buf, 0, 0, &zErrMsg);

	if (rc  != SQLITE_OK)
	{
		printf("Sqlite3 error:%s\n", sql_buf);
	}

	db_close(db);

	return 0;
}

int print_db_all(const char *db_name, const char *table)
{
	int rc, ret = 0;
	char sql_buf[128];
	sqlite3 *db;
	int ret_sq = 0;
	char *zErrMsg = NULL;
	int nRow, nColumn;
	char **dbResult;

	ret_sq = db_open(db_name, &db);
	if (ret_sq  != SQLITE_OK)
	{
		db_close(db);
		printf("Error in get_new_from_db at sqlite3_open\n");
		return -1;
	}

	memset(sql_buf, '\0', 128);

	sprintf(sql_buf, "select * from %s;", table);
	rc = sqlite3_get_table( db, sql_buf, &dbResult, &nRow, &nColumn, &zErrMsg);

	printf("\n++++++++++++++Table:%s nRow:%d nColumn:%d+++++++++++++++\n", table, nRow, nColumn);
	if (rc == SQLITE_OK)
	{
		int j, k, index = nColumn;

		for (j = 0; j < nRow; j++)
		{
			for (k = 0; k < nColumn; k++)
			{
				printf("%s ", dbResult[index++]);
			}
			printf("\n");
		}
	}
	printf("\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	sqlite3_free_table( dbResult);

	db_close(db);

	return ret;
}


void mac_format(char *des_mac, const char *src_mac)
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


int db_open(const char *db_name, sqlite3 **ppDb)
{
	pthread_mutex_lock (&db_access_mutex);

	return sqlite3_open(db_name, ppDb);
}


int db_close(sqlite3* pDb)
{
	int ret = sqlite3_close(pDb);

	pthread_mutex_unlock (&db_access_mutex);

	return ret;
}









