/*
 * esc_deal.c
 *
 *  Created on: Oct 21, 2014
 *      Author: jack
 */

#include "esc_deal.h"
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

static int com_fd = 0;

static int justify(char align);
static int send_esc_to_printer(const char *cmd, int len);
static int barcode_chr(char chr);
static int set_barcode_height(char height);
static int set_barcode_width(char width);
static int send_data(const char *pdata, int len);
static int set_font_bold_on(void);
static int set_font_bold_off(void);


int open_port(const char *path)
{
	com_fd = open(path, O_RDWR | O_NOCTTY | O_NDELAY);
	if (com_fd < 0)
	{
		printf("open_port: Unable to open %s\n", path);
		return -1;
	}

	fcntl(com_fd, F_SETFL, 0);
	return (com_fd);
}


int close_port(void)
{
	if (com_fd < 0)
	{
		return -1;
	}

	close(com_fd);
	return 0;
}

int print_barcode(const char *pdata)
{
	char sndbuf[128] = {'\x1D', '\x6B', '\x49', '\x0', '\x7B', '\x42',};
	int len = strlen(pdata);

	justify(1);//align center
	barcode_chr('0');
	set_barcode_height(80);
	set_barcode_width(2);

	sndbuf[3] = len + 2;
	memcpy(&sndbuf[6], pdata, len);

	return send_data(sndbuf, 6 + len);
}


int printer_init(void)
{
	char sndbuf[128] = {
			"\x1B\x40"			//initialize
			"\x1B\x73\x1"		//speed
			"\x1B\x6D\x8"		//Light-Dark:1-8
			"\x1B\x4D\x2"		//# 0:24X24 1:16X16 2:12X12
			"\x1C\x26"			//
			"\x1B\x52\x0F"		//
	};
	return send_data(sndbuf, 16);
}

int print_text(const char *data)
{
	return send_data(data, strlen(data));
}

int print_text_bold(const char *pdata)
{
	int ret = 0;

	ret = set_font_bold_on();
	if (ret < 0)
	{
		return ret;
	}

	ret = print_text(pdata);
	if (ret < 0)
	{
		return ret;
	}

	ret = set_font_bold_off();
	return ret;
}

int line_feed(void)
{
	return send_esc_to_printer("\x0A", 1);
}

static int justify(char align)
{
	char buf[3] = "\x1B\x61";

	buf[2] = align;

	return send_esc_to_printer(buf, 3);
}

static int send_esc_to_printer(const char *cmd, int len)
{
	return send_data(cmd, len);
}

static int barcode_chr(char chr)
{
	char buf[3] = "\x1D\x48";

	buf[2] = chr;

	return send_esc_to_printer(buf, 3);
}

static int set_barcode_height(char height)
{
	char buf[3] = "\x1D\x68";
	buf[2] = height;
	return send_esc_to_printer(buf, 3);
}

static int set_barcode_width(char width)
{
	char buf[3] = "\x1D\x77";
	buf[2] = width;
	return send_esc_to_printer(buf, 3);
}



static int send_data(const char *pdata, int len)
{
	if (com_fd < 0 || pdata == NULL)
	{
		return -1;
	}

	return write(com_fd, pdata, len);
}


static int set_font_bold_on(void)
{
	return send_esc_to_printer("\x1B\x21\x30", 3);
}

static int set_font_bold_off(void)
{
	return send_esc_to_printer("\x1B\x21\x0", 3);
}




