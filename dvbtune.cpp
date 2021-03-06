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

#include "dvbtune.h"

dvbtune::dvbtune()
{
	dvr_name.clear();
	dmx_name.clear();
	out_name.clear();
	frontend_name.clear();
	status		= TUNER_AVAIL;
	loop		= false;
	iq_options	= 0x00;
	adapter		= 0;
	frontend	= 0;
	index		= 0;
	dmx_fd.clear();
	frontend_fd	= -1;
	dvr_fd		= -1;
	out_fd		= -1;
	old_position	= 0;
	fmin		= 0;
	fmax		= 0;
	fstep		= 0;
	motor_delay	= 0;
	caps		= 0;
	extended_caps	= 0;
	servo		= false;
	fd_timeout.tv_sec	= 1;
	fd_timeout.tv_usec	= 0;
	mydvr			= new dvr_thread;
	mydvr->mytune	= this;
	closefd_timer   = new QTimer(this);
	connect(closefd_timer, SIGNAL(timeout()), this, SLOT(closefd()));
}

dvbtune::~dvbtune()
{
	mydvr->loop = false;
	mydvr->quit();
	mydvr->wait(1000);
	while (mydvr->isRunning()) {
		qDebug() << "mydvr->isRunning()";
		msleep(100);
	}
	mydvr->deleteLater();

	closefd();
}

__u64 dvbtune::maskbits(__u64 value, __u64 mask)
{
	value = value & mask;
	for(int i = 0; i < 64 && !((mask >> i) & 0x01); i++) {
		value = value >> 1;
	}
	return value;
}

__u64 dvbtune::read64(__u64 mask)
{
	if (index+7 >= (unsigned int)buffer.size()) {
		return 0;
	}

	return ((__u64)read8((mask & 0xFF00000000000000) >> 56) << 56) +
		   ((__u64)read8((mask & 0x00FF000000000000) >> 48) << 48) +
		   ((__u64)read8((mask & 0x0000FF0000000000) >> 40) << 40) +
		   ((__u64)read8((mask & 0x000000FF00000000) >> 32) << 32) +
		   ((__u32)read8((mask & 0x00000000FF000000) >> 24) << 24) +
		   ((__u32)read8((mask & 0x0000000000FF0000) >> 16) << 16) +
		   ((__u16)read8((mask & 0x000000000000FF00) >>  8) <<  8) +
				   read8((mask & 0x00000000000000FF));
}

__u64 dvbtune::read56(__u64 mask)
{
	if (index+6 >= (unsigned int)buffer.size()) {
		return 0;
	}

	return ((__u64)read8((mask & 0x00FF000000000000) >> 48) << 48) +
		   ((__u64)read8((mask & 0x0000FF0000000000) >> 40) << 40) +
		   ((__u64)read8((mask & 0x000000FF00000000) >> 32) << 32) +
		   ((__u32)read8((mask & 0x00000000FF000000) >> 24) << 24) +
		   ((__u32)read8((mask & 0x0000000000FF0000) >> 16) << 16) +
		   ((__u16)read8((mask & 0x000000000000FF00) >>  8) <<  8) +
				   read8((mask & 0x00000000000000FF));
}

__u64 dvbtune::read48(__u64 mask)
{
	if (index+5 >= (unsigned int)buffer.size()) {
		return 0;
	}

	return ((__u64)read8((mask & 0x0000FF0000000000) >> 40) << 40) +
		   ((__u64)read8((mask & 0x000000FF00000000) >> 32) << 32) +
		   ((__u32)read8((mask & 0x00000000FF000000) >> 24) << 24) +
		   ((__u32)read8((mask & 0x0000000000FF0000) >> 16) << 16) +
		   ((__u16)read8((mask & 0x000000000000FF00) >>  8) <<  8) +
				   read8((mask & 0x00000000000000FF));
}

__u64 dvbtune::read40(__u64 mask)
{
	if (index+4 >= (unsigned int)buffer.size()) {
		return 0;
	}

	return ((__u64)read8((mask & 0x000000FF00000000) >> 32) << 32) +
		   ((__u32)read8((mask & 0x00000000FF000000) >> 24) << 24) +
		   ((__u32)read8((mask & 0x0000000000FF0000) >> 16) << 16) +
		   ((__u16)read8((mask & 0x000000000000FF00) >>  8) <<  8) +
				   read8((mask & 0x00000000000000FF));
}

__u32 dvbtune::read32(__u32 mask)
{
	if (index+3 >= (unsigned int)buffer.size()) {
		return 0;
	}

	return ((__u32)read8((mask & 0xFF000000) >> 24) << 24) +
		   ((__u32)read8((mask & 0x00FF0000) >> 16) << 16) +
		   ((__u16)read8((mask & 0x0000FF00) >>  8) <<  8) +
				   read8((mask & 0x000000FF));
}

__u32 dvbtune::read24(__u32 mask)
{
	if (index+2 >= (unsigned int)buffer.size()) {
		return 0;
	}

	return ((__u32)read8((mask & 0x00FF0000) >> 16) << 16) +
		   ((__u16)read8((mask & 0x0000FF00) >>  8) <<  8) +
				   read8((mask & 0x000000FF));
}

__u16 dvbtune::read16(__u16 mask)
{
	if (index+1 >= (unsigned int)buffer.size()) {
		return 0;
	}

	return ((__u16)read8((mask & 0xFF00) >> 8) << 8) +
				   read8((mask & 0x00FF));
}

__u8 dvbtune::read8(__u8 mask)
{
	if (index >= (unsigned int)buffer.size()) {
		return 0;
	}

	return (unsigned char)buffer.at(index++) & mask;
}

QString dvbtune::readstr(unsigned int len)
{
	unsigned int tmp = index;
	index += len;
	if (len >= (unsigned int)buffer.size()) {
		return "";
	}
	return buffer.mid(tmp, len);
}

QString dvbtune::readstr16(unsigned int len)
{
	QString tmp_s;
	len *= 2;
	len += index;
	if (len >= (unsigned int)buffer.size()) {
		return "";
	}
	while ((unsigned int)index < len) {
		index++;
		__u8 tmp_c = read8();
		if (tmp_c != 0x00) {
			tmp_s.append(tmp_c);
		}
	}
	return tmp_s;
}

void dvbtune::closefd()
{
	if ((status & TUNER_TUNED) || (status & TUNER_SCAN)) {
		return;
	}

	if (closefd_timer->isActive()) {
		closefd_timer->stop();
	}

	close_demux();
	close_dvr();

	if (!frontend_name.isEmpty()) {
		close(frontend_fd);
		frontend_fd = -1;
		frontend_name.clear();
		unsetbit(TUNER_TUNED);
	}

	myswitch.tone			= -1;
	myswitch.voltage		= -1;
	myswitch.committed		= -1;
	myswitch.uncommitted	= -1;
}

bool dvbtune::openfd()
{
	if (frontend_name.isEmpty()) {
		frontend_name = "/dev/dvb/adapter" + QString::number(adapter) + "/frontend" + QString::number(frontend);
		frontend_fd = open(frontend_name.toStdString().c_str(), O_RDWR|O_NONBLOCK);
		if (frontend_fd < 0) {
			qDebug() << "Failed to open" << frontend_name << "-" << strerror(errno);
			unsetbit(TUNER_AVAIL);
			emit adapter_status(adapter);
			emit update_status("Tuner not available", 1);
			frontend_fd = -1;
			frontend_name.clear();
			return false;
		}
		setbit(TUNER_AVAIL);
	}
	return true;
}

double dvbtune::radian( double number )
{
	return number*M_PI/180;
}

