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
	dvr_name	= "";
	dmx_name	= "";
	sct_name	= "";
	out_name	= "";
	frontend_name	= "";
	ready		= true;
	loop		= false;
	iq_options	= 0x00;
	adapter		= 0;
	frontend	= 0;
	index		= 0;
	frontend_fd	= 0;
	dvr_fd		= 0;
	sct_fd		= 0;
	out_fd		= 0;
	old_position	= 0;
	fmin		= 0;
	fmax		= 0;
	fstep		= 0;
}

dvbtune::~dvbtune()
{
	qDebug() << "~dvbtune()";
	
	stop_demux();
	closefd();
}

unsigned int dvbtune::maskbits(unsigned int value, unsigned int mask)
{
	value = value & mask;
	for(int i = 0; i < 32 && !((mask >> i) & 0x01); i++) {
		value = value >> 1;
	}
	return value;
}

unsigned int dvbtune::read32(unsigned int mask)
{
	if (index+3 >= buffer.size()) {
		return 0;
	}
	unsigned int ret = ((unsigned char)buffer.at(index) << 24) + ((unsigned char)buffer.at(index+1) << 16) + ((unsigned char)buffer.at(index+2) << 8) + (unsigned char)buffer.at(index+3);
	index += 4;
	return maskbits(ret, mask);
}

unsigned int dvbtune::read24(unsigned int mask)
{
	if (index+2 >= buffer.size()) {
		return 0;
	}
	unsigned int ret = ((unsigned char)buffer.at(index) << 16) + ((unsigned char)buffer.at(index+1) << 8) + (unsigned char)buffer.at(index+2);
	index += 3;
	return maskbits(ret, mask);
}

unsigned int dvbtune::read16(unsigned int mask)
{
	if (index+1 >= buffer.size()) {
		return 0;
	}

	unsigned int ret = ((unsigned char)buffer.at(index) << 8) + (unsigned char)buffer.at(index+1);
	index += 2;
	return maskbits(ret, mask);
}

unsigned int dvbtune::read8(unsigned int mask)
{
	if (index >= buffer.size()) {
		return 0;
	}

	unsigned int ret = (unsigned char)buffer.at(index);
	index += 1;
	return maskbits(ret, mask);
}

QString dvbtune::readstr(unsigned int pos, unsigned int len)
{
	index += len;
	return buffer.mid(pos, len);

}

void dvbtune::closefd()
{
	if (frontend_name == "") {
		return;
	}
	while(!ready) {
		qDebug() << "waiting till ready to close";
		msleep(250);
	}
	close(frontend_fd);
	frontend_name = "";
}

void dvbtune::openfd()
{
	if (frontend_name == "") {
		frontend_name = "/dev/dvb/adapter" + QString::number(adapter) + "/frontend" + QString::number(frontend);
		frontend_fd = open(frontend_name.toStdString().c_str(), O_RDWR|O_NONBLOCK);
		if (frontend_fd < 0) {
			qDebug() << "Failed to open" << frontend_name;
			return;
		}
	}
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
	openfd();
	
	struct dtv_property p[1];
	p[0].cmd = DTV_ENUM_DELSYS;

	struct dtv_properties p_status;
	p_status.num = 1;
	p_status.props = p;

	if (ioctl(frontend_fd, FE_GET_PROPERTY, &p_status) == -1) {
		qDebug() << "FE_GET_PROPERTY failed";
		return;
	}	
	delsys.clear();
	for (;p[0].u.buffer.len > 0; p[0].u.buffer.len--) {
		delsys.append(p[0].u.buffer.data[p[0].u.buffer.len - 1]);
	}
	
	struct dvb_frontend_info fe_info;
	if (ioctl(frontend_fd, FE_GET_INFO, &fe_info) == -1) {
		qDebug() << "FE_GET_INFO failed";
		return;
	}
	caps	= fe_info.caps;
	name	= fe_info.name;
	fmin	= fe_info.frequency_min;
	fmax	= fe_info.frequency_max;
	fstep	= fe_info.frequency_stepsize;
}

