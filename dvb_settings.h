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

#include <QString>
#include <QDebug>
#include <QPointer>
#include <QTreeWidget>
#include <linux/dvb/frontend.h>

#define MAX_LNBS 16

#define GRAY QColor(80,80,80)
#define DGRAY QColor(60,60,60)
#define DGREEN QColor(0,100,0)
#define GREEN QColor(0,255,0)

#define TNY_BUFSIZE (20*188)
#define LIL_BUFSIZE (348*188)
#define BIG_BUFSIZE (30*LIL_BUFSIZE)
#define MAX_PES_SIZE (4*1024)
#define DMXOFF 5
#define PACKETRETRY 3
#define PERSISTENCE 30

// Status bits
#define TUNER_AVAIL			1  // free todo anything
#define TUNER_IOCTL			2  // busy sending ioctl
#define TUNER_IOCTL_QUEUE	4  // busy sending ioctl
#define TUNER_TUNED			8  // tuned
#define TUNER_DEMUX			16 // demux'ing
#define TUNER_RDING			32 // read()'ing
#define TUNER_SCAN			64 // busying scanning

#define STATUS_NOEXP   0
#define STATUS_REMOVE -1
#define STATUS_CLEAR  -2

struct asc1_data
{
	QString			name;
	unsigned int	counter;
	int				Hdeg;
	int				Vdeg;
};

class switch_settings
{
public:
	int voltage;
	int tone;
	int committed;
	int uncommitted;

	switch_settings();
};

class ac3_desc
{
public:
	ac3_desc();
	QVector<QString> sample_rate_code;
	QVector<QString> bit_rate_code;
	QVector<QString> dsurmod;
	QVector<QString> num_channels;
	QVector<QString> priority;
	QVector<QString> bsmode;
};

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
	int frame_len;
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
	QString name;
	
	tuning_options();
};

class data_service
{
public:
	data_service();

	QVector<QString> text;
};

class compression_type
{
public:
	compression_type();
	QString whatis(unsigned int val);

	QVector<QString> text;
	QVector<unsigned int> min;
	QVector<unsigned int> max;
};

class audio_type
{
public:
	audio_type();
	QString whatis(unsigned int val);

	QVector<QString> text;
	QVector<unsigned int> min;
	QVector<unsigned int> max;
};

class encoding_mode
{
public:
	encoding_mode();
	QString whatis(unsigned int val);

	QVector<QString> text;
	QVector<unsigned int> min;
	QVector<unsigned int> max;
};

class cue_stream
{
public:
	cue_stream();
	QString whatis(unsigned int val);

	QVector<QString> text;
	QVector<unsigned int> min;
	QVector<unsigned int> max;
};

class atsc_service_type
{
public:
	atsc_service_type();
	QString whatis(unsigned int val);

	QVector<QString> text;
	QVector<unsigned int> min;
	QVector<unsigned int> max;
};

class dvb_service_type
{
public:
	dvb_service_type();
	QString whatis(unsigned int val);

	QVector<QString> text;
	QVector<unsigned int> min;
	QVector<unsigned int> max;
};

class etm_location
{
public:
	etm_location();

	QVector<QString> text;
};

class atsc_modulation
{
public:
	atsc_modulation();
	QString whatis(unsigned int val);

	QVector<QString> text;
	QVector<unsigned int> min;
	QVector<unsigned int> max;
};

class mgt_table
{
public:
	mgt_table();
	QString whatis(unsigned int val);

	QVector<QString> text;
	QVector<unsigned int> min;
	QVector<unsigned int> max;
};

class rating_region
{
public:
	rating_region();
	QVector<QString> name;
};

class dvb_settings
{
public:
	dvb_settings();
	QVector<QString> fec_name;
	QString fec(int val);
	QVector<QString> system_name;
	QString system(int val);
	QVector<QString> modulation_name;
	QString modulation(int val);
	QVector<QString> dtag_modulation;
	QVector<QString> dtag_rolloff;
	QVector<QString> dtag_fec;
	QVector<QString> dtag_polarization;
	QVector<QString> rolloff_name;
	QString rolloff(int val);
	QVector<QString> pilot_name;
	QString pilot(int val);
	QVector<QString> inversion_name;
	QString inversion(int val);
	QVector<QString> tone;
	QVector<QString> voltage;
	QVector<QString> stream_type;
	QVector<QString> table_name;
	QVector<QString> ca_name;
	QVector<QString> dvb_descriptortag;
};

class private_data_specifier
{
public:
	private_data_specifier();
	QString whois(unsigned long int val);

	QVector<QString> text;
	QVector<unsigned long int> min;
	QVector<unsigned long int> max;
};

class data_broadcast_id
{
public:
	data_broadcast_id();
	QString whatis(unsigned int val);

	QVector<QString> text;
	QVector<unsigned int> min;
	QVector<unsigned int> max;
};

class stream_content
{
public:
	stream_content();
	QString whatis(unsigned int stream_type, unsigned int component_type);
	QVector<QString> text;
	QVector<unsigned int> sval;
	QVector<unsigned int> cmin;
	QVector<unsigned int> cmax;
};

class frame_rate
{
public:
	frame_rate();
	QVector<QString> rate;
};

class data_stream_type
{
public:
	data_stream_type();
	QVector<QString> name;
};

class tree_item
{
public:
	tree_item();
	void save();
	void restore();

	int parent;
	int current;
	int saved;
	QString text;
	QColor color;
	bool expanded;
	bool return_parent;
	unsigned int pid;
	unsigned int table;
	QTreeWidgetItem * tree;
};

class dvb_pat
{
public:
	QVector<unsigned int> number;
	QVector<unsigned int> pid;
};

class dvb_pids
{
public:
	QVector<unsigned int> pid;
	QVector<unsigned int> tbl;
};

class dvb_data
{
public:
	unsigned int pid;
	unsigned int table;
	QByteArray buffer;
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
	unsigned int pmt_pid;
	unsigned int pmt_num;
	QVector<unsigned int> desc_type;
	QVector<unsigned int> desc_pid;
	QVector<dvb_descriptor> tag;
};

class dvb_sdt
{
public:
	QVector<unsigned int> sid;
	QVector<QString> sname;
	QVector<QString> pname;
};

class freq_list
{
public:
	freq_list();
	void qam();
	void atsc();
	void dvbt();
	QVector<int> freq;
	QVector<int> ch;
};

bool isSatellite(int system);
bool isATSC(int system);
bool isVectorQAM(QVector<int> system);
bool isVectorATSC(QVector<int> system);
bool isVectorDVBT(QVector<int> system);
bool isQAM(int system);
bool isDVBT(int system);
int azero(int num);
QString tohex(unsigned long val, int length);

#endif // DVB_SETTINGS_H
