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

#ifndef TUNING_H
#define TUNING_H

#include <QWidget>
#include <QtCore>
#include <QProcess>
#include <QThread>
#include <QDebug>
#include <QTreeWidget>
#include <QListWidget>
#include <QStatusBar>
#include <QThread>
#include <QKeyEvent>
#include "demux_file.h"
#include "demux_dvr.h"
#include "dvbtune.h"
#include "dvb_settings.h"
#include "tuning_thread.h"
#include "dvbstream_thread.h"
#include "iqplot.h"

namespace Ui {
class tuning;
}

class tuning : public QWidget
{
	Q_OBJECT
public:
	explicit tuning(QWidget *parent = 0);
	~tuning();

	void init();
	void setup_demux();
	dvbtune *mytune;
	bool shutdown;

signals:
	void setup_server();

public slots:
	void update_status(QString text, int time);

private slots:
	void updatesignal();
	void updateresults();
	void stop_demux();
	void on_pushButton_play_clicked();
	void on_pushButton_ipcleaner_clicked();
	void on_pushButton_demux_clicked();
	void on_pushButton_file_clicked();
	void on_pushButton_expand_clicked();
	void on_pushButton_unexpand_clicked();
	void on_pushButton_stream_clicked();
	void on_treeWidget_itemClicked(QTreeWidgetItem * item, int column);
	void list_create(QString text, int pid);
	void tree_create_root(int *parent, QString text, int pid);
	void tree_create_child(int *parent, QString text, int pid);
	void setcolor(int index, QColor color);
	void on_listWidget_itemClicked(QListWidgetItem *item);
	void delete_iqplot();
	void on_pushButton_iqplot_clicked();
	void parsetp_done();

private:
	Ui::tuning *ui;
	QStatusBar *mystatusbar;
	QSettings *mysettings;
	dvb_settings dvbnames;
	QProcess myProcess;
	QVector<int> tree_pid;
	QVector<int> list_pid;
	QVector<QTreeWidgetItem *> tree_item;
	QVector<QListWidgetItem *> list_item;
	tuning_thread mythread;
	dvbstream_thread mystream;
	QThread mystream_thread;
	iqplot *myiqplot;
	bool myiqplot_running;
	bool parsetp_completed;

protected:
	void closeEvent(QCloseEvent *event);
	void keyPressEvent(QKeyEvent *event);
};

#endif // TUNING_H
