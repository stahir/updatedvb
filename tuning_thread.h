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

#ifndef TUNING_THREAD_H
#define TUNING_THREAD_H

#include <QThread>
#include <QtCore>
#include <QDebug>
#include "dvbtune.h"
#include "dvb_settings.h"

class tuning_thread : public QThread
{
	Q_OBJECT	
public:
	tuning_thread();
	~tuning_thread();
	void run();
	void parsetp();
	unsigned int dtag_convert(unsigned int temp);
	int parse_pat();
	int parse_pmt();
	int parse_sdt();
	int parse_psip();
	int parse_descriptor(int parent);

	dvb_settings dvbnames;
	dvbtune *mytune;
	dvb_pat mypat;
	dvb_sdt mysdt;

	bool loop;
	bool ready;
	QVector<QString> thread_function;
	
signals:
	void setcolor(int parent, QColor color);
	void list_create(QString text, int pid);
	void tree_create_root(int *parent, QString text, int pid);
	void tree_create_child(int *parent, QString text, int pid);

private:
	void tree_create_root_wait(int *parent, QString text, int pid);
	void tree_create_child_wait(int *parent, QString text, int pid);
};

#endif // TUNING_THREAD_H