double dvbtune::degree( double number )
{
	return number*180/M_PI;
}

void dvbtune::getops()
{
	struct dtv_property p[1];
	p[0].cmd = DTV_ENUM_DELSYS;

	struct dtv_properties p_status;
	p_status.num = 1;
	p_status.props = p;

	if (!ioctl_FE_GET_PROPERTY(&p_status)) {
		unsetbit(TUNER_AVAIL);
		return;
	}

	delsys.clear();
	for (;p[0].u.buffer.len > 0; p[0].u.buffer.len--) {
		delsys.append(p[0].u.buffer.data[p[0].u.buffer.len - 1]);
	}
	
	struct dvb_frontend_info fe_info;
	ioctl_FE_GET_INFO(&fe_info);

	caps	= fe_info.caps;
	name	= fe_info.name;
	fmin	= fe_info.frequency_min;
	fmax	= fe_info.frequency_max;
	fstep	= fe_info.frequency_stepsize;

	if (caps & FE_HAS_EXTENDED_CAPS) {
		struct dvb_frontend_extended_info fe_extended_info;
		ioctl_FE_GET_EXTENDED_INFO(&fe_extended_info);
		extended_caps = fe_extended_info.extended_caps;
	}

	if (!(status & TUNER_TUNED) && !(status & TUNER_SCAN)) {
		closefd();
	}
}

void dvbtune::step_motor(int direction, int steps)
{
	// 0x68 = East
	// 0x69 = West
	// 0xFF = 1 step
	struct dvb_diseqc_master_cmd diseqc_cmd = { { 0xe0, 0x31, (__u8)(0x68 + direction), (__u8)(0xFF - (steps-1)), 0x00, 0x00 }, 4 };
	
	ioctl_FE_SET_VOLTAGE(tp.voltage);

	if (myswitch.tone == (int)!SEC_TONE_ON) { // SEC_TONE_ON == 0
		myswitch.tone = -1;
		ioctl_FE_SET_TONE(SEC_TONE_OFF);
	}
	ioctl_FE_DISEQC_SEND_MASTER_CMD(diseqc_cmd);

	closefd_timer->start(1000);
}

void dvbtune::usals_drive(double sat_long)
{
	double r_eq = 6378.14;		// Earth radius
	double r_sat = 42164.57;	// Distance from earth centre to satellite

	double site_lat  = radian(tune_ops.site_lat);
	double site_long = radian(tune_ops.site_long);
	sat_long  = radian(sat_long);

	double declination = degree( atan( r_eq * sin(site_lat) / ( (r_sat - r_eq) + (r_eq * (1 - cos(site_lat))) ) ) );

	// x = [0], y = [1], z = [2]
	double dishVector[3] = { r_eq * cos(site_lat), 0, r_eq * sin(site_lat) };
	double satVector[3] = { r_sat * cos(site_long - sat_long), r_sat * sin(site_long - sat_long), 0 };
	double satPointing[3] = { satVector[0] - dishVector[0], satVector[1] - dishVector[1], satVector[2] - dishVector[2] } ;

	double motor_angle = degree( atan( satPointing[1]/satPointing[0] ) );

	int sixteenths = fabs(motor_angle) * 16.0 + 0.5;
	int angle_1, angle_2;
	angle_1 = motor_angle > 0.0 ? 0xd0 : 0xe0;
	angle_1 |= sixteenths >> 8;
	angle_2  = sixteenths & 0xff;

	qDebug()<< "Long:" << degree(site_long) 
			<< "Lat:" << degree(site_lat )
			<< "Orbital Pos:" << degree(sat_long) 
			<< "RotorCmd:" << hex << angle_1 << angle_2 << dec
			<< "motor_angle:" << motor_angle
			<< "declination:" << declination;

	struct dvb_diseqc_master_cmd diseqc_cmd = { { 0xe0, 0x31, 0x6e, (__u8)angle_1, (__u8)angle_2, 0x00 }, 5 };

	ioctl_FE_SET_VOLTAGE(tp.voltage);

	if (myswitch.tone == (int)!SEC_TONE_ON) { // SEC_TONE_ON == 0
		myswitch.tone = -1;
		ioctl_FE_SET_TONE(SEC_TONE_OFF);
	}
	ioctl_FE_DISEQC_SEND_MASTER_CMD(diseqc_cmd);

	sat_long = degree(sat_long);
	int howlong;
	if (!old_position) {
		howlong = 45000;
	} else {
		howlong = abs(old_position - sat_long) * motor_delay;
	}
	qDebug() << "Motor should take aprox" << howlong/1000 << "sec to move from" << old_position << "to" << sat_long;

	old_position = sat_long;
	closefd_timer->start(howlong);
}

void dvbtune::gotox_drive(unsigned int loc, float pos)
{
	struct dvb_diseqc_master_cmd diseqc_cmd = { { 0xe0, 0x31, 0x6B, (__u8)loc, 0x00, 0x00 }, 4 };
	
	ioctl_FE_SET_VOLTAGE(tp.voltage);

	if (myswitch.tone == (int)!SEC_TONE_ON) { // SEC_TONE_ON == 0
		myswitch.tone = -1;
		ioctl_FE_SET_TONE(SEC_TONE_OFF);
	}
	ioctl_FE_DISEQC_SEND_MASTER_CMD(diseqc_cmd);

	int howlong;
	if (!old_position) {
		howlong = 45000;
	} else {
		howlong = abs(old_position - pos) * motor_delay;
	}
	qDebug() << "Motor should take aprox" << howlong/1000 << "sec to move from" << old_position << "to" << pos;

	old_position = pos;
	closefd_timer->start(howlong);
}

void dvbtune::gotox_save(unsigned int position)
{
	struct dvb_diseqc_master_cmd diseqc_cmd = { { 0xe0, 0x31, 0x6A, (__u8)position, 0x00, 0x00 }, 4 };

	if (myswitch.tone == (int)!SEC_TONE_ON) { // SEC_TONE_ON == 0
		myswitch.tone = -1;
		ioctl_FE_SET_TONE(SEC_TONE_OFF);
	}
	ioctl_FE_DISEQC_SEND_MASTER_CMD(diseqc_cmd);

	if (!(status & TUNER_TUNED) && !(status & TUNER_SCAN)) {
		closefd();
	}
}

