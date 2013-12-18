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

#ifndef BLINDSCAN_SAVE_H
#define BLINDSCAN_SAVE_H

#include <QDialog>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QDebug>
#include "dvb_settings.h"
#include <unistd.h>

namespace Ui {
class blindscan_save;
}

class blindscan_save : public QDialog
{
	Q_OBJECT
	
public:
	explicit blindscan_save(QWidget *parent = 0);
	~blindscan_save();

	QVector<tp_info> mytp_info;
	
private slots:
	void on_pushButton_save_clicked();
	void on_pushButton_cancel_clicked();
	
private:
	Ui::blindscan_save *ui;
	dvb_settings dvbnames;
	QStatusBar *mystatus;
};

#endif // BLINDSCAN_SAVE_H
