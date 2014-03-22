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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDebug>
#include <QDialog>
#include <QSettings>
#include <QDir>
#include <QtSerialPort/QtSerialPort>
#include <iostream>
#include "dvb_settings.h"
using namespace std;

namespace Ui {
class settings;
}

class settings : public QDialog
{
	Q_OBJECT
	
public:
	explicit settings(QWidget *parent = 0);
	~settings();
	void load_settings();
	void save_settings();
	
private slots:
	void on_comboBox_lnb_currentIndexChanged(int index);
	void on_comboBox_adapter_currentIndexChanged(int index);
	void on_comboBox_type_currentIndexChanged(int index);
	void on_pushButton_save_clicked();
	void on_pushButton_close_clicked();
	void on_checkBox_diseqc_v13_clicked();
	void on_checkBox_diseqc_v12_clicked();
	void on_checkBox_asc1_clicked();
	void on_pushButton_asc1_upload_clicked();
	void on_pushButton_asc1_download_clicked();

private:
	Ui::settings *ui;
	QSettings *mysettings;
	bool noload;
	bool nosave;
	int lnb, adp;
};

#endif // SETTINGS_H