void dvbtune::setup_switch()
{
	struct dvb_diseqc_master_cmd committed_switch_cmds[] = {
		{ { 0xE0, 0x10, 0x38, 0xF0, 0x00, 0x00 }, 4 },
		{ { 0xE0, 0x10, 0x38, 0xF4, 0x00, 0x00 }, 4 },
		{ { 0xE0, 0x10, 0x38, 0xF8, 0x00, 0x00 }, 4 },
		{ { 0xE0, 0x10, 0x38, 0xFC, 0x00, 0x00 }, 4 }
	};

	struct dvb_diseqc_master_cmd uncommitted_switch_cmds[] = {
		{ { 0xE0, 0x10, 0x39, 0xF0, 0x00, 0x00 }, 4 },
		{ { 0xE0, 0x10, 0x39, 0xF1, 0x00, 0x00 }, 4 },
		{ { 0xE0, 0x10, 0x39, 0xF2, 0x00, 0x00 }, 4 },
		{ { 0xE0, 0x10, 0x39, 0xF3, 0x00, 0x00 }, 4 },
		{ { 0xE0, 0x10, 0x39, 0xF4, 0x00, 0x00 }, 4 },
		{ { 0xE0, 0x10, 0x39, 0xF5, 0x00, 0x00 }, 4 },
		{ { 0xE0, 0x10, 0x39, 0xF6, 0x00, 0x00 }, 4 },
		{ { 0xE0, 0x10, 0x39, 0xF7, 0x00, 0x00 }, 4 }
	};

	if (myswitch.voltage != tp.voltage) {
		ioctl_FE_SET_VOLTAGE(tp.voltage);
	}

	if (myswitch.uncommitted != tune_ops.uncommitted && tune_ops.uncommitted > 0) {
		if (myswitch.tone == (int)!SEC_TONE_ON) { // SEC_TONE_ON == 0
			myswitch.tone = -1;
			ioctl_FE_SET_TONE(SEC_TONE_OFF);
		}
		ioctl_FE_DISEQC_SEND_MASTER_CMD(uncommitted_switch_cmds[tune_ops.uncommitted-1]);
	}

	if (myswitch.committed != tune_ops.committed && tune_ops.committed > 0) {
		if (myswitch.tone == (int)!SEC_TONE_ON) { // SEC_TONE_ON == 0
			myswitch.tone = -1;
			ioctl_FE_SET_TONE(SEC_TONE_OFF);
		}
		ioctl_FE_DISEQC_SEND_MASTER_CMD(committed_switch_cmds[tune_ops.committed-1]);
	}

	if (myswitch.tone != tune_ops.tone) {
		ioctl_FE_SET_TONE(!tune_ops.tone);
	}

	myswitch.voltage	= tp.voltage;
	myswitch.tone		= tune_ops.tone;
	myswitch.committed	= tune_ops.committed;
	myswitch.uncommitted	= tune_ops.uncommitted;
}

void dvbtune::check_frontend()
{
	fe_status_t festatus;
	ioctl_FE_READ_STATUS(&festatus);
	tp.status = festatus;

	struct dtv_property p[13];
	p[0].cmd = DTV_FREQUENCY;
	p[1].cmd = DTV_DELIVERY_SYSTEM;
	p[2].cmd = DTV_SYMBOL_RATE;
	p[3].cmd = DTV_MODULATION;
	p[4].cmd = DTV_INNER_FEC;
	p[5].cmd = DTV_INVERSION;
	p[6].cmd = DTV_ROLLOFF;
	p[7].cmd = DTV_PILOT;
	p[8].cmd = DTV_MATYPE;
	p[9].cmd = DTV_STAT_SIGNAL_STRENGTH;
	p[10].cmd = DTV_STAT_CNR;
	p[11].cmd = DTV_STAT_POST_ERROR_BIT_COUNT;
	p[12].cmd = DTV_FRAME_LEN;

	struct dtv_properties p_status;
	p_status.num = 13;
	p_status.props = p;

	// get the actual parameters from the driver for that channel
	ioctl_FE_GET_PROPERTY(&p_status);

	tp.frequency	= (int)p_status.props[0].u.data / 1000;
	tp.frequency	= abs(tp.frequency + tune_ops.f_lof);	
	tp.system	= p_status.props[1].u.data;
	tp.symbolrate	= p_status.props[2].u.data/1000;
	tp.modulation	= p_status.props[3].u.data;
	tp.fec		= p_status.props[4].u.data;
	tp.inversion	= p_status.props[5].u.data;
	tp.rolloff	= p_status.props[6].u.data;
	tp.pilot	= p_status.props[7].u.data;
	tp.matype	= p_status.props[8].u.data;
	tp.lvl_scale	= p_status.props[9].u.st.stat[0].scale;
	if (tp.lvl_scale == FE_SCALE_DECIBEL) {
		tp.lvl	= p_status.props[9].u.st.stat[0].svalue * 0.001;
	} else {
		int lvl = 0;
		if (ioctl_FE_READ_SIGNAL_STRENGTH(&lvl)) {
			tp.lvl = (lvl * 100) / 0xffff;
			if (tp.lvl < 0) {
				tp.lvl = 0;
			}
		} else {
			tp.lvl = 0;
		}
	}
	tp.snr_scale = p_status.props[10].u.st.stat[0].scale;
	if (tp.snr_scale == FE_SCALE_DECIBEL) {
		tp.snr = p_status.props[10].u.st.stat[0].svalue * 0.001;
	} else {
		unsigned int snr = 0;
		if (ioctl_FE_READ_SNR(&snr)) {
			if (isATSC(tp.system) || isQAM(tp.system) || isSatellite(tp.system)) {
				tp.snr_scale = FE_SCALE_DECIBEL;
				tp.snr = snr/10.0;
			}
		} else {
			tp.snr = 0;
		}
	}
	tp.ber_scale	= p_status.props[11].u.st.stat[0].scale;
	if (p_status.props[11].u.st.stat[0].scale == FE_SCALE_COUNTER) {
		tp.ber = 0;
		ioctl_FE_READ_BER(&tp.ber); // use dvbv3 ioctl for ber 
		//tp.ber	= p_status.props[11].u.st.stat[0].uvalue; //TODO fix dvbv5 ber across devices
	} else {
		tp.ber = 0;
		ioctl_FE_READ_BER(&tp.ber);
	}
	tp.frame_len = p_status.props[12].u.data;

	unsetbit(TUNER_IOCTL);

	emit update_signal();
}

int dvbtune::closest_freq(int freq, int system)
{
	freq_list myfreq;

	if (isQAM(system)) {
		myfreq.qam();
	} else if (isATSC(system)) {
		myfreq.atsc();
	} else if (isDVBT(system)) {
		myfreq.dvbt();
	}
	int ret_freq = freq;
	if (freq < myfreq.freq.at(0)) {
		ret_freq = myfreq.freq.at(0);
	} else if (freq > myfreq.freq.at(myfreq.freq.size()-1)) {
		ret_freq = myfreq.freq.at(myfreq.freq.size()-1);
	} else {
		for(int c = 0; c < myfreq.freq.size()-1; c++) {
			if (freq > myfreq.freq.at(c) && freq < myfreq.freq.at(c+1)) {
				int middle = (myfreq.freq.at(c) + myfreq.freq.at(c+1))/2;
				if (freq < middle) {
					ret_freq = myfreq.freq.at(c);
				} else {
					ret_freq = myfreq.freq.at(c+1);
				}
			}
		}
	}
	return ret_freq;
}

QString dvbtune::format_freq(int frequency, int system)
{
	QString ret_string = QString::number(frequency);
	freq_list myfreq;

	if (isATSC(system)) {
		myfreq.atsc();
	}
	if (isQAM(system)) {
		myfreq.qam();
	}
	if (isDVBT(system)) {
		myfreq.dvbt();
	}
	if (myfreq.freq.contains(frequency)) {
		ret_string = QString::number(frequency/1000) + "mhz, ch " + QString::number(myfreq.ch.at(myfreq.freq.indexOf(frequency)));
	}
	return ret_string;
}

int dvbtune::tune_clear()
{
	pids_rate.clear();
	pids_rate.fill(0, 0x2000+1);

	dvbdata.clear();
	packet_processed.clear();

	struct dtv_property p_clear[1];
	p_clear[0].cmd = DTV_CLEAR;

	struct dtv_properties cmdseq_clear;
	cmdseq_clear.num     = 1;
	cmdseq_clear.props   = p_clear;

	ioctl_FE_SET_PROPERTY(&cmdseq_clear);

	return 1;
}

