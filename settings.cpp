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

#include "settings.h"
#include "ui_settings.h"

settings::settings(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::settings)
{
	nosave = true;
	ui->setupUi(this);
	mysettings = new QSettings("UDL", "updateDVB");
	
	noload = true;
	for(int i = 0; i < MAX_LNBS; i++) {
		ui->comboBox_lnb->insertItem(i, QString::number(i));
	}
	noload = false;

	ui->comboBox_lnb->setCurrentIndex(0);
	load_settings();
	nosave = false;
}

settings::~settings()
{
	qDebug() << "~settings()";
	delete mysettings;
	delete ui;
}

void settings::load_settings()
{
	if (noload) {
		return;
	}
	
	save_settings();
	
	lnb = ui->comboBox_lnb->currentIndex();
	adp = ui->comboBox_adapter->currentIndex();

	ui->checkBox_diseqc_v12->setChecked(mysettings->value("adapter"+QString::number(adp)+"_diseqc_v12").toBool());
	ui->checkBox_diseqc_v13->setChecked(mysettings->value("adapter"+QString::number(adp)+"_diseqc_v13").toBool());

	ui->checkBox_enabled->setChecked(mysettings->value("lnb"+QString::number(lnb)+"_enabled").toBool());
	ui->checkBox_tone->setChecked(mysettings->value("lnb"+QString::number(lnb)+"_tone").toBool());
	ui->comboBox_voltage->setCurrentIndex(mysettings->value("lnb"+QString::number(lnb)+"_voltage").toInt());
	
	ui->comboBox_diseqctype->setCurrentIndex(mysettings->value("lnb"+QString::number(lnb)+"_diseqctype").toInt()+1);
	on_comboBox_diseqctype_currentIndexChanged(0);
	ui->comboBox_diseqcport->setCurrentIndex(mysettings->value("lnb"+QString::number(lnb)+"_diseqcport").toInt());
	
	int t_index = ui->comboBox_type->findText(mysettings->value("lnb"+QString::number(lnb)+"_type").toString());
	if (t_index >= 0) {
		ui->comboBox_type->setCurrentIndex(t_index);
	}
	
	ui->lineEdit_f_lof->setText(mysettings->value("lnb"+QString::number(lnb)+"_freqlof").toString());
	ui->lineEdit_f_start->setText(mysettings->value("lnb"+QString::number(lnb)+"_freqstart").toString());
	ui->lineEdit_f_stop->setText(mysettings->value("lnb"+QString::number(lnb)+"_freqstop").toString());

	if (mysettings->value("cmd_play").toString() != "") {
		ui->lineEdit_play->setText(mysettings->value("cmd_play").toString());
	} else {
		ui->lineEdit_play->setText("/usr/bin/mplayer /dev/dvb/adapter{}/dvr0");
	}

	ui->lineEdit_ipcleaner->setText(mysettings->value("cmd_ipcleaner").toString());

	ui->lineEdit_lat->setText(mysettings->value("site_lat").toString());
	ui->lineEdit_long->setText(mysettings->value("site_long").toString());	
}

void settings::save_settings()
{
	if (nosave) {
		return;
	}
	
	mysettings->setValue("adapter"+QString::number(adp)+"_diseqc_v12", ui->checkBox_diseqc_v12->isChecked());
	mysettings->setValue("adapter"+QString::number(adp)+"_diseqc_v13", ui->checkBox_diseqc_v13->isChecked());
	
	mysettings->setValue("lnb"+QString::number(lnb)+"_enabled", ui->checkBox_enabled->isChecked());
	mysettings->setValue("lnb"+QString::number(lnb)+"_tone", ui->checkBox_tone->isChecked());
	mysettings->setValue("lnb"+QString::number(lnb)+"_voltage", ui->comboBox_voltage->currentIndex());

	mysettings->setValue("lnb"+QString::number(lnb)+"_diseqctype", ui->comboBox_diseqctype->currentIndex()-1);
	mysettings->setValue("lnb"+QString::number(lnb)+"_diseqcport", ui->comboBox_diseqcport->currentIndex());
	
	mysettings->setValue("lnb"+QString::number(lnb)+"_type", ui->comboBox_type->currentText());

	mysettings->setValue("lnb"+QString::number(lnb)+"_freqlof", ui->lineEdit_f_lof->text().toInt());
	mysettings->setValue("lnb"+QString::number(lnb)+"_freqstart", ui->lineEdit_f_start->text().toInt());
	mysettings->setValue("lnb"+QString::number(lnb)+"_freqstop", ui->lineEdit_f_stop->text().toInt());

	mysettings->setValue("cmd_play", ui->lineEdit_play->text());
	mysettings->setValue("cmd_ipcleaner", ui->lineEdit_ipcleaner->text());

	mysettings->setValue("site_lat", ui->lineEdit_lat->text());
	mysettings->setValue("site_long", ui->lineEdit_long->text());

	lnb = ui->comboBox_lnb->currentIndex();
	adp = ui->comboBox_adapter->currentIndex();
}

