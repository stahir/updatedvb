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
	void setup_demux(QString type = "TS");
	dvbtune *mytune;
	bool shutdown;

signals:
	void server_new();
	void adapter_status(int adapter);

public slots:
	void update_status(QString text, int time = STATUS_REMOVE);

private slots:
	void update_signal();
	void update_results();
	void on_pushButton_play_clicked();
	void on_pushButton_ipcleaner_clicked();
	void on_pushButton_demux_clicked();
	void on_pushButton_file_clicked();
	void on_pushButton_expand_clicked();
	void on_pushButton_unexpand_clicked();
	void on_pushButton_stream_clicked();
	void on_pushButton_bbframe_clicked();
	void on_treeWidget_itemClicked(QTreeWidgetItem * item, int column);
	void list_create();
	void tree_create_root(int *parent, QString text, int pid);
	void tree_create_child(int *parent, QString text, int pid);
	void tree_select_children(QTreeWidgetItem * item);
	void setcolor(int index, QColor color);
	void on_listWidget_itemClicked(QListWidgetItem *item);
	void delete_iqplot();
	void delete_demux_file();
	void on_pushButton_iqplot_clicked();
	void parsetp_done();
	void parsetp_start();
	void myProcess_finished();

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
	QPointer<iqplot> myiqplot;
	QPointer<demux_file> mydemux_file;
	bool parsetp_started;
	QTime unlock_t;
	dvbstream_thread mystream;
	QThread mystream_thread;
	QThread reader_thread;
	QVector<QString> mystatus;
	QSignalMapper status_mapper;
	QTimer *status_timer;

protected:
	void closeEvent(QCloseEvent *event);
	void keyPressEvent(QKeyEvent *event);
};

#endif // TUNING_H