int dvbtune::tune()
{
	close_demux();
	close_dvr();

	iq_x.clear();
	iq_y.clear();

	if (isSatellite(tp.system)) {
		setup_switch();	
	}

	tune_clear();

	setbit(TUNER_TUNED);

	int i = 0;
	struct dtv_property p_tune[14];

	if (isSatellite(tp.system)) {
		p_tune[i].cmd = DTV_FREQUENCY;		p_tune[i++].u.data = abs(tp.frequency - abs(tune_ops.f_lof)) * 1000;
		p_tune[i].cmd = DTV_DELIVERY_SYSTEM;	p_tune[i++].u.data = tp.system;
		p_tune[i].cmd = DTV_MODULATION;		p_tune[i++].u.data = tp.modulation;
		p_tune[i].cmd = DTV_VOLTAGE;		p_tune[i++].u.data = tp.voltage;
		p_tune[i].cmd = DTV_SYMBOL_RATE;	p_tune[i++].u.data = tp.symbolrate * 1000;
		p_tune[i].cmd = DTV_TONE;		p_tune[i++].u.data = !tune_ops.tone;
		p_tune[i].cmd = DTV_INNER_FEC;		p_tune[i++].u.data = tp.fec;
		p_tune[i].cmd = DTV_INVERSION;		p_tune[i++].u.data = tp.inversion;
		p_tune[i].cmd = DTV_ROLLOFF;		p_tune[i++].u.data = tp.rolloff;
		p_tune[i].cmd = DTV_BANDWIDTH_HZ;	p_tune[i++].u.data = 0;
		p_tune[i].cmd = DTV_PILOT;		p_tune[i++].u.data = tp.pilot;
		p_tune[i].cmd = DTV_ENABLE_MODCOD;	p_tune[i++].u.data = tune_ops.modcod;
		p_tune[i].cmd = DTV_DVBS2_MIS_ID;	p_tune[i++].u.data = tune_ops.mis;
		qDebug() << "tune() Frequency: " << tp.frequency << dvbnames.voltage[tp.voltage] << tp.symbolrate;
	} else if (isQAM(tp.system) || isATSC(tp.system)) {
		tp.frequency = closest_freq(tp.frequency, tp.system);
		p_tune[i].cmd = DTV_FREQUENCY;		p_tune[i++].u.data = tp.frequency * 1000;
		p_tune[i].cmd = DTV_DELIVERY_SYSTEM;	p_tune[i++].u.data = tp.system;
		p_tune[i].cmd = DTV_MODULATION;		p_tune[i++].u.data = tp.modulation;
		qDebug() << "tune() Frequency: " << tp.frequency;
	} else if (isDVBT(tp.system)) {
		tp.frequency = closest_freq(tp.frequency, tp.system);
		p_tune[i].cmd = DTV_FREQUENCY;		p_tune[i++].u.data = tp.frequency * 1000;
		p_tune[i].cmd = DTV_DELIVERY_SYSTEM;	p_tune[i++].u.data = tp.system;
		p_tune[i].cmd = DTV_MODULATION;		p_tune[i++].u.data = tp.modulation;
		p_tune[i].cmd = DTV_INVERSION;		p_tune[i++].u.data = INVERSION_AUTO;
		p_tune[i].cmd = DTV_BANDWIDTH_HZ;	p_tune[i++].u.data = 8000000;
		p_tune[i].cmd = DTV_CODE_RATE_HP;	p_tune[i++].u.data = FEC_AUTO;
		p_tune[i].cmd = DTV_CODE_RATE_LP;	p_tune[i++].u.data = FEC_AUTO;
		p_tune[i].cmd = DTV_GUARD_INTERVAL;	p_tune[i++].u.data = GUARD_INTERVAL_AUTO;
		p_tune[i].cmd = DTV_TRANSMISSION_MODE;	p_tune[i++].u.data = TRANSMISSION_MODE_AUTO;
		p_tune[i].cmd = DTV_HIERARCHY;		p_tune[i++].u.data = HIERARCHY_AUTO;
		qDebug() << "tune() Frequency: " << tp.frequency;
	} else {
		qDebug() << "Invalid system";
	}

	p_tune[i++].cmd = DTV_TUNE;
	
	struct dtv_properties cmdseq_tune;
	cmdseq_tune.num     = i;
	cmdseq_tune.props   = p_tune;

	ioctl_FE_SET_PROPERTY(&cmdseq_tune);

	// Keep trying for up to 5 seconds
	QTime t;
	t.start();
	while (t.elapsed() < 5000) {
		ioctl_FE_READ_STATUS(&festatus);

		if (festatus & FE_TIMEDOUT) {
			emit update_status("", STATUS_CLEAR);
			emit update_status("Tuning Timedout, searched for: " + QString::number(t.elapsed()/1000.0, 'f', 1) + "s", 2);
			check_frontend();
			return -1;
		}
		
		if (festatus & FE_HAS_LOCK) {
			emit update_status("", STATUS_CLEAR);
			emit update_status("Tuning Locked, searched for: " + QString::number(t.elapsed()/1000.0, 'f', 1) + "s", 1);
			qDebug() << "Locked...";
			check_frontend();
			emit update_results();
			return 1;	
		} else {
			qDebug() << "No Lock...";
			msleep(200);		
		}
	}
	emit update_status("", STATUS_CLEAR);
	emit update_status("Tuning Failed, searched for: " + QString::number(t.elapsed()/1000.0, 'f', 1) + "s", 2);
	check_frontend();
	return -1;
}

void dvbtune::get_bitrate()
{
	if (!(festatus & FE_HAS_LOCK) || !demux_packets_loop) {
		return;
	}

	while (status & TUNER_RDING) {
		msleep(10);
	}

	QTime stime;
	int ttime = 0;

	QVector<unsigned int> new_rate;
	new_rate.clear();
	new_rate.fill(0, 0x2000+1);

	if (pids_rate.isEmpty()) {
		pids_rate.clear();
		pids_rate.fill(0, 0x2000+1);
	}

	int len = 0;
	char buf[LIL_BUFSIZE];
	buffer.clear();

	pids.clear();
	pids.append(0x2000);
	demux_video();

	if (!open_dvr()) {
		return;
	}

	struct pollfd pfd[1];
	pfd[0].fd = dvr_fd;
	pfd[0].events = POLLIN;

	setbit(TUNER_RDING);
	stime.start();
	for (unsigned long i = 0; i < BIG_BUFSIZE && demux_packets_loop; i += len) {
		if (poll(pfd, 1, 1000)) {
			len = 0;
			memset(buf, 0, LIL_BUFSIZE);
			if (pfd[0].revents & POLLIN) {
				len = read(dvr_fd, buf, LIL_BUFSIZE);
				if (len > 0) {
					buffer.append(buf, len);
				} else {
					// Even if read failed we do eventually want to exit this loop
					len = LIL_BUFSIZE;
				}
			}
		}
	}
	ttime = stime.elapsed();
	unsetbit(TUNER_RDING);

	if (buffer.size() < BIG_BUFSIZE) {
		qDebug() << Q_FUNC_INFO << "Buffer size mismatch:" << buffer.size() << "vs" << BIG_BUFSIZE;
		return;
	}
		
	signed long i = 0;
	signed int p = 0;
	while (i < buffer.size()) {
		if (buffer.at(i) != 0x47) {
			qDebug() << "desync";
			while (i < buffer.size() && buffer.at(i) != 0x47) {
				i++;
			}
		}
		if (i >= buffer.size()) {
			continue;
		}
		p = (((unsigned char)buffer.at(i+1) << 8) | (unsigned char)buffer.at(i+2)) & 0x00001FFF;
		new_rate[p]++;
		new_rate[0x2000]++;
		i += 188;
	}
	
	for (i = 0; i <= 0x2000; i++) {
		switch(pids_rate[i]) {
		case 0:
			pids_rate[i] = (new_rate[i]*188*8)/ttime;
			break;
		case 1:
			// If there were packets before, lets just show 1 vs not showing it on the list at all
			if ((new_rate[i]*188*8)/ttime > 1) {
				pids_rate[i] = (new_rate[i]*188*8)/ttime;
			}
			break;
		default:
			pids_rate[i] += (new_rate[i]*188*8)/ttime;
			pids_rate[i] /= 2;
			break;
		}
	}
}

