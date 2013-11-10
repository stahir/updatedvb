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

#ifndef BLINDSCAN_H
#define BLINDSCAN_H

#include <qglobal.h>
#if QT_VERSION >= 0x050000
        #include <QtWidgets>
#else
        #include <QtGui>
#endif

#include <QDialog>
#include <QtCore>
#include <QProcess>
#include <QDebug>
#include "dvbtune.h"
#include "tuning.h"
#include "dvb_settings.h"
#include "blindscan_save.h"
#include "blindscan_thread.h"

namespace Ui {
class blindscan;
}

class blindscan : public QDialog
{
	Q_OBJECT	
public:
	explicit blindscan(QWidget *parent = 0);
	~blindscan();
	void init();
	void scan();
	void smartscan();
	
	dvbtune *mytune;	
	tuning *tuningdialog;
	
private slots:
	void updatesignal();
	void on_pushButton_tune_clicked();	
	void on_pushButton_save_clicked();
	void on_pushButton_expand_clicked();
	void on_pushButton_unexpand_clicked();
	void updateprogress(int i);
	
private:
	Ui::blindscan *ui;

	int tree_create_root(QString text);
	int tree_create_child(int parent, QString text);

	blindscan_thread mythread;
	int pindex;
	int cindex;
	QTreeWidgetItem *ptree[65535];
	QTreeWidgetItem *ctree[65535];
	dvb_settings dvbnames;
	QVector<tp_info> mytp_info;	
	QStatusBar *mystatus;
	QProgressBar *myprogress;
	QVBoxLayout *mylayout;
};

#endif // BLINDSCAN_H