void dvbtune::step_motor(int direction, int steps)
{
	// 0x68 = East
	// 0x69 = West
	// 0xFF = 1 step
	struct dvb_diseqc_master_cmd diseqc_cmd = { { 0xe0, 0x31, (__u8)(0x68 + direction), (__u8)(0xFF - (steps-1)), 0x00, 0x00 }, 4 };
	
	qDebug() << "cmd:" << hex
			 << diseqc_cmd.msg[0]
			 << diseqc_cmd.msg[1]
			 << diseqc_cmd.msg[2]
			 << diseqc_cmd.msg[3]
			 << diseqc_cmd.msg[4]
			 << diseqc_cmd.msg[5];
	
	if (ioctl(frontend_fd, FE_DISEQC_SEND_MASTER_CMD, &diseqc_cmd) == -1) {
		qDebug() << "FE_DISEQC_SEND_MASTER_CMD ERROR!";
	}	
}

void dvbtune::usals_drive(double sat_long)
{
	openfd();

	if (ioctl(frontend_fd, FE_SET_TONE, SEC_TONE_OFF) == -1)
		perror("FE_SET_TONE ERROR!");
	msleep(20);

	if (ioctl(frontend_fd, FE_SET_VOLTAGE, SEC_VOLTAGE_18) == -1)
		perror("FE_SET_VOLTAGE ERROR!");
	msleep(20);

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

	qDebug() << "cmd:" << hex
			 << diseqc_cmd.msg[0]
			 << diseqc_cmd.msg[1]
			 << diseqc_cmd.msg[2]
			 << diseqc_cmd.msg[3]
			 << diseqc_cmd.msg[4]
			 << diseqc_cmd.msg[5];

	msleep(1000);
	if (ioctl(frontend_fd, FE_DISEQC_SEND_MASTER_CMD, &diseqc_cmd) == -1) {
		qDebug() << "FE_DISEQC_SEND_MASTER_CMD ERROR!";
	}
	int howlong;
	if (!old_position) {
		howlong = 45000;
	} else {
		howlong = abs((int)(old_position - degree(sat_long))) * 500;
	}
	old_position = degree(sat_long);
	qDebug() << "Motor should take aprox" << howlong/1000 << "sec to move";
}

void dvbtune::gotox_drive(int position)
{
	struct dvb_diseqc_master_cmd diseqc_cmd = { { 0xe0, 0x31, 0x6B, (__u8)position, 0x00, 0x00 }, 4 };
	
	qDebug() << "cmd:" << hex
			 << diseqc_cmd.msg[0]
			 << diseqc_cmd.msg[1]
			 << diseqc_cmd.msg[2]
			 << diseqc_cmd.msg[3]
			 << diseqc_cmd.msg[4]
			 << diseqc_cmd.msg[5];
	
	if (ioctl(frontend_fd, FE_DISEQC_SEND_MASTER_CMD, &diseqc_cmd) == -1) {
		qDebug() << "FE_DISEQC_SEND_MASTER_CMD ERROR!";
	}	
}

