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
	min_old = -1;
	max_old = -1;
	loop	= false;
	loop_delay	= 0;
	ready	= true;
	step	= 5;
	mytune	= NULL;
	f_start	= 0;
	f_stop	= 0;
}

scan::~scan()
{
}

void scan::run()
{
	mytune->setbit(TUNER_SCAN);
	QTime t;
	t.start();
	do {
		while (!ready) {
			msleep(10);
		}
		ready = false;
		mytune->tp_try.clear();
		min_old = -1;
		max_old = -1;

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
			min_old = min;
			max_old = max;
			mytune->tp.voltage = SEC_VOLTAGE_18;
			sweep();
			break;
		}
		emit markers_draw();

		while (t.elapsed() < loop_delay*1000 && loop) {
			msleep(100);
		}
		t.restart();
	} while(loop);
	mytune->unsetbit(TUNER_SCAN);
	mytune->closefd();
}

void scan::rescale() {
	if (x.size() <= 1 || y.size() <= 1) {
		return;
	}

	QVector<double> ys = y;
	qSort(ys);
	ys.removeAll(0);
	if (ys.isEmpty()) {
		return;
	}
	min = ys[ys.size() * 0.05];
	max = ys[ys.size() - 1];
	int dev = (max - min);
	int slope;
	if (isSatellite(mytune->tp.system)) {
		max += dev * 0.10;
		slope = 10 / step;
	} else {
		max += dev * 0.40; // non-satellite needs a little extra room because of the channel number
		slope = 1;
	}

	if (min_old > 0 && max_old > 0) {
		if (min_old < min) {
			min = min_old;
		}
		if (max_old > max) {
			max = max_old;
		}
	}
	for (int i = 0; i < y.size(); i++) {
		if (y.at(i) < min || y.at(i) == 0) {
			y[i] = min;
		}
	}

	tp_info tmp;

	unsigned int threshold = (max-min)/8;
	unsigned int start = 0;
	unsigned int end = 0;
	int tmax = 0;
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
		if ( start && (y.at(i-slope) - y.at(i)) > threshold && tmax > min) {
			end = x.at(i);
			tmp.frequency			= (start + end)/2;
			tmp.voltage				= mytune->tp.voltage;
			tmp.system				= mytune->tp.system;
			tmp.spectrumscan_lvl	= tmax;
			mytune->tp_try.append(tmp);
			start = 0;
			end = 0;
			tmax = 0;
		}
	}

	emit signaldraw(x, y, min, max, mytune->tp.voltage, *fe_scan.type);
}

void scan::sweep()
{
	qDebug() << "sweep() - Start:" << mytune->tune_ops.f_start << "Stop:" << mytune->tune_ops.f_stop << "lof:" << mytune->tune_ops.f_lof << "voltage:" << dvbnames.voltage[mytune->tp.voltage];

	QTime t;
	t.start();

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

	emit update_status("Scanning...", STATUS_NOEXP);

	short int rf_levels_h[65535];

	if (isSatellite(mytune->tp.system) || step == 1) {
		if (!isSatellite(mytune->tp.system)) {
			step *= 1000;
		}

		fe_scan.type		= new __u32;
		fe_scan.rf_level	= rf_levels_h;
		fe_scan.num_freq	= ((f_stop - f_start) / step) + 1;
		fe_scan.freq		= (__u32*) malloc(fe_scan.num_freq * sizeof(__u32));
		for (int i = 0; i < fe_scan.num_freq; i++) {
			*(fe_scan.freq + i) = (f_start + (i * step)) * 1000;
		}

		mytune->spectrum_scan(&fe_scan);

		x.clear();
		y.clear();
		for(unsigned int i = 0; i < fe_scan.num_freq; i++) {
			x.append(abs(mytune->tune_ops.f_lof + ((long int)*(fe_scan.freq + i)/1000)));
			y.append(rf_levels_h[i]);
		}
	} else {
		freq_list myfreq;
		if (isQAM(mytune->tp.system)) {
			myfreq.qam();
		} else if (isATSC(mytune->tp.system)) {
			myfreq.atsc();
		} else if (isDVBT(mytune->tp.system)) {
			myfreq.dvbt();
		}

		QVector<unsigned long int> freq;
		for (int i = 0; i < myfreq.freq.size(); i++) {
			if (myfreq.freq.at(i) >= f_start && myfreq.freq.at(i) <= f_stop) {
				freq.append(myfreq.freq.at(i));
			}
		}

		fe_scan.type		= new __u32;
		fe_scan.rf_level	= rf_levels_h;
		fe_scan.num_freq	= freq.size();
		fe_scan.freq		= (__u32*) malloc(freq.size() * sizeof(__u32));
		for (int i = 0; i < fe_scan.num_freq; i++) {
			*(fe_scan.freq + i) = freq.at(i) * 1000;
		}

		mytune->spectrum_scan(&fe_scan);

		x.clear();
		y.clear();
		for(unsigned int i = 0; i < fe_scan.num_freq; i++) {
			x.append(abs(mytune->tune_ops.f_lof + ((long int)*(fe_scan.freq + i)/1000)) - 3000);
			y.append(0);
			x.append(abs(mytune->tune_ops.f_lof + ((long int)*(fe_scan.freq + i)/1000)));
			y.append(rf_levels_h[i]);
			x.append(abs(mytune->tune_ops.f_lof + ((long int)*(fe_scan.freq + i)/1000)) + 3000);
			y.append(0);
		}
	}

	rescale();
	emit update_status("", STATUS_CLEAR);
	emit update_status("Completed in " + QString::number(t.elapsed()/1000.0, 'f', 1) + "s", 2);

	free(fe_scan.type);
	free(fe_scan.freq);
}

void scan::setup()
{
	ready = true;
}
