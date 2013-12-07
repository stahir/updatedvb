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

#include "blindscan_thread.h"

blindscan_thread::blindscan_thread()
{

}

blindscan_thread::~blindscan_thread()
{
	qDebug() << "~blindscan_thread()";
}

void blindscan_thread::run()
{
	loop = true;
	do {
		if (thread_function.indexOf("blindscan") != -1) {
			switch(mytune->tune_ops.voltage) {
			case 0:
			case 1:
			case 2:
				mytune->tp.voltage = mytune->tune_ops.voltage;
				blindscan();
				loop = false;
				break;
			case 3:
				mytune->tp.voltage = SEC_VOLTAGE_13;
				blindscan();
				mytune->tp.voltage = SEC_VOLTAGE_18;
				blindscan();
				loop = false;
				break;
			}
		}
		if (thread_function.indexOf("smartscan") != -1) {
			smartscan();
			loop = false;
		}
	} while (loop);
	thread_function.clear();
}

void blindscan_thread::smartscan()
{
	QTime t;
	t.start();
	qDebug() << "smartscan()";

	float size = mytune->tp_try.size();
	int progress = 0;
	for(int i = 0; i < mytune->tp_try.size() && loop; i++) {
		progress = ((i+1)/size)*100;
		mytune->tp.frequency	= mytune->tp_try.at(i).frequency;
		mytune->tp.voltage		= mytune->tp_try.at(i).voltage;
		mytune->tp.symbolrate	= 1000;
		ready = false;
		mytune->tune();
		while (!ready) {
			msleep(10);
		}
		emit updateprogress(progress);
	}
	ready = true;
	qDebug() << "Total time: " << t.elapsed();
}

void blindscan_thread::blindscan()
{
	QTime t;
	t.start();
	qDebug() << "blindscan()";

	float size = abs(mytune->tune_ops.f_start-mytune->tune_ops.f_stop) - 18;
	float rolloff;
	int progress = 0;
	mytune->tp.frequency	= mytune->tune_ops.f_start;
	mytune->tp.symbolrate	= 1000;
	while (mytune->tp.frequency < mytune->tune_ops.f_stop && loop) {
		ready = false;
		mytune->tune();
		while (!ready) {
			msleep(10);
		}
		if ( mytune->tp.status & FE_HAS_LOCK ) {
			switch(mytune->tp.rolloff) {
			case 1:
				rolloff = 1.20;
				break;
			case 2:
				rolloff = 1.25;
				break;
			case 0:
			default:
				rolloff = 1.35;
				break;
			}
			mytune->tp.frequency += ((mytune->tp.symbolrate/1000) * rolloff)/2 + 9;
			mytune->tp.symbolrate = 1000;
		} else {
			mytune->tp.frequency += 18;
		}
		progress = ((mytune->tp.frequency-mytune->tune_ops.f_start)/size)*100;
		emit updateprogress(progress);
	}
	ready = true;
	qDebug() << "Total time: " << t.elapsed();
}