void dvbtune::gotox_save(int position)
{
	struct dvb_diseqc_master_cmd diseqc_cmd = { { 0xe0, 0x31, 0x6A, (__u8)position, 0x00, 0x00 }, 4 };

	qDebug() << "cmd:" << hex
			 << diseqc_cmd.msg[0]
			 << diseqc_cmd.msg[1]
			 << diseqc_cmd.msg[2]
			 << diseqc_cmd.msg[3]
			 << diseqc_cmd.msg[4]
			 << diseqc_cmd.msg[5];
	
	if (ioctl(frontend_fd, FE_DISEQC_SEND_MASTER_CMD, &diseqc_cmd) == -1) {
		qDebug() << "FE_DISEQC_SEND_MASTER_CMD ERROR!";
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

	if (ioctl(frontend_fd, FE_SET_TONE, SEC_TONE_OFF) == -1) {
		qDebug() << "FE_SET_TONE ERROR!";
	}
	msleep(20);

	if (ioctl(frontend_fd, FE_SET_VOLTAGE, tp.voltage) == -1) {
		qDebug() << "FE_SET_VOLTAGE ERROR!";
	}
	servo ? msleep(500) : msleep(20);

	if (tune_ops.uncommitted > 0) {
		qDebug() << "Uncommitted:" << tune_ops.uncommitted;
		if (ioctl(frontend_fd, FE_DISEQC_SEND_MASTER_CMD, &uncommitted_switch_cmds[tune_ops.uncommitted-1]) == -1) {
			qDebug() << "FE_DISEQC_SEND_MASTER_CMD ERROR!";
		}
		msleep(20);
	}

	if (tune_ops.committed > 0) {
		qDebug() << "Committed:" << tune_ops.committed;
		if (ioctl(frontend_fd, FE_DISEQC_SEND_MASTER_CMD, &committed_switch_cmds[tune_ops.committed-1]) == -1) {
			qDebug() << "FE_DISEQC_SEND_MASTER_CMD ERROR!";
		}
		msleep(20);
	}

	if (tune_ops.tone) {
		if (ioctl(frontend_fd, FE_SET_TONE, !tune_ops.tone) == -1) {
			qDebug() << "FE_SET_TONE ERROR!";
		}
		msleep(20);
	}
}

void dvbtune::check_frontend()
{
	fe_status_t status;

	if (ioctl(frontend_fd, FE_READ_STATUS, &status) == -1) {
		qDebug() << "FE_READ_STATUS failed";
	}

	struct dtv_property p[12];
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

	struct dtv_properties p_status;
	p_status.num = 12;
	p_status.props = p;

	// get the actual parameters from the driver for that channel
	if (ioctl(frontend_fd, FE_GET_PROPERTY, &p_status) == -1) {
		qDebug() << "FE_GET_PROPERTY failed";
		return;
	}

	tp.frequency	= (int)p_status.props[0].u.data / 1000;
	tp.frequency	= abs(tp.frequency + tune_ops.f_lof);	
	tp.system		= p_status.props[1].u.data;
	tp.symbolrate	= p_status.props[2].u.data/1000;
	tp.modulation	= p_status.props[3].u.data;
	tp.fec			= p_status.props[4].u.data;
	tp.inversion	= p_status.props[5].u.data;
	tp.rolloff		= p_status.props[6].u.data;
	tp.pilot		= p_status.props[7].u.data;
	tp.matype		= p_status.props[8].u.data;
	tp.lvl_scale	= p_status.props[9].u.st.stat[0].scale;
	if (tp.lvl_scale == FE_SCALE_DECIBEL) {
		tp.lvl		= p_status.props[9].u.st.stat[0].svalue * 0.0001;
	} else {
		int lvl;
		if (ioctl(frontend_fd, FE_READ_SIGNAL_STRENGTH, &lvl) == -1) {
			tp.lvl = 0;
		} else {
			tp.lvl = (lvl * 100) / 0xffff;
			if (tp.lvl < 0) {
				tp.lvl = 0;
			}
		}
	}
	tp.snr_scale	= p_status.props[10].u.st.stat[0].scale;
	if (tp.snr_scale == FE_SCALE_DECIBEL) {
		tp.snr		= p_status.props[10].u.st.stat[0].svalue * 0.0001;
	} else {
		unsigned int snr = 0;
		if (ioctl(frontend_fd, FE_READ_SNR, &snr) == -1) {
			tp.snr = 0;
		} else {
			if (isATSC(tp.system) || isQAM(tp.system)) {
				tp.snr_scale = FE_SCALE_DECIBEL;
				tp.snr = snr/10.0;
			}
		}
	}
	tp.ber_scale	= p_status.props[11].u.st.stat[0].scale;
	if (p_status.props[11].u.st.stat[0].scale == FE_SCALE_COUNTER) {
		tp.ber		= p_status.props[11].u.st.stat[0].uvalue;
	} else {
		tp.ber = 0;
		if (ioctl(frontend_fd, FE_READ_BER, &tp.ber) == -1) {
			tp.ber = 0;
		}
	}

	tp.status		= status;
	emit updatesignal();
}

int dvbtune::tune()
{
	stop_demux();
	iq_x.clear();
	iq_y.clear();
	openfd();
	if (tp.system != SYS_ATSC &&
		tp.system != SYS_ATSCMH &&
		tp.system != SYS_DVBC_ANNEX_A &&
		tp.system != SYS_DVBC_ANNEX_B &&
		tp.system != SYS_DVBC_ANNEX_C &&
		tp.system != SYS_DVBC_ANNEX_AC &&
		tp.system != SYS_DVBT &&
		tp.system != SYS_DVBT2) {
		setup_switch();	
	}

	struct dtv_property p_clear[1];
	p_clear[0].cmd = DTV_CLEAR;

	struct dtv_properties cmdseq_clear;
	cmdseq_clear.num     = 1;
	cmdseq_clear.props   = p_clear;

	if ((ioctl(frontend_fd, FE_SET_PROPERTY, &cmdseq_clear)) == -1) {
		qDebug() << "FE_SET_PROPERTY DTV_CLEAR failed";
		return -1;
	}

	int i = 0;
	struct dtv_property p_tune[13];
	p_tune[i].cmd = DTV_DELIVERY_SYSTEM;	p_tune[i++].u.data = tp.system;
	p_tune[i].cmd = DTV_MODULATION;			p_tune[i++].u.data = tp.modulation;

	if (isSatellite(tp.system)) {
		qDebug() << "Satellite selected";
		p_tune[i].cmd = DTV_FREQUENCY;		p_tune[i++].u.data = abs(tp.frequency - abs(tune_ops.f_lof)) * 1000;
		p_tune[i].cmd = DTV_VOLTAGE;		p_tune[i++].u.data = tp.voltage;
		p_tune[i].cmd = DTV_SYMBOL_RATE;	p_tune[i++].u.data = tp.symbolrate * 1000;
		p_tune[i].cmd = DTV_TONE;			p_tune[i++].u.data = !tune_ops.tone;
		p_tune[i].cmd = DTV_INNER_FEC;		p_tune[i++].u.data = tp.fec;
		p_tune[i].cmd = DTV_INVERSION;		p_tune[i++].u.data = tp.inversion;
		p_tune[i].cmd = DTV_ROLLOFF;		p_tune[i++].u.data = tp.rolloff;
		p_tune[i].cmd = DTV_BANDWIDTH_HZ;	p_tune[i++].u.data = 0;
		p_tune[i].cmd = DTV_PILOT;			p_tune[i++].u.data = tp.pilot;
		p_tune[i].cmd = DTV_DVBS2_MIS_ID;	p_tune[i++].u.data = tune_ops.mis;
		qDebug() << "tune() Frequency: " << tp.frequency << dvbnames.voltage[tp.voltage] << tp.symbolrate;
	} else {
		int fr;
		qam myqam;
		atsc myatsc;

		switch (tp.system) {
		case SYS_DVBC_ANNEX_B:
			qDebug() << "QAM";
			fr = tp.frequency;
			if (fr < myqam.freq.at(0)) {
				tp.frequency = myqam.freq.at(0);
			} else if (fr > myqam.freq.at(myqam.freq.size()-1)) {
				tp.frequency = myqam.freq.at(myqam.freq.size()-1);
			} else {
				for(int c = 0; c < myqam.freq.size()-1; c++) {
					if (fr > myqam.freq.at(c) && fr < myqam.freq.at(c+1)) {
						int middle = (myqam.freq.at(c) + myqam.freq.at(c+1))/2;
						if (fr < middle) {
							tp.frequency = myqam.freq.at(c);
						} else {
							tp.frequency = myqam.freq.at(c+1);
						}
					}
				}
			}
			break;
		case SYS_ATSC:
		case SYS_ATSCMH:
			qDebug() << "ATSC";
			fr = tp.frequency;
			if (fr < myatsc.freq.at(0)) {
				tp.frequency = myatsc.freq.at(0);
			} else if (fr > myatsc.freq.at(myatsc.freq.size()-1)) {
				tp.frequency = myatsc.freq.at(myatsc.freq.size()-1);
			} else {
				for(int c = 0; c < myatsc.freq.size()-1; c++) {
					if (fr > myatsc.freq.at(c) && fr < myatsc.freq.at(c+1)) {
						int middle = (myatsc.freq.at(c) + myatsc.freq.at(c+1))/2;
						if (fr < middle) {
							tp.frequency = myatsc.freq.at(c);
						} else {
							tp.frequency = myatsc.freq.at(c+1);
						}
					}
				}
			}
			break;
		}

		p_tune[i].cmd = DTV_FREQUENCY;	p_tune[i++].u.data = tp.frequency * 1000;
		qDebug() << "tune() Frequency: " << tp.frequency;
	}
	p_tune[i++].cmd = DTV_TUNE;
	
	struct dtv_properties cmdseq_tune;
	cmdseq_tune.num     = i;
	cmdseq_tune.props   = p_tune;

	if (ioctl(frontend_fd, FE_SET_PROPERTY, &cmdseq_tune) == -1) {
		qDebug() << "FE_SET_PROPERTY TUNE failed";
		return -1;
	}

	// Keep trying for upto 2 second
	QTime t;
	t.start();
	fe_status_t status;
	while (t.elapsed() < 2000) {
		if (ioctl(frontend_fd, FE_READ_STATUS, &status) == -1) {
			qDebug() << "FE_READ_STATUS failed";
			return -1;
		}

		if (status & FE_TIMEDOUT) {
			qDebug() << "Tuning Failed, time:" << t.elapsed() << "status:" << hex << status;
			check_frontend();
			return -1;
		}
		
		if (status & FE_HAS_LOCK) {
			qDebug() << "Tuning Locked, time:" << t.elapsed() << "status:" << hex << status;
			check_frontend();
			emit updateresults();
			return 1;	
		} else {
			qDebug() << "No Lock...";
			msleep(200);		
		}
	}
	qDebug() << "Tuning Failed, time:" << t.elapsed() << "status:" << hex << status;
	check_frontend();
	return -1;
}

void dvbtune::get_bitrate()
{
	qDebug() << "get_bitrate()";
	
	QTime stime;
	int ttime = 0;
	
	if (dvr_name == "") {
		dvr_name = "/dev/dvb/adapter" + QString::number(adapter) + "/dvr0";
		qDebug() << "get_bitrate() opening" << dvr_name;
		dvr_fd = open(dvr_name.toStdString().c_str(), O_RDONLY|O_NONBLOCK);
		if (dvr_fd < 0) {
			qDebug() << "Failed to open" << dvr_name;
			return;
		}
	}
	
	char buf[BUFFY];
	memset(buf, 0, BUFFY);
	
	buffer.clear();
	stime.start();	
	while (stime.elapsed() < 2000 && buffer.size() < BUFFY && loop) {
		msleep(100);
		ready = false;
		int len = read(dvr_fd, buf, BUFFY);
		ready = true;
		if (len < 0) {
			qDebug() << "get_bitrate() read() failed";
			continue;
		}
		buffer.append(buf, len);
	}
	ttime = stime.elapsed();
		
	pids_rate.clear();
	pids_rate.fill(0, 0xFFFF+1);

	if (buffer.size() < 188) {
		qDebug() << "read size too small," << buffer.size() << " bytes";
		return;
	}
		
	int i = 0;
	int p = 0;
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
		pids_rate[p]++;
		pids_rate[0x2000]++;
		i += 188;
	}
	
	for (i = 0; i <= 0xFFFF; i++) {
		if (pids_rate[i] > 0) {
			pids_rate[i] = (pids_rate[i]*188*8)/ttime;
		}
	}
}

