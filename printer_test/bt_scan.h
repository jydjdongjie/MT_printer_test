/*
 * bt_scan.h
 *
 *  Created on: Oct 16, 2014
 *      Author: jack
 */

#ifndef BT_SCAN_H_
#define BT_SCAN_H_

#define SCAN_BT_DEV 		"hci0"
#define SEARCH_KEY_STR		"美团外卖 "
#define SEATCH_KEY_STR_G	"Gprinter "
#define SCAN_DELAY_SEC		(5)

void *task_bt_scan(void *parg);
int bt_scan_do(const char *hci_dev, const char *key_str, const char *table);

#endif /* BT_SCAN_H_ */
