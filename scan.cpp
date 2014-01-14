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
	mytune	= NULL;
	f_start	= 0;
	f_stop	= 0;
}

scan::~scan()
{
	qDebug() << "~scan()";
}

void scan::run()
{
	do {
		while (!ready) {
			msleep(10);
		}
		ready = false;
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
	if (x.size() <= 1 || y.size() <= 1) {
		return;
	}
	QVector<double> ys = y;
	qSort(ys);
	for (int i = 0; i < y.size(); i++) {
		if (y.at(i) == 0) {
			ys.remove(0);
		}
	}
	if (ys.size() <= 1) {
		return;
	}
	min = ys[ys.size() * 0.05];
	max = ys[ys.size() - 1];
	int dev = (max - min);
	min += dev * 0.10;
	max += dev * 0.10;
	for (int i = 0; i < y.size(); i++) {
		if (y.at(i) < min) {
			y[i] = min;
		}
	}

	tp_info tmp;
	int slope;
	if (isATSC(mytune->tp.system) || isQAM(mytune->tp.system)) {
		slope = 1;
	} else {
		slope = 10 / step;
	}

	unsigned int threshold = (max-min)/8;
	unsigned int start = 0;
	unsigned int end = 0;
	unsigned int tmax = 0;
	for(int i = slope; i < x.size(); i++) {
		if ( (y.at(i) - y.at(i-slope)) > threshold ) {
			start = x.at(i-slope);
			if (!tmax) {
				tmax = y.at(i);
			}
		}
		if (tmax && y.at(i) > tmax) {
			tmax = y.at(i);
		}
		if ( start && (y.at(i-slope) - y.at(i)) > threshold && tmax > (unsigned int)min) {
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

	emit signaldraw(x, y, min, max, mytune->tp.voltage);
}

void scan::sweep_atsc()
{
	short unsigned int rf_levels_h[65535];
	struct dvb_fe_spectrum_scan scan;

	QVector<unsigned long int> freq;
	atsc myatsc;
	if (step != 1) {
		for (int i = 0; i < myatsc.freq.size(); i++) {
			if (myatsc.freq.at(i) >= f_start && myatsc.freq.at(i) <= f_stop) {
				freq.append(myatsc.freq.at(i));
			}
		}
	
		scan.rf_level	= rf_levels_h;		
		scan.num_freq	= freq.size();
		scan.freq		= (__u32*) malloc(freq.size() * sizeof(__u32));
		for (int i = 0; i < scan.num_freq; i++) {
			*(scan.freq + i) = freq.at(i) * 1000;
		}

		mytune->spectrum_scan(&scan);

		x.clear();
		y.clear();
		for(unsigned int i = 0; i < scan.num_freq; i++) {
			x.append(abs(mytune->tune_ops.f_lof + ((long int)*(scan.freq + i)/1000)) - 3000);
			y.append(0);
			x.append(abs(mytune->tune_ops.f_lof + ((long int)*(scan.freq + i)/1000)));
			y.append(rf_levels_h[i]);
			x.append(abs(mytune->tune_ops.f_lof + ((long int)*(scan.freq + i)/1000)) + 3000);
			y.append(0);
		}
	} else {
		step = 3000;
		scan.rf_level	= rf_levels_h;
		scan.num_freq	= ((f_stop - f_start) / step) + 1;
		scan.freq		= (__u32*) malloc(scan.num_freq * sizeof(__u32));
		for (int i = 0; i < scan.num_freq; i++) {
			*(scan.freq + i) = (f_start + (i * step)) * 1000;
		}

		mytune->spectrum_scan(&scan);

		x.clear();
		y.clear();
		for(unsigned int i = 0; i < scan.num_freq; i++) {
			x.append(abs(mytune->tune_ops.f_lof + ((long int)*(scan.freq + i)/1000)));
			y.append(rf_levels_h[i]);
		}
	}
	free(scan.freq);
}

void scan::sweep_qam()
{
	short unsigned int rf_levels_h[65535];
	struct dvb_fe_spectrum_scan scan;
	
	QVector<unsigned long int> freq;
	qam myqam;
	if (step != 1) {
		for (int i = 0; i < myqam.freq.size(); i++) {
			if (myqam.freq.at(i) >= f_start && myqam.freq.at(i) <= f_stop) {
				freq.append(myqam.freq.at(i));
			}
		}
	
		scan.rf_level	= rf_levels_h;		
		scan.num_freq	= freq.size();
		scan.freq		= (__u32*) malloc(freq.size() * sizeof(__u32));
		for (int i = 0; i < scan.num_freq; i++) {
			*(scan.freq + i) = freq.at(i) * 1000;
		}
	
		mytune->spectrum_scan(&scan);

		x.clear();
		y.clear();
		for(unsigned int i = 0; i < scan.num_freq; i++) {
			x.append(abs(mytune->tune_ops.f_lof + ((long int)*(scan.freq + i)/1000)) - 3000);
			y.append(0);
			x.append(abs(mytune->tune_ops.f_lof + ((long int)*(scan.freq + i)/1000)));
			y.append(rf_levels_h[i]);
			x.append(abs(mytune->tune_ops.f_lof + ((long int)*(scan.freq + i)/1000)) + 3000);
			y.append(0);
		}
	} else {
		step = 3000;
		scan.rf_level	= rf_levels_h;
		scan.num_freq	= ((f_stop - f_start) / step) + 1;
		scan.freq		= (__u32*) malloc(scan.num_freq * sizeof(__u32));
		for (int i = 0; i < scan.num_freq; i++) {
			*(scan.freq + i) = (f_start + (i * step)) * 1000;
		}

		mytune->spectrum_scan(&scan);

		x.clear();
		y.clear();
		for(unsigned int i = 0; i < scan.num_freq; i++) {
			x.append(abs(mytune->tune_ops.f_lof + ((long int)*(scan.freq + i)/1000)));
			y.append(rf_levels_h[i]);
		}
	}
	free(scan.freq);
}

void scan::sweep_satellite()
{
	short unsigned int rf_levels_h[65535];
	struct dvb_fe_spectrum_scan scan;

	scan.rf_level	= rf_levels_h;
	scan.num_freq	= ((f_stop - f_start) / step) + 1;
	scan.freq		= (__u32*) malloc(scan.num_freq * sizeof(__u32));
	for (int i = 0; i < scan.num_freq; i++) {
		*(scan.freq + i) = (f_start + (i * step)) * 1000;
	}

	mytune->spectrum_scan(&scan);

	x.clear();
	y.clear();
	for(unsigned int i = 0; i < scan.num_freq; i++) {
		x.append(abs(mytune->tune_ops.f_lof + ((long int)*(scan.freq + i)/1000)));
		y.append(rf_levels_h[i]);
	}
	free(scan.freq);
}

void scan::sweep()
{
	qDebug() << "sweep() - Start:" << mytune->tune_ops.f_start << "Stop:" << mytune->tune_ops.f_stop << "lof:" << mytune->tune_ops.f_lof << "voltage:" << dvbnames.voltage[mytune->tp.voltage];	

	if (abs(mytune->tune_ops.f_start - abs(mytune->tune_ops.f_lof)) < abs(mytune->tune_ops.f_stop - abs(mytune->tune_ops.f_lof))) {
		f_start	= abs(mytune->tune_ops.f_start - abs(mytune->tune_ops.f_lof));
		f_stop	= abs(mytune->tune_ops.f_stop - abs(mytune->tune_ops.f_lof));
	} else {
		f_start	= abs(mytune->tune_ops.f_stop - abs(mytune->tune_ops.f_lof));
		f_stop	= abs(mytune->tune_ops.f_start - abs(mytune->tune_ops.f_lof));
	}

	if (f_start < mytune->fmin/1000) {
		f_start = mytune->fmin/1000;
	}
	if (f_start > mytune->fmax/1000) {
		f_start = mytune->fmax/1000;
	}
	if (f_stop < mytune->fmin/1000) {
		f_stop = mytune->fmin/1000;
	}
	if (f_stop > mytune->fmax/1000) {
		f_stop = mytune->fmax/1000;
	}
	if (!abs(f_stop-f_start)) {
		return;
	}

	emit update_status("Scanning...", 0);
	if (isSatellite(mytune->tp.system)) {
		sweep_satellite();
	}
	if (isATSC(mytune->tp.system)) {
		sweep_atsc();
	}

	if (isQAM(mytune->tp.system)) {
		sweep_qam();
	}
	emit update_status("Scanning...", -1);
	rescale();
	emit update_status("Done", 1);
}

void scan::setup()
{
	ready = true;
}