void dvbtune::demux_video()
{
	while (dmx_fd.size()) {
		while(!ready) {
			qDebug() << "waiting till ready to close";
			msleep(250);
		}
		ioctl(dmx_fd.last(), DMX_STOP);	
		close(dmx_fd.last());
		dmx_fd.pop_back();
	}

	dmx_name = "/dev/dvb/adapter" + QString::number(adapter) + "/demux0";

	struct dmx_pes_filter_params pesFilterParams;
	for(int a = 0; a < pids.size(); a++)
	{
		qDebug() << "Filtering PID:" << hex << pids[a];

		int temp_fd = open(dmx_name.toStdString().c_str(), O_RDWR|O_NONBLOCK);
		if (temp_fd < 0) {
			qDebug() << "Failed to open" << dmx_name;
			return;
		}
		dmx_fd.append(temp_fd);
		ioctl(dmx_fd.last(), DMX_SET_BUFFER_SIZE, BUFFY);

		pesFilterParams.pid = pids[a];
		pesFilterParams.input = DMX_IN_FRONTEND;
		pesFilterParams.output = DMX_OUT_TS_TAP;
		pesFilterParams.pes_type = DMX_PES_OTHER;
		pesFilterParams.flags = DMX_IMMEDIATE_START;
		if (ioctl(dmx_fd.last(), DMX_SET_PES_FILTER, &pesFilterParams) == -1) {
			qDebug() << "DEMUX: DMX_SET_PES_FILTER";
			return;
		}
	}
}

