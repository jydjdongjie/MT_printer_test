/*
 * esc_deal.h
 *
 *  Created on: Oct 21, 2014
 *      Author: jack
 */

#ifndef ESC_DEAL_H_
#define ESC_DEAL_H_

//operate the com to printer
int open_port(const char *path);
int close_port(void);

//config & use the printer
int printer_init(void);
int print_barcode(const char *pdata);
int print_text(const char *data);
int print_text_bold(const char *pdata);
int line_feed(void);

#endif /* ESC_DEAL_H_ */
