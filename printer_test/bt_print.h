/*
 * bt_print.h
 *
 *  Created on: Oct 16, 2014
 *      Author: jack
 */

#ifndef BT_PRINT_H_
#define BT_PRINT_H_


#define PRINT_DELAY_SEC (1)
#define PRINT_BT_DEV	"hci1"
#define PRINT_MT_CN	"√¿Õ≈Õ‚¬Ù "
#define PRINT_G_EN "Gprinter"

int bt_print_do(const char *hci_dev, const char *key_str, const char *db_name, const char *db_table);
void *task_bt_print(void *parg);
int get_local_bt_dev(char *pdev, int cmd);
#endif /* BT_PRINT_H_ */