int dvbtune::demux_packets(QVector<dvb_pids> mypids)
{
	if (!(festatus & FE_HAS_LOCK)) {
		return -1;
	}

	demux_packets_loop = true;

	stop_demux();

	char buf[TNY_BUFSIZE];

	setbit(TUNER_RDING);
	for(int a = 0; a < mypids.size() && demux_packets_loop; a++)
	{
		struct dmx_sct_filter_params sctfilter;
		memset(&sctfilter, 0, sizeof(struct dmx_sct_filter_params));
		sctfilter.pid = mypids.at(a).pid;
		for (int i = 0; i < mypids.at(a).tbl.size(); i++) {
			sctfilter.filter.filter[i]	= (unsigned int)mypids.at(a).tbl.at(i);
			sctfilter.filter.mask[i]	= (unsigned int)mypids.at(a).msk.at(i);
		}
		sctfilter.timeout = mypids.at(a).timeout;
		sctfilter.flags = DMX_IMMEDIATE_START | DMX_CHECK_CRC | DMX_ONESHOT;
		ioctl_DMX_SET_FILTER(&sctfilter);

		memset(buf, 0, TNY_BUFSIZE);
		int len = read(dmx_fd.first(), buf, TNY_BUFSIZE);
		if (len > 0) {
			buffer.clear();
			buffer.append(buf, len);
			dvb_data tmp;
			tmp.pid		= mypids.at(a).pid;
			tmp.table	= mypids.at(a).tbl;
			tmp.mask	= mypids.at(a).msk;
			tmp.buffer	= buffer;
			dvbdata.append(tmp);
			// We start at 1000 to find a packet, then goto 300 after to speed things up
			mypids[a].timeout = 300;
		}
	}
	unsetbit(TUNER_RDING);

	stop_demux();

	return 1;
}

void dvbtune::set_data_format(fe_data_format format)
{
	if (data_format != format) {
		ioctl_FE_SET_DFMT(format);
	}
}

void dvbtune::demux_bbframe()
{
	set_data_format(FE_DFMT_BB_FRAME);

	stop_demux();

	struct dmx_bb_filter_params bbFilterParams;
	memset(&bbFilterParams, 0, sizeof (struct dmx_bb_filter_params));
	bbFilterParams.isi	= DMX_ISI_ALL;
	bbFilterParams.input	= DMX_IN_FRONTEND;
	bbFilterParams.output	= DMX_OUT_TS_TAP;
	bbFilterParams.type	= DMX_BB_FRAME;
	bbFilterParams.flags	= DMX_IMMEDIATE_START;
	ioctl_DMX_SET_BB_FILTER(&bbFilterParams);
	emit adapter_status(adapter);
}

void dvbtune::demux_video()
{
	set_data_format(FE_DFMT_TS_PACKET);

	stop_demux();

	struct dmx_pes_filter_params pesFilterParams;
	memset(&pesFilterParams, 0, sizeof(pesFilterParams));
	for(int a = 0; a < pids.size(); a++)
	{
		pesFilterParams.pid = pids[a];
		pesFilterParams.input		= DMX_IN_FRONTEND;
		pesFilterParams.output		= DMX_OUT_TS_TAP;
		pesFilterParams.pes_type	= DMX_PES_OTHER;
		pesFilterParams.flags		= DMX_IMMEDIATE_START;
		ioctl_DMX_SET_PES_FILTER(a, &pesFilterParams);
	}
	emit adapter_status(adapter);
}

void dvbtune::demux_stream(bool start)
{
	if (start) {
		demux_video();

		mydvr->thread_function.append("demux_stream");
		mydvr->start();
	} else {
		if (mydvr->thread_function.contains("demux_stream")) {
			mydvr->thread_function.remove(mydvr->thread_function.indexOf("demux_stream"));
			while (status & TUNER_RDING) {
				msleep(10);
			}
		}
	}
	emit adapter_status(adapter);
}

void dvbtune::demux_file(bool start)
{
	if (start) {
		if (data_format == FE_DFMT_TS_PACKET) {
			demux_video();
		} else {
			demux_bbframe();
		}

		mydvr->file_name = out_name;
		mydvr->thread_function.append("demux_file");
		mydvr->start();
	} else {
		if (mydvr->thread_function.contains("demux_file")) {
			mydvr->thread_function.remove(mydvr->thread_function.indexOf("demux_file"));
			while (status & TUNER_RDING) {
				msleep(10);
			}
			mydvr->close_file();
		}
	}
	emit adapter_status(adapter);
}

int dvbtune::crc32()
{
	// CRC32 lookup table for polynomial 0x04c11db7
	static u_long crc_table[256] = {
	        0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
	        0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
	        0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
	        0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
	        0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
	        0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
	        0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
	        0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
	        0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
	        0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
	        0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
	        0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
	        0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
	        0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
	        0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
	        0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
	        0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
	        0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
	        0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
	        0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
	        0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
	        0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
	        0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
	        0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
	        0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
	        0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
	        0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
	        0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
	        0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
	        0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
	        0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
	        0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
	        0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
	        0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
	        0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
	        0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
	        0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
	        0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
	        0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
	        0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
	        0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
	        0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
	        0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
	};

	int len = buffer.size();
	unsigned int crc_buffer = 0;

	crc_buffer += (unsigned char)buffer.at(len-4) << 24;
	crc_buffer += (unsigned char)buffer.at(len-3) << 16;
	crc_buffer += (unsigned char)buffer.at(len-2) << 8;
	crc_buffer += (unsigned char)buffer.at(len-1) << 0;

	quint32 crc_calc = 0xffffffff;
	for (int i = 0; i < len-4; i++) {
		crc_calc = (crc_calc << 8) ^ crc_table[((crc_calc >> 24) ^ (unsigned char)buffer.at(i)) & 0xff];
	}

	if (crc_buffer == crc_calc) {
		return 1;
	} else {
		qDebug() << "CRC mismatch: " << hex << crc_buffer << ":" << hex << crc_calc;
		return 0;
	}
}

bool dvbtune::open_dvr()
{
	if (dvr_name.isEmpty()) {
		while(status & TUNER_RDING || status & TUNER_IOCTL) {
			msleep(10);
		}
		dvr_name = "/dev/dvb/adapter" + QString::number(adapter) + "/dvr0";
		dvr_fd = open(dvr_name.toStdString().c_str(), O_RDONLY);
		if (dvr_fd < 0) {
			qDebug() << "Failed to open" << dvr_name << "-" << strerror(errno);
			dvr_fd = -1;
			dvr_name.clear();
			return false;
		}
	}
	return true;
}

void dvbtune::close_dvr()
{
	close_demux();

	if (!dvr_name.isEmpty()) {
		while(status & TUNER_RDING || status & TUNER_IOCTL) {
			msleep(10);
		}
		close(dvr_fd);
		dvr_fd = -1;
		dvr_name.clear();
	}
	emit adapter_status(adapter);
}

