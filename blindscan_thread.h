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

#ifndef BLINDSCAN_THREAD_H
#define BLINDSCAN_THREAD_H

#include <QThread>
#include <QTime>
#include <QDebug>
#include "dvbtune.h"
#include "tuning.h"
#include "dvb_settings.h"

class blindscan_thread : public QThread
{
	Q_OBJECT
public:
	blindscan_thread();
	~blindscan_thread();
	void run();

	dvbtune *mytune;
	bool loop;
	waitout mutex;
	QVector<QString> thread_function;
signals:
	void update_progress(int i);
private:
	void blindscan();
	void smartscan();
	QSettings *mysettings;
};

#endif // BLINDSCAN_THREAD_H