void settings::on_comboBox_lnb_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	load_settings();
}

void settings::on_comboBox_adapter_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	load_settings();
}

void settings::on_comboBox_diseqctype_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	switch (ui->comboBox_diseqctype->currentIndex()) {
	case 1:
		ui->label_diseqcport->show();
		ui->comboBox_diseqcport->show();
		ui->comboBox_diseqcport->clear();
		ui->comboBox_diseqcport->insertItem(0, "");
		ui->comboBox_diseqcport->insertItem(1, "1");
		ui->comboBox_diseqcport->insertItem(2, "2");
		ui->comboBox_diseqcport->insertItem(3, "3");
		ui->comboBox_diseqcport->insertItem(4, "4");
		break;
	case 2:
		ui->label_diseqcport->show();
		ui->comboBox_diseqcport->show();
		ui->comboBox_diseqcport->clear();
		ui->comboBox_diseqcport->insertItem(0, "");
		ui->comboBox_diseqcport->insertItem(1, "1");
		ui->comboBox_diseqcport->insertItem(2, "2");
		ui->comboBox_diseqcport->insertItem(3, "3");
		ui->comboBox_diseqcport->insertItem(4, "4");
		ui->comboBox_diseqcport->insertItem(5, "5");
		ui->comboBox_diseqcport->insertItem(6, "6");
		ui->comboBox_diseqcport->insertItem(7, "7");
		ui->comboBox_diseqcport->insertItem(7, "8");
		break;
	case 0:
	default:
		ui->label_diseqcport->hide();
		ui->comboBox_diseqcport->hide();
		ui->comboBox_diseqcport->clear();
		break;
	}
}

void settings::on_comboBox_type_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	qDebug() << "on_comboBox_type_currentIndexChanged()" << ui->comboBox_type->currentText();
	
	if (ui->comboBox_type->currentText() == "C-Band") {
		ui->lineEdit_f_lof->setText("-5150");
		ui->lineEdit_f_start->setText("3700");
		ui->lineEdit_f_stop->setText("4200");
	}
		if (ui->comboBox_type->currentText() == "Ku Linear") {
		ui->lineEdit_f_lof->setText("10750");
		ui->lineEdit_f_start->setText("11700");
		ui->lineEdit_f_stop->setText("12200");
	}
	if (ui->comboBox_type->currentText() == "Ku Circular") {
		ui->lineEdit_f_lof->setText("11250");
		ui->lineEdit_f_start->setText("12200");
		ui->lineEdit_f_stop->setText("12700	");
	}
	if (ui->comboBox_type->currentText() == "Universal Low") {
		ui->lineEdit_f_lof->setText("9750");
		ui->lineEdit_f_start->setText("10700");
		ui->lineEdit_f_stop->setText("11200");
	}
	if (ui->comboBox_type->currentText() == "Universal High") {
		ui->lineEdit_f_lof->setText("10600");
		ui->lineEdit_f_start->setText("11550");
		ui->lineEdit_f_stop->setText("12050");
	}
	if (ui->comboBox_type->currentText() == "ATSC") {
		ui->lineEdit_f_lof->setText("0");
		ui->lineEdit_f_start->setText("52000");
		ui->lineEdit_f_stop->setText("700000");
	}
	if (ui->comboBox_type->currentText() == "QAM") {
		ui->lineEdit_f_lof->setText("0");
		ui->lineEdit_f_start->setText("52000");
		ui->lineEdit_f_stop->setText("900000");
	}
	if (ui->comboBox_type->currentText() == "Custom") {
		ui->lineEdit_f_lof->setText("0");
		ui->lineEdit_f_start->setText("950");
		ui->lineEdit_f_stop->setText("1450");
	}
}

void settings::on_pushButton_save_clicked()
{
	save_settings();
}

void settings::on_pushButton_close_clicked()
{
    this->close();
}
