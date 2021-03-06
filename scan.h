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

#ifndef scan_H
#define scan_H

#include <QDebug>
#include <QThread>
#include <QVector>
#include "dvb_settings.h"
#include "dvbtune.h"

class scan : public QThread
{
    Q_OBJECT
signals:
	void signaldraw(QVector<double> x, QVector<double> y, int min, int max, int cindex, unsigned int scale);
	void update_status(QString text, int time);
	void markers_draw();

public:
	bool loop;
	waitout mutex;
	dvbtune *mytune;
	unsigned int points;
	unsigned int step;
	int min, max, min_old, max_old;
	QVector<double> x;
	QVector<double> y;
	int loop_delay;

	scan();
	~scan();
	void sweep();
	void rescale();
private:
	struct dvb_fe_spectrum_scan fe_scan;
	dvb_settings dvbnames;
	int f_start, f_stop;

	void run();
};

#endif // scan_H