bool dvbtune::open_demux()
{
	if (dmx_name.isEmpty()) {
		dmx_name = "/dev/dvb/adapter" + QString::number(adapter) + "/demux0";
	}
	int dmx_size = pids.size() ? pids.size() : 1; // We need at least one so the sct filter has something
	while (dmx_fd.size() < dmx_size) {
		while(status & TUNER_RDING || status & TUNER_IOCTL) {
			msleep(10);
		}
		int tmp_fd = open(dmx_name.toStdString().c_str(), O_RDWR);
		if (tmp_fd < 0) {
			qDebug() << "Failed to open" << dmx_name << "-" << strerror(errno);
			return false;
		} else {
			dmx_fd.append(tmp_fd);
			ioctl_DMX_SET_BUFFER_SIZE(BIG_BUFSIZE);
		}
	}
	return true;
}

void dvbtune::close_demux()
{
	stop_demux();
	if (!dmx_name.isEmpty()) {
		while(status & TUNER_RDING || status & TUNER_IOCTL) {
			msleep(10);
		}
		while (!dmx_fd.isEmpty()) {
			close(dmx_fd.last());
			dmx_fd.removeLast();
		}
		dmx_name.clear();
	}
}

void dvbtune::stop_demux()
{
	ioctl_DMX_STOP();

	emit adapter_status(adapter);
}

void dvbtune::spectrum_scan(dvb_fe_spectrum_scan *scan)
{
	if (isSatellite(tp.system)) {
		setup_switch();	
	}

	ioctl_FE_GET_SPECTRUM_SCAN(scan);
}

void dvbtune::iqplot()
{
	struct dvb_fe_constellation_samples const_samples;
	struct dvb_fe_constellation_sample samples[DTV_MAX_CONSTELLATION_SAMPLES];
	const_samples.num = DTV_MAX_CONSTELLATION_SAMPLES;
	const_samples.samples = samples;
	const_samples.options = iq_options;
	ioctl_FE_GET_CONSTELLATION_SAMPLES(&const_samples);

	for (unsigned int i = 0 ; i < const_samples.num ; i++) {
		while (iq_x.size() >= (int)const_samples.num * PERSISTENCE) {
			iq_x.erase(iq_x.begin());
			iq_y.erase(iq_y.begin());
		}
		iq_x.append(samples[i].imaginary);
		iq_y.append(samples[i].real);
	}
	emit iqdraw(iq_x, iq_y);
}

