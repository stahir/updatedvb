/*
 *	updateDVB, a DVB/ATSC spectrum analyzer, tuning and stream analyzing application.
 *	Copyright (C) 2013  Chris Lee (updatelee@gmail.com)
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DVBTUNE_H
#define DVBTUNE_H

#include <QDebug>
#include <QDialog>
#include <QByteArray>
#include <QThread>
#include <QString>
#include <QTime>
#include <QTcpServer>
#include <QTcpSocket>
#include <iostream>
#include <iomanip>
#include <sys/ioctl.h>
#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <sstream>
#include <math.h>
#include "dvb_settings.h"
using namespace std;

#define BUFFY (188*348*30)
#define TCP_BUFSIZE (188*348)
#define MAX_PES_SIZE (4*1024)
#define DMXOFF 5
#define PACKETRETRY 3
#define PERSISTENCE 30

class dvbtune : public QThread
{
	Q_OBJECT
signals:
	void updatesignal();
	void updateresults();
	void iqdraw(QVector<short int> x, QVector<short int> y);
	void adapter_status(int adapter, bool is_busy);
	void demux_status(int bytes);
	
public:
	dvbtune();
	~dvbtune();
	unsigned int adapter;
	unsigned int frontend;

	tp_info				tp;
	QVector<tp_info>	tp_try;
	tuning_options		tune_ops;
	switch_settings		myswitch;

	int index;
	QByteArray buffer;

	int frontend_fd, dvr_fd, sct_fd, out_fd;
	QVector<int> dmx_fd;
	QString frontend_name, dvr_name, dmx_name, sct_name, out_name;
	QVector<int> pids;
	QVector<int> pids_rate;
	double old_position;

	unsigned char iq_options;
	QVector<short int> iq_x;
	QVector<short int> iq_y;	
	
	int crc32();
	void check_frontend();
	void get_bitrate();
	int tune();
	int demux_packet(int pid, unsigned char table = 0, int timeout = 3000);
	void demux_video();
	void demux_file();
	QByteArray demux_stream();
	void stop_demux();
	void setup_switch();
	void spectrum_scan(dvb_fe_spectrum_scan *scan);
	void openfd();
	void closefd();
	void close_dvr();
	void getops();
	unsigned int maskbits(unsigned int value, unsigned int mask = 0xFFFFFFFF);
	unsigned int read32(unsigned int mask = 0xFFFFFFFF);
	unsigned int read24(unsigned int mask = 0xFFFFFFFF);
	unsigned int read16(unsigned int mask = 0xFFFFFFFF);
	unsigned int read8(unsigned int mask = 0xFFFFFFFF);
	QString readstr(unsigned int pos, unsigned int len);
	void usals_drive(double sat_long);
	void gotox_drive(int position);
	void gotox_save(int position);
	void step_motor(int direction, int steps);
	void iqplot();
	
	void run();
	bool loop;
	bool ready;

	QVector<int> delsys;
	u_int64_t caps;
	QString name;
	int fmin;
	int fmax;
	int fstep;
	bool servo;
	QVector<QString> thread_function;
private:
	double radian(double number);
	double degree(double number);
	
	dvb_settings dvbnames;
};

#endif // DVBTUNE_H
