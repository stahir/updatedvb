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

#ifndef DVB_SETTINGS_H
#define DVB_SETTINGS_H

#include <QVector>
#include <QString>
#include <linux/dvb/frontend.h>

#define MAX_LNBS 16

class tp_info
{
public:
	int frequency;
	int voltage;
	int symbolrate;
	int fec;
	int system;
	int modulation;
	int inversion;
	int rolloff;
	int pilot;
	int matype;
	unsigned int ucb;
	unsigned int ber;
	unsigned int ber_scale;
	float snr;
	unsigned int snr_scale;
	float lvl;
	unsigned int lvl_scale;
	int spectrumscan_lvl;
	int status;
	
	tp_info();
};

class tuning_options
{
public:
	int mis;
	int f_start;
	int f_stop;
	int f_lof;
	int voltage;
	int tone;
	int committed;
	int uncommitted;
	double site_lat, site_long;
	
	tuning_options();
};

class dvb_settings
{
public:
	dvb_settings();
	QVector<QString> fec;
	QVector<QString> system;
	QVector<QString> modulation;
	QVector<QString> dtag_modulation;
	QVector<QString> dtag_rolloff;
	QVector<QString> dtag_fec;
	QVector<QString> dtag_polarization;
	QVector<QString> rolloff;
	QVector<QString> pilot;
	QVector<QString> inversion;
	QVector<QString> tone;
	QVector<QString> voltage;
	QVector<QString> stream_type;
	QVector<QString> table_name;
	QVector<QString> ca_name;
	QVector<QString> dvb_descriptortag;
};

class dvb_pat
{
public:
	QVector<unsigned int> number;
	QVector<unsigned int> pid;
};

class dvb_ca
{
public:
	QVector<unsigned int> system_id;
	QVector<unsigned int> pid;
};

class dvb_descriptor
{
public:
	QVector<unsigned int> id;
};

class dvb_pmt
{
public:
	unsigned int pcr;
	dvb_ca myca;
	QVector<unsigned int> type;
	QVector<unsigned int> pid;
	QVector<dvb_descriptor> tag;
};

class dvb_sdt
{
public:
	QVector<unsigned int> sid;
	QVector<QString> sname;
	QVector<QString> pname;
};

class atsc
{
public:
	atsc();
	QVector<int> freq;
	QVector<int> ch;
};

class qam
{
public:
	qam();
	QVector<int> freq;
	QVector<int> ch;
};

bool isSatellite(int system);
bool isATSC(int system);
bool isQAM(int system);

#endif // DVB_SETTINGS_H
