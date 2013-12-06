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
#include <iostream>
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
	void on_comboBox_lnb_currentIndexChanged();
	void on_comboBox_adapter_currentIndexChanged();
	void on_comboBox_diseqctype_currentIndexChanged();
	void on_comboBox_f_lof_currentIndexChanged();
	void on_pushButton_save_clicked();
	void on_pushButton_cancel_clicked();
	
private:
	Ui::settings *ui;
	QSettings *mysettings;
	bool nosave;
	int lnb, adp;
};

#endif // SETTINGS_H
