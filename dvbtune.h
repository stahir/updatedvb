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
#include <QByteArray>
#include <QString>
#include <QTime>
#include <sys/ioctl.h>
#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>
#include <math.h>
#include "dvb_settings.h"
#include "dvr_thread.h"

class dvr_thread;

class dvbtune : public QThread
{
	Q_OBJECT
signals:
	void update_signal();
	void update_results();
	void update_status(QString text, int time);
	void iqdraw(QVector<short int> x, QVector<short int> y);
	void adapter_status(int adapter);
	void demux_status(int bytes);
	void send_stream(QByteArray);

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
	QVector<QByteArray> packet_buffer;
	QVector<QByteArray> packet_processed;

	int frontend_fd, dvr_fd, out_fd;
	QVector<int> dmx_fd;
	QVector<int> sec_fd;
	QString frontend_name, dvr_name, dmx_name, sec_name, out_name;
	struct timeval fd_timeout;
	QVector<unsigned int> pids;
	QVector<unsigned int> pids_rate;
	double old_position;

	unsigned char iq_options;
	QVector<short int> iq_x;
	QVector<short int> iq_y;	
	
	dvr_thread *mydvr;

	int crc32();
	void check_frontend();
	void get_bitrate();
	int tune();
	int tune_clear();
	int demux_packets(QVector<unsigned int> filter_pids);
	void demux_file(bool start);
	void demux_video();
	void demux_bbframe();
	void demux_stream(bool start);
	void stop_demux();
	void close_demux();
	void setup_switch();
	void spectrum_scan(dvb_fe_spectrum_scan *scan);
	void openfd();
	void closefd();
	void close_dvr();
	void getops();
	__u64 maskbits(__u64 value, __u64 mask = 0xFFFFFFFFFFFFFFFF);
	__u64 read64(__u64 mask = 0xFFFFFFFFFFFFFFFF);
	__u64 read56(__u64 mask = 0x00FFFFFFFFFFFFFF);
	__u64 read48(__u64 mask = 0x0000FFFFFFFFFFFF);
	__u64 read40(__u64 mask = 0x000000FFFFFFFFFF);
	__u32 read32(__u32 mask = 0xFFFFFFFF);
	__u32 read24(__u32 mask = 0x00FFFFFF);
	__u16 read16(__u16 mask = 0xFFFF);
	__u8 read8(__u8 mask = 0xFF);
	QString readstr(unsigned int pos, unsigned int len);
	void usals_drive(double sat_long);
	void gotox_drive(int position);
	void gotox_save(int position);
	void step_motor(int direction, int steps);
	void iqplot();
	QString min_snr();
	int closest_freq(int freq, int system);
	QString format_freq(int freq, int system);
	
	void run();
	bool loop;
	unsigned int status;

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
