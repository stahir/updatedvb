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

#include <qglobal.h>
#if QT_VERSION >= 0x050000
        #include <QtWidgets>
#else
        #include <QtGui>
#endif

#include <QDialog>
#include <QtCore>
#include <QProcess>
#include <QThread>
#include <QDebug>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_plot_scaleitem.h>
#include <qwt_scale_engine.h>
#include "demux_file.h"
#include "demux_dvr.h"
#include "dvbtune.h"
#include "dvb_settings.h"
#include "tuning_thread.h"
#include "dvbstream_thread.h"

const unsigned int MAX_GRADIANT = 6;

namespace Ui {
class tuning;
}

class tuning : public QDialog
{
	Q_OBJECT
public:
	explicit tuning(QWidget *parent = 0);
	~tuning();

	void init();
	void setup_demux();
	dvbtune *mytune;
	bool shutdown;

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
	void iqdraw(QVector<short int> x, QVector<short int> y);
	void delete_tuning();
	void on_pushButton_iqplot_clicked();
	
private:
	QSettings *mysettings;
	dvb_settings dvbnames;
	Ui::tuning *ui;
	QProcess myProcess;
	QVector<int> tree_pid;
	QVector<int> list_pid;
	QVector<QTreeWidgetItem *> tree_item;
	QVector<QListWidgetItem *> list_item;
	tuning_thread mythread;
	dvbstream_thread mystream;
	QwtPlotCurve *curve[MAX_GRADIANT];
	QwtPlotScaleItem *scaleX;
	QwtPlotScaleItem *scaleY;
	QwtSymbol *scatter_symbol[MAX_GRADIANT];
};

#endif // TUNING_H