QByteArray dvbtune::demux_stream()
{
	if (dvr_name == "") {
		dvr_name = "/dev/dvb/adapter" + QString::number(adapter) + "/dvr0";
		qDebug() << "demux_stream() opening" << dvr_name;
//		dvr_fd = open(dvr_name.toStdString().c_str(), O_RDONLY|O_NONBLOCK);
		dvr_fd = open(dvr_name.toStdString().c_str(), O_RDONLY);
		if (dvr_fd < 0) {
			qDebug() << "Failed to open" << dvr_name;
			return QByteArray();
		}
	}

	int len;
	char buf[TCP_BUFSIZE];

	memset(buf, 0, TCP_BUFSIZE);
	ready = false;
	len = read(dvr_fd, buf, TCP_BUFSIZE);
	ready = true;

	if (len == -1) {
		return QByteArray();
	}
	if (len != TCP_BUFSIZE) {
		qDebug() << "dvr read issue:" << len << "of" << TCP_BUFSIZE;
	}
	
	return QByteArray(buf, len);
}

void dvbtune::demux_file()
{
	if (dvr_name == "") {
		dvr_name = "/dev/dvb/adapter" + QString::number(adapter) + "/dvr0";
		qDebug() << "demux_file() opening" << dvr_name;
		dvr_fd = open(dvr_name.toStdString().c_str(), O_RDONLY);
		if (dvr_fd < 0) {
			qDebug() << "Failed to open" << dvr_name;
			return;
		}
	}
	if (!out_fd) {
		qDebug() << "open out_fd";
		out_fd = open(out_name.toStdString().c_str(), O_CREAT|O_TRUNC|O_RDWR|O_NONBLOCK, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (out_fd < 0) {
			qDebug() << "Failed to open" << out_name;
			return;
		}
	}

	int buf_size = 188*348;
	char buf[buf_size];

	do {
		memset(buf, 0, buf_size);
		ready = false;
		int len = read(dvr_fd, buf, buf_size);
		ssize_t wlen = write(out_fd, buf, len);
		ready = true;
		emit demux_status(len);
		Q_UNUSED(wlen);
	} while (thread_function.indexOf("demux_file") != -1);
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

int dvbtune::demux_packet(int pid, unsigned char table, int timeout)
{
	if (sct_name == "") {
		sct_name = "/dev/dvb/adapter" + QString::number(adapter) + "/demux0";
		qDebug() << "demux_packet() opening" << sct_name;
		sct_fd = open(sct_name.toStdString().c_str(), O_RDWR);
		if (sct_fd < 0) {
			qDebug() << "Failed to open" << dmx_name;
			return -1;
		}
		if (ioctl(sct_fd, DMX_SET_BUFFER_SIZE, BUFFY) == -1) {
			qDebug() << "DEMUX: DMX_SET_BUFFER_SIZE";
			return -1;
		}
	}
	qDebug().nospace() << "demux_packet(0x" << hex << pid << ", " << table << ")";

	struct dmx_sct_filter_params sctfilter;
	memset(&sctfilter, 0, sizeof(struct dmx_sct_filter_params));
	sctfilter.pid = pid;
	if (table > 0) {
		sctfilter.filter.filter[0] = table;
		sctfilter.filter.mask[0] = 0xFF;
	}
	sctfilter.timeout = timeout;
	sctfilter.flags = DMX_IMMEDIATE_START | DMX_CHECK_CRC | DMX_ONESHOT;

	if (ioctl(sct_fd, DMX_SET_FILTER, &sctfilter) == -1) {
		qDebug() << "DEMUX: DMX_SET_FILTER";
		return -1;
	}
	
	index = 0;
	buffer.clear();
	char buf[BUFFY];
	memset(buf, 0, BUFFY);
	ready = false;
	int len = read(sct_fd, buf, BUFFY);
	ready = true;
	if (len < 0) {
		qDebug() << "demux_packet() read() error";
		return -1;
	}
	buffer.append(buf, len);

	ioctl(sct_fd, DMX_STOP);

	if (len > 10 && crc32()) {
		return 1;
	}
	qDebug() << "Failed to get correct CRC, giving up...";
	return -1;
}

void dvbtune::close_dvr()
{
	if (dvr_name != "") {
		while(!ready) {
			qDebug() << "waiting till ready to close";
			msleep(250);
		}
		qDebug() << "close_dvr() closing" << dvr_name;
		dvr_name = "";
		close(dvr_fd);
	}
	if (out_name != "") {
		while(!ready) {
			qDebug() << "waiting till ready to close";
			msleep(250);
		}
		qDebug() << "close_dvr() closing" << out_name;
		out_name = "";
		close(out_fd);
		out_fd = 0;
	}
}

void dvbtune::stop_demux()
{
	while (dmx_fd.size()) {
		while(!ready) {
			qDebug() << "waiting till ready to close";
			msleep(250);
		}
		ioctl(dmx_fd.last(), DMX_STOP);	
		qDebug() << "stop_demux() closing" << dmx_name;
		close(dmx_fd.last());
		dmx_fd.pop_back();
	}
	dmx_name = "";
	
	if (sct_name != "") {
		while(!ready) {
			qDebug() << "waiting till ready to close";
			msleep(250);
		}
		ioctl(sct_fd, DMX_STOP);
		qDebug() << "stop_demux() closing" << sct_name;
		close(sct_fd);
		sct_name = "";
	}
	if (dvr_name != "") {
		while(!ready) {
			qDebug() << "waiting till ready to close";
			msleep(250);
		}
		qDebug() << "stop_demux() closing" << dvr_name;
		close(dvr_fd);
		dvr_name = "";
	}
	if (out_name != "") {
		while(!ready) {
			qDebug() << "waiting till ready to close";
			msleep(250);
		}
		qDebug() << "stop_demux() closing" << out_name;
		close(out_fd);
		out_name = "";
	}
}

void dvbtune::spectrum_scan(dvb_fe_spectrum_scan *scan)
{
	QTime t;
	t.start();
	openfd();

	if (isSatellite(tp.system)) {
		setup_switch();	
	}

	if (ioctl(frontend_fd, FE_GET_SPECTRUM_SCAN, scan) != 0) {
		qDebug() << "Error! FE_GET_SPECTRUM_SCAN";
    }
	qDebug() << "Spectrum scan time: " << t.elapsed();
}

void dvbtune::iqplot()
{
    struct dvb_fe_constellation_samples const_samples;
    struct dvb_fe_constellation_sample samples[DTV_MAX_CONSTELLATION_SAMPLES];
    const_samples.num = DTV_MAX_CONSTELLATION_SAMPLES;
    const_samples.samples = samples;
	const_samples.options = iq_options;

	ready = false;
	if ((ioctl(frontend_fd, FE_GET_CONSTELLATION_SAMPLES, &const_samples)) == -1) {
		qDebug() << "ERROR: FE_GET_CONSTELLATION_SAMPLES";
		return;
	}
	for (unsigned int i = 0 ; i < const_samples.num ; i++) {
		while (iq_x.size() >= (int)const_samples.num * PERSISTENCE) {
			iq_x.erase(iq_x.begin());
			iq_y.erase(iq_y.begin());
		}
		iq_x.append(samples[i].imaginary);
		iq_y.append(samples[i].real);
	}
	ready = true;
	emit iqdraw(iq_x, iq_y);
}

void dvbtune::run()
{
	emit adapter_status(adapter, true);
	
	QTime check_frontend_t;
	check_frontend_t.start();
	QTime iqplot_t;
	iqplot_t.start();
	loop = true;
	do {
		if (thread_function.indexOf("demux_file") != -1) {
			demux_file();
		}
		if (thread_function.indexOf("iqplot") != -1) {
			if (iqplot_t.elapsed() > 500) {
				iqplot();
				iqplot_t.restart();
			}
		}
		if (thread_function.indexOf("check_frontend") != -1) {
			if (check_frontend_t.elapsed() > 2000) {
				check_frontend();
				check_frontend_t.restart();
			}
		}
		if (thread_function.indexOf("tune") != -1) {
			tune();
			thread_function.append("check_frontend");
			thread_function.remove(thread_function.indexOf("tune"));			
		}
		msleep(100);
	} while(loop);
	qDebug() << "dvbtune() run() complete";
	thread_function.clear();

	emit adapter_status(adapter, false);
}
