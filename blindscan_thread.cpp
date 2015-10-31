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
	mytune	= NULL;
	loop	= false;
}

blindscan_thread::~blindscan_thread()
{
}

void blindscan_thread::run()
{
	loop = true;
	do {
		if (thread_function.contains("blindscan")) {
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
		if (thread_function.contains("smartscan")) {
			smartscan();
			loop = false;
		}
		if (thread_function.isEmpty()) {
			msleep(100);
		}
	} while (loop);
	thread_function.clear();
}

void blindscan_thread::smartscan()
{
	QTime t;
	t.start();

	float size = mytune->tp_try.size();
	for(int i = 0; i < mytune->tp_try.size() && loop; i++) {
		int progress = ((i+1)/size)*100;
		mytune->tp.frequency	= mytune->tp_try.at(i).frequency;
		mytune->tp.voltage	= mytune->tp_try.at(i).voltage;
		mytune->tp.system	= mytune->tp_try.at(i).system;
		mytune->tp.symbolrate	= 1000;
		mutex.lock();
		mytune->tune();
		mutex.wait(&loop);
		emit update_progress(progress);
	}
	qDebug() << "Total time: " << t.elapsed();
}

void blindscan_thread::blindscan()
{
	QTime t;
	t.start();

	float size = abs(mytune->tune_ops.f_start-mytune->tune_ops.f_stop) - 18;
	float rolloff;
	mytune->tp.frequency	= mytune->tune_ops.f_start;
	mytune->tp.symbolrate	= 1000;
	while (mytune->tp.frequency < mytune->tune_ops.f_stop && loop) {
		mutex.lock();
		mytune->tune();
		mutex.wait(&loop);
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
		int progress = ((mytune->tp.frequency-mytune->tune_ops.f_start)/size)*100;
		emit update_progress(progress);
	}
	qDebug() << "Total time: " << t.elapsed();
}