QString dvbtune::min_snr()
{
	QString snr;

	switch (tp.system) {
	case SYS_ATSC:
	case SYS_ATSCMH:
		snr = "15.2";
		break;
	case SYS_DVBT:
		switch (tp.modulation) {
		case QPSK:
			snr = "14.0";
			break;
		case QAM_16:
			snr = "20.0";
			break;
		default:
			break;
		}
		break;
	case SYS_DVBT2:
		switch (tp.modulation) {
		case QAM_16:
			snr = "12.3";
			break;
		case QAM_64:
			snr = "17.1";
			break;
		case QAM_256:
			snr = "18.9";
			break;
		default:
			break;
		}
		break;
	case SYS_DVBC_ANNEX_C:
		switch (tp.modulation) {
		case QAM_64:
			snr = "30.0";
			break;
		case QAM_256:
			snr = "34.0";
			break;
		default:
			break;
		}
		break;
	case SYS_DVBC_ANNEX_B:
		switch (tp.modulation) {
		case QAM_16:
			snr = "18.0";
			break;
		case QAM_64:
			snr = "24.0";
			break;
		case QAM_256:
			snr = "30.0";
			break;
		default:
			break;
		}
		break;
	case SYS_DVBS:
		switch (tp.fec) {
		case FEC_1_2:
			snr = "2.7";
			break;
		case FEC_2_3:
			snr = "4.4";
			break;
		case FEC_3_4:
			snr = "5.5";
			break;
		case FEC_5_6:
			snr = "6.5";
			break;
		case FEC_7_8:
			snr = "7.2";
			break;
		default:
			break;
		}
		break;
	case SYS_DVBS2:
		switch (tp.modulation) {
		case QPSK:
			switch (tp.fec) {
			case FEC_3_5:
				snr = "2.2";
				break;
			case FEC_3_4:
				snr = "4.0";
				break;
			case FEC_5_6:
				snr = "5.2";
				break;
			case FEC_8_9:
				snr = "6.2";
				break;
			case FEC_9_10:
				snr = "6.4";
				break;
			default:
				break;
			}
			break;
		case PSK_8:
			switch (tp.fec) {
			case FEC_3_5:
				snr = "5.5";
				break;
			case FEC_2_3:
				snr = "6.6";
				break;
			case FEC_3_4:
				snr = "7.9";
				break;
			case FEC_5_6:
				snr = "9.4";
				break;
			case FEC_8_9:
				snr = "10.7";
				break;
			case FEC_9_10:
				snr = "11.0";
				break;
			default:
				break;
			}
			break;
		case APSK_16:
			switch (tp.fec) {
			case FEC_2_3:
				snr = "9.0";
				break;
			case FEC_3_4:
				snr = "10.2";
				break;
			case FEC_4_5:
				snr = "11.0";
				break;
			case FEC_5_6:
				snr = "11.6";
				break;
			case FEC_8_9:
				snr = "12.9";
				break;
			case FEC_9_10:
				snr = "12.1";
				break;
			default:
				break;
			}
			break;
		case APSK_32:
			switch (tp.fec) {
			case FEC_3_4:
				snr = "14.8";
				break;
			case FEC_4_5:
				snr = "15.7";
				break;
			case FEC_5_6:
				snr = "16.3";
				break;
			case FEC_8_9:
				snr = "17.7";
				break;
			case FEC_9_10:
				snr = "18.1";
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return snr;
}

void dvbtune::run()
{
	QTime check_frontend_t;
	check_frontend_t.start();
	QTime iqplot_t;
	iqplot_t.start();
	loop = true;
	do {
		if (thread_function.contains("iqplot")) {
			if (iqplot_t.elapsed() > 500) {
				iqplot();
				iqplot_t.restart();
			} else {
				msleep(100);
			}
		}
		if (thread_function.contains("check_frontend")) {
			if (check_frontend_t.elapsed() > 2000) {
				check_frontend();
				check_frontend_t.restart();
			} else {
				msleep(100);
			}
		}
		if (thread_function.contains("tune")) {
			tune();
			thread_function.append("check_frontend");
			thread_function.remove(thread_function.indexOf("tune"));			
		}
		if (thread_function.isEmpty()) {
			msleep(100);
		}
	} while(loop);
	thread_function.clear();
	tune_clear();
}

void dvbtune::setbit(unsigned int MASK)
{
	status = status | MASK;
}
void dvbtune::unsetbit(unsigned int MASK)
{
	status = status & ~MASK;
}

bool dvbtune::ioctl_FE_SET_TONE(bool tone)
{
	if (!openfd()) {
		return false;
	}
	while (status & TUNER_IOCTL) {
		setbit(TUNER_IOCTL_QUEUE);
		msleep(10);
	}
	unsetbit(TUNER_IOCTL_QUEUE);

	setbit(TUNER_IOCTL);
	if (ioctl(frontend_fd, FE_SET_TONE, tone) < 0) {
		qDebug() << Q_FUNC_INFO << "ERROR! device:" << frontend_name;
		unsetbit(TUNER_IOCTL);
		return false;
	}
	msleep(20);
	unsetbit(TUNER_IOCTL);

	while (status & TUNER_IOCTL_QUEUE) { // Give other functions a chance to send ioctl's
		msleep(100);
	}

	return true;
}

bool dvbtune::ioctl_FE_DISEQC_SEND_MASTER_CMD(dvb_diseqc_master_cmd diseqc_cmd)
{
	if (!openfd()) {
		return false;
	}
	while (status & TUNER_IOCTL) {
		setbit(TUNER_IOCTL_QUEUE);
		msleep(10);
	}
	unsetbit(TUNER_IOCTL_QUEUE);

	setbit(TUNER_IOCTL);
	if (ioctl(frontend_fd, FE_DISEQC_SEND_MASTER_CMD, &diseqc_cmd) < 0) {
		qDebug() << Q_FUNC_INFO << "ERROR! device:" << frontend_name;
		unsetbit(TUNER_IOCTL);
		return false;
	}
	msleep(20);
	unsetbit(TUNER_IOCTL);

	while (status & TUNER_IOCTL_QUEUE) { // Give other functions a chance to send ioctl's
		msleep(100);
	}

	return true;
}

bool dvbtune::ioctl_FE_SET_VOLTAGE(int voltage)
{
	if (!openfd()) {
		return false;
	}
	while (status & TUNER_IOCTL) {
		setbit(TUNER_IOCTL_QUEUE);
		msleep(10);
	}
	unsetbit(TUNER_IOCTL_QUEUE);

	setbit(TUNER_IOCTL);
	if (ioctl(frontend_fd, FE_SET_VOLTAGE, voltage) < 0) {
		qDebug() << Q_FUNC_INFO << "ERROR! device:" << frontend_name;
		unsetbit(TUNER_IOCTL);
		return false;
	}
	servo ? msleep(400) : msleep(20);
	unsetbit(TUNER_IOCTL);

	while (status & TUNER_IOCTL_QUEUE) { // Give other functions a chance to send ioctl's
		msleep(100);
	}

	return true;
}

bool dvbtune::ioctl_FE_READ_STATUS(fe_status_t *fe_status)
{
	if (!openfd()) {
		return false;
	}
	while (status & TUNER_IOCTL) {
		setbit(TUNER_IOCTL_QUEUE);
		msleep(10);
	}
	unsetbit(TUNER_IOCTL_QUEUE);

	setbit(TUNER_IOCTL);
	if (ioctl(frontend_fd, FE_READ_STATUS, fe_status) < 0) {
		qDebug() << Q_FUNC_INFO << "ERROR! device:" << frontend_name;
		unsetbit(TUNER_IOCTL);
		return false;
	}
	unsetbit(TUNER_IOCTL);

	while (status & TUNER_IOCTL_QUEUE) { // Give other functions a chance to send ioctl's
		msleep(100);
	}

	return true;
}

bool dvbtune::ioctl_FE_SET_PROPERTY(dtv_properties *p_status)
{
	if (!openfd()) {
		return false;
	}
	while (status & TUNER_IOCTL) {
		setbit(TUNER_IOCTL_QUEUE);
		msleep(10);
	}
	unsetbit(TUNER_IOCTL_QUEUE);

	setbit(TUNER_IOCTL);
	if (ioctl(frontend_fd, FE_SET_PROPERTY, p_status) < 0) {
		qDebug() << Q_FUNC_INFO << "ERROR! device:" << frontend_name;
		unsetbit(TUNER_IOCTL);
		return false;
	}
	unsetbit(TUNER_IOCTL);

	while (status & TUNER_IOCTL_QUEUE) { // Give other functions a chance to send ioctl's
		msleep(100);
	}

	return true;
}

bool dvbtune::ioctl_FE_GET_PROPERTY(dtv_properties *p_status)
{
	if (!openfd()) {
		return false;
	}
	while (status & TUNER_IOCTL) {
		setbit(TUNER_IOCTL_QUEUE);
		msleep(10);
	}
	unsetbit(TUNER_IOCTL_QUEUE);

	setbit(TUNER_IOCTL);
	if (ioctl(frontend_fd, FE_GET_PROPERTY, p_status) < 0) {
		qDebug() << Q_FUNC_INFO << "ERROR! device:" << frontend_name;
		unsetbit(TUNER_IOCTL);
		return false;
	}
	unsetbit(TUNER_IOCTL);

	while (status & TUNER_IOCTL_QUEUE) { // Give other functions a chance to send ioctl's
		msleep(100);
	}

	return true;
}

bool dvbtune::ioctl_FE_GET_INFO(dvb_frontend_info *fe_info)
{
	if (!openfd()) {
		return false;
	}
	while (status & TUNER_IOCTL) {
		setbit(TUNER_IOCTL_QUEUE);
		msleep(10);
	}
	unsetbit(TUNER_IOCTL_QUEUE);

	setbit(TUNER_IOCTL);
	if (ioctl(frontend_fd, FE_GET_INFO, fe_info) < 0) {
		qDebug() << Q_FUNC_INFO << "ERROR! device:" << frontend_name;
		unsetbit(TUNER_IOCTL);
		return false;
	}
	unsetbit(TUNER_IOCTL);

	while (status & TUNER_IOCTL_QUEUE) { // Give other functions a chance to send ioctl's
		msleep(100);
	}

	return true;
}

bool dvbtune::ioctl_FE_GET_EXTENDED_INFO(dvb_frontend_extended_info *fe_extended_info)
{
	if (!openfd()) {
		return false;
	}
	while (status & TUNER_IOCTL) {
		setbit(TUNER_IOCTL_QUEUE);
		msleep(10);
	}
	unsetbit(TUNER_IOCTL_QUEUE);

	setbit(TUNER_IOCTL);
	if (ioctl(frontend_fd, FE_GET_EXTENDED_INFO, fe_extended_info) < 0) {
		qDebug() << Q_FUNC_INFO << "ERROR! device:" << frontend_name;
		unsetbit(TUNER_IOCTL);
		return false;
	}
	unsetbit(TUNER_IOCTL);

	while (status & TUNER_IOCTL_QUEUE) { // Give other functions a chance to send ioctl's
		msleep(100);
	}

	return true;
}

bool dvbtune::ioctl_FE_READ_SIGNAL_STRENGTH(int *lvl)
{
	if (!openfd()) {
		return false;
	}
	while (status & TUNER_IOCTL) {
		setbit(TUNER_IOCTL_QUEUE);
		msleep(10);
	}
	unsetbit(TUNER_IOCTL_QUEUE);

	setbit(TUNER_IOCTL);
	if (ioctl(frontend_fd, FE_READ_SIGNAL_STRENGTH, lvl) < 0) {
		qDebug() << Q_FUNC_INFO << "ERROR! device:" << frontend_name;
		unsetbit(TUNER_IOCTL);
		return false;
	}
	unsetbit(TUNER_IOCTL);

	while (status & TUNER_IOCTL_QUEUE) { // Give other functions a chance to send ioctl's
		msleep(100);
	}

	return true;
}

bool dvbtune::ioctl_FE_READ_SNR(unsigned int *snr)
{
	if (!openfd()) {
		return false;
	}
	while (status & TUNER_IOCTL) {
		setbit(TUNER_IOCTL_QUEUE);
		msleep(10);
	}
	unsetbit(TUNER_IOCTL_QUEUE);

	setbit(TUNER_IOCTL);
	if (ioctl(frontend_fd, FE_READ_SNR, snr) < 0) {
		qDebug() << Q_FUNC_INFO << "ERROR! device:" << frontend_name;
		unsetbit(TUNER_IOCTL);
		return false;
	}
	unsetbit(TUNER_IOCTL);

	while (status & TUNER_IOCTL_QUEUE) { // Give other functions a chance to send ioctl's
		msleep(100);
	}

	return true;
}

bool dvbtune::ioctl_FE_READ_BER(unsigned int *ber)
{
	if (!openfd()) {
		return false;
	}
	while (status & TUNER_IOCTL) {
		setbit(TUNER_IOCTL_QUEUE);
		msleep(10);
	}
	unsetbit(TUNER_IOCTL_QUEUE);

	setbit(TUNER_IOCTL);
	if (ioctl(frontend_fd, FE_READ_BER, ber) < 0) {
		qDebug() << Q_FUNC_INFO << "ERROR! device:" << frontend_name;
		unsetbit(TUNER_IOCTL);
		return false;
	}
	unsetbit(TUNER_IOCTL);

	while (status & TUNER_IOCTL_QUEUE) { // Give other functions a chance to send ioctl's
		msleep(100);
	}

	return true;
}

bool dvbtune::ioctl_FE_GET_SPECTRUM_SCAN(dvb_fe_spectrum_scan *scan)
{
	if (!openfd()) {
		return false;
	}
	while (status & TUNER_IOCTL) {
		setbit(TUNER_IOCTL_QUEUE);
		msleep(10);
	}
	unsetbit(TUNER_IOCTL_QUEUE);

	setbit(TUNER_IOCTL);
	if (ioctl(frontend_fd, FE_GET_SPECTRUM_SCAN, scan) < 0) {
		qDebug() << Q_FUNC_INFO << "ERROR! device:" << frontend_name;
		unsetbit(TUNER_IOCTL);
		return false;
	}
	unsetbit(TUNER_IOCTL);

	while (status & TUNER_IOCTL_QUEUE) { // Give other functions a chance to send ioctl's
		msleep(100);
	}

	return true;
}

bool dvbtune::ioctl_FE_GET_CONSTELLATION_SAMPLES(dvb_fe_constellation_samples *const_samples)
{
	if (!openfd()) {
		return false;
	}
	while (status & TUNER_IOCTL) {
		setbit(TUNER_IOCTL_QUEUE);
		msleep(10);
	}
	unsetbit(TUNER_IOCTL_QUEUE);

	setbit(TUNER_IOCTL);
	if (ioctl(frontend_fd, FE_GET_CONSTELLATION_SAMPLES, const_samples) < 0) {
		qDebug() << Q_FUNC_INFO << "ERROR! device:" << frontend_name;
		unsetbit(TUNER_IOCTL);
		return false;
	}
	unsetbit(TUNER_IOCTL);

	while (status & TUNER_IOCTL_QUEUE) { // Give other functions a chance to send ioctl's
		msleep(100);
	}

	return true;
}

bool dvbtune::ioctl_DMX_SET_BUFFER_SIZE(unsigned int size)
{
	if (!open_demux()) {
		return false;
	}
	while (status & TUNER_IOCTL) {
		setbit(TUNER_IOCTL_QUEUE);
		msleep(10);
	}
	unsetbit(TUNER_IOCTL_QUEUE);

	setbit(TUNER_IOCTL);
	if (ioctl(dmx_fd.last(), DMX_SET_BUFFER_SIZE, size) < 0) {
		qDebug() << Q_FUNC_INFO << "ERROR! device:" << dmx_name;
		unsetbit(TUNER_IOCTL);
		return false;
	}
	unsetbit(TUNER_IOCTL);

	while (status & TUNER_IOCTL_QUEUE) { // Give other functions a chance to send ioctl's
		msleep(100);
	}

	return true;
}

bool dvbtune::ioctl_DMX_SET_FILTER(dmx_sct_filter_params *sctfilter)
{
	if (!open_demux()) {
		return false;
	}
	while (status & TUNER_IOCTL) {
		setbit(TUNER_IOCTL_QUEUE);
		msleep(10);
	}
	unsetbit(TUNER_IOCTL_QUEUE);

	setbit(TUNER_DEMUX);

	setbit(TUNER_IOCTL);
	if (ioctl(dmx_fd.first(), DMX_SET_FILTER, sctfilter) < 0) {
		qDebug() << Q_FUNC_INFO << "ERROR! device:" << dmx_name;
		unsetbit(TUNER_IOCTL);
		return false;
	}
	unsetbit(TUNER_IOCTL);

	while (status & TUNER_IOCTL_QUEUE) { // Give other functions a chance to send ioctl's
		msleep(100);
	}

	return true;
}

bool dvbtune::ioctl_DMX_SET_BB_FILTER(dmx_bb_filter_params *bbFilterParams)
{
	if (!open_demux()) {
		return false;
	}
	while (status & TUNER_IOCTL) {
		setbit(TUNER_IOCTL_QUEUE);
		msleep(10);
	}
	unsetbit(TUNER_IOCTL_QUEUE);

	setbit(TUNER_DEMUX);

	setbit(TUNER_IOCTL);
	if (ioctl(dmx_fd.first(), DMX_SET_BB_FILTER, bbFilterParams) < 0) {
		qDebug() << Q_FUNC_INFO << "ERROR! device:" << dmx_name;
		unsetbit(TUNER_IOCTL);
		return false;
	}
	unsetbit(TUNER_IOCTL);

	while (status & TUNER_IOCTL_QUEUE) { // Give other functions a chance to send ioctl's
		msleep(100);
	}

	return true;
}

bool dvbtune::ioctl_DMX_SET_PES_FILTER(int index, dmx_pes_filter_params *pesFilterParams)
{
	if (!open_demux()) {
		return false;
	}
	while (status & TUNER_IOCTL) {
		setbit(TUNER_IOCTL_QUEUE);
		msleep(10);
	}
	unsetbit(TUNER_IOCTL_QUEUE);

	setbit(TUNER_DEMUX);

	setbit(TUNER_IOCTL);
	if (ioctl(dmx_fd.at(index), DMX_SET_PES_FILTER, pesFilterParams) < 0) {
		qDebug() << Q_FUNC_INFO << "ERROR! device:" << dmx_name;
		unsetbit(TUNER_IOCTL);
		return false;
	}
	unsetbit(TUNER_IOCTL);

	while (status & TUNER_IOCTL_QUEUE) { // Give other functions a chance to send ioctl's
		msleep(100);
	}

	return true;
}

bool dvbtune::ioctl_DMX_STOP()
{
	if (dmx_name.isEmpty()) { // If its not open we dont need to open it just to stop it. Better to just exit
		return false;
	}
	while (status & TUNER_IOCTL || status & TUNER_RDING) {
		setbit(TUNER_IOCTL_QUEUE);
		msleep(10);
	}
	unsetbit(TUNER_IOCTL_QUEUE);

	setbit(TUNER_IOCTL);
	for (int i = 0; i < dmx_fd.size(); i++) {
		if (ioctl(dmx_fd.at(i), DMX_STOP) < 0) {
			qDebug() << Q_FUNC_INFO << "ERROR! device:" << dmx_name;
		}
	}
	unsetbit(TUNER_IOCTL);

	while (status & TUNER_IOCTL_QUEUE) { // Give other functions a chance to send ioctl's
		msleep(100);
	}

	unsetbit(TUNER_DEMUX);

	return true;
}

bool dvbtune::ioctl_FE_SET_DFMT(enum fe_data_format format)
{
	if (!openfd()) {
		return false;
	}
	while (status & TUNER_IOCTL) {
		setbit(TUNER_IOCTL_QUEUE);
		msleep(10);
	}
	unsetbit(TUNER_IOCTL_QUEUE);

	data_format = format;

	setbit(TUNER_IOCTL);
	if (ioctl(frontend_fd, FE_SET_DFMT, format) < 0) {
		qDebug() << Q_FUNC_INFO << "ERROR! device:" << frontend_name;
		unsetbit(TUNER_IOCTL);
		return false;
	}
	unsetbit(TUNER_IOCTL);

	while (status & TUNER_IOCTL_QUEUE) { // Give other functions a chance to send ioctl's
		msleep(100);
	}

	return true;
}
