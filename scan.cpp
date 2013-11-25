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

#include "scan.h"

scan::scan()
{
	min		= -1;
	max		= -1;
	loop	= false;
	ready	= true;
	step	= 5;
}

scan::~scan()
{
	qDebug() << "~scan()";
}

void scan::run()
{
	do {
		mytune->tp_try.clear();		
		switch(mytune->tune_ops.voltage) {
		case 0:
		case 1:
		case 2:
			mytune->tp.voltage = mytune->tune_ops.voltage;
			sweep();
			break;
		case 3: // Both polarities
			mytune->tp.voltage = SEC_VOLTAGE_13;
			sweep();
			mytune->tp.voltage = SEC_VOLTAGE_18;
			sweep();
			break;
		}
	} while(loop);
}

void scan::rescale() {
	if (!x.size()) {
		return;
	}
	QVector<double> ys = y;
	qSort(ys);
	if (min == -1 || min > ys[ys.size() * 0.05]) {
		min = ys[ys.size() * 0.05];
	}
	if (max == -1 || max < ys[ys.size() - 1] * 1.01) {
		max = ys[ys.size() - 1] * 1.01;
	}


	tp_info tmp;
	int slope;
	if (mytune->tp.system == SYS_ATSC ||
		mytune->tp.system == SYS_ATSCMH ||
		mytune->tp.system == SYS_DVBC_ANNEX_A ||
		mytune->tp.system == SYS_DVBC_ANNEX_B ||
		mytune->tp.system == SYS_DVBC_ANNEX_C ||
		mytune->tp.system == SYS_DVBC_ANNEX_AC ||
		mytune->tp.system == SYS_DVBT ||
		mytune->tp.system == SYS_DVBT2) {
		slope = 10 / (step/1000);
	} else {
		slope = 10 / step;
	}
	unsigned int threshold = (max-min)/8;
	unsigned int start = 0;
	unsigned int end = 0;
	unsigned int tmax = 0;
	for(int i = 0; i < x.size(); i++) {
		if (i > slope) {
			if ( (y.at(i) - y.at(i-slope)) > threshold ) {
				start = x.at(i-slope);
				if (!tmax) {
					tmax = y.at(i);
				}
			}
			if (tmax && y.at(i) > tmax) {
				tmax = y.at(i);
			}
			if ( start && (y.at(i-slope) - y.at(i)) > threshold ) {
				end = x.at(i);
				tmp.frequency			= (start + end)/2;
				tmp.voltage				= mytune->tp.voltage;
				tmp.spectrumscan_lvl	= tmax;
				mytune->tp_try.append(tmp);
				start = 0;
				end = 0;
				tmax = 0;
			}
		}
	}

	emit signaldraw(x, y, min, max, mytune->tp.voltage);
}

void scan::sweep()
{
	while (!ready) {
		msleep(10);
	}
	ready = false;
	
	qDebug() << "sweep() - Start:" << mytune->tune_ops.f_start << "Stop:" << mytune->tune_ops.f_stop << "lof:" << mytune->tune_ops.f_lof << "voltage:" << dvbnames.voltage[mytune->tp.voltage];	
	
	short unsigned int rf_levels_h[65535];
	struct dvb_fe_spectrum_scan scan;

	int f_start, f_stop, direction;
	if (abs(mytune->tune_ops.f_start - abs(mytune->tune_ops.f_lof)) < abs(mytune->tune_ops.f_stop - abs(mytune->tune_ops.f_lof))) {
		direction = 1;
		f_start	= abs(mytune->tune_ops.f_start - abs(mytune->tune_ops.f_lof));
		f_stop	= abs(mytune->tune_ops.f_stop - abs(mytune->tune_ops.f_lof));
	} else {
		direction = -1;
		f_start	= abs(mytune->tune_ops.f_stop - abs(mytune->tune_ops.f_lof));
		f_stop	= abs(mytune->tune_ops.f_start - abs(mytune->tune_ops.f_lof));
	}
		
	scan.step_size			= step * 1000;
	scan.start_frequency	= f_start * 1000;
	scan.num_steps			= ((f_stop - f_start) / step) + 1;
	scan.rf_level			= rf_levels_h;

	mytune->spectrum_scan(&scan);

	x.clear();
	y.clear();
	for(unsigned int i = 0; i < scan.num_steps; i++) {
		x.append(abs(f_start + mytune->tune_ops.f_lof) + (i * step * direction));
		y.append(rf_levels_h[i]);
	}
	rescale();
}

void scan::setup()
{
	min	= -1;
	max	= -1;
	ready = true;
}
