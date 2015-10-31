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

#include <QDialog>
#include <QThread>
#include <QtCore>
#include <QDebug>
#include <QDate>
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
	void parse_pat();
	void parse_pmt();
	void parse_sdt();
	void parse_dcii_sdt();
	void parse_cat();
	void parse_eit();
	void parse_nit();
	void parse_psip_eit();
	void parse_psip_stt();
	void parse_psip_ett();
	void parse_psip_mgt();
	void parse_psip_rrt();
	void parse_psip_tvct();
	void parse_tdt();
	void parse_etm(tree_item *item, QString desc = "Text");
	void parse_descriptor(tree_item *item);
	void filter_pids(unsigned int pid, QVector<unsigned int> table, QVector<unsigned int> mask, unsigned int timeout = 300);

	dvb_settings dvbnames;
	dvbtune *mytune;
	dvb_pat mypat;
	dvb_sdt mysdt;

	bool loop;
	bool parsetp_loop;
	bool parsetp_running;
	QVector<QString> thread_function;
	QVector<int> pid_parent;
	QVector<dvb_pids> fpids;

	waitout mutex;
	
signals:
	void list_create();
	void tree_create(tree_item *item);
	void parsetp_done();

private:
	void tree_create_wait(tree_item *item);

protected:
	void closeEvent(QCloseEvent *event);
};

#endif // TUNING_THREAD_H
