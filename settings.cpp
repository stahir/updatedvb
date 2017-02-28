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
	
	mystatusbar = new QStatusBar;
	mystatusbar->setVisible(true);
	ui->verticalLayout->addWidget(mystatusbar);

	connect(&status_mapper, SIGNAL(mapped(QString)), this, SLOT(update_status(QString)));
}

settings::~settings()
{
	delete mysettings;
	delete mystatusbar;
	delete ui;
}

void settings::startup()
{
	noload = true;
	QVector<int> adaps;
	QDir dvb_dir("/dev/dvb");
	dvb_dir.setFilter(QDir::Dirs|QDir::NoDotAndDotDot);
	QStringList adapter_entries = dvb_dir.entryList();
	adapter_entries = adapter_entries.filter("adapter");
	for(int i = 0; i < adapter_entries.size(); i++) {
		QString adapter_name = adapter_entries.at(i);
		adapter_name.replace("adapter", "");
		adaps.append(adapter_name.toInt());
	}
	qSort(adaps);
	for (int i = 0; i < adaps.size(); i++) {
		if (!mysettings->value("adapter"+QString::number(adaps.at(i))+"_name").toString().isEmpty()) {
			ui->comboBox_adapter->insertItem(adaps.at(i), QString("%1 - %2").arg(adaps.at(i)).arg(mysettings->value("adapter"+QString::number(adaps.at(i))+"_name").toString()), adaps.at(i));
		} else {
			ui->comboBox_adapter->insertItem(adaps.at(i), QString("%1").arg(adaps.at(i)), adaps.at(i));
		}
	}
	for (int i = 0; i < MAX_LNBS; i++) {
		if (!mysettings->value("lnb"+QString::number(i)+"_name").toString().isEmpty()) {
			ui->comboBox_lnb->insertItem(i, QString("%1 - %2").arg(i).arg(mysettings->value("lnb"+QString::number(i)+"_name").toString()) , i);
			ui->comboBox_default_lnb->insertItem(i, QString("%1 - %2").arg(i).arg(mysettings->value("lnb"+QString::number(i)+"_name").toString()) , i);
		} else {
			ui->comboBox_lnb->insertItem(i, QString("%1").arg(i) , i);
			ui->comboBox_default_lnb->insertItem(i, QString("%1").arg(i) , i);
		}
	}

	noload = false;

	ui->comboBox_lnb->setCurrentIndex(0);
	ui->progressBar->hide();
	load_settings();
	nosave = false;
}

void settings::load_settings()
{
	if (noload) {
		return;
	}
	
	save_settings();
	
	lnb = ui->comboBox_lnb->currentIndex();
	adp = ui->comboBox_adapter->currentData().toInt();
	fnd = ui->comboBox_frontend->currentData().toInt();

	ui->lineEdit_lat->setText(mysettings->value("site_lat").toString());
	ui->lineEdit_long->setText(mysettings->value("site_long").toString());
	ui->lineEdit_asc1_serialport->setText(mysettings->value("asc1_serialport").toString());

	ui->lineEdit_loop_delay->setText(QString::number(mysettings->value("adapter"+QString::number(adp)+"_loop_delay").toInt()));
	ui->checkBox_save_images->setChecked(mysettings->value("adapter"+QString::number(adp)+"_save_images").toBool());

	ui->checkBox_diseqc_v13->setChecked(mysettings->value("adapter"+QString::number(adp)+"_diseqc_v13").toBool());
	on_checkBox_diseqc_v13_clicked();
	ui->checkBox_diseqc_v12->setChecked(mysettings->value("adapter"+QString::number(adp)+"_diseqc_v12").toBool());
	on_checkBox_diseqc_v12_clicked();

	ui->checkBox_servo->setChecked(mysettings->value("adapter"+QString::number(adp)+"_servo").toBool());
	ui->checkBox_asc1->setChecked(mysettings->value("adapter"+QString::number(adp)+"_asc1").toBool());
	if (mysettings->value("adapter"+QString::number(adp)+"_name").toString() != "") {
		ui->lineEdit_adapter_name->setText(mysettings->value("adapter"+QString::number(adp)+"_name").toString());
	} else {
		if (mytuners.size() > ui->comboBox_adapter->currentIndex()) {
			ui->lineEdit_adapter_name->setText(mytuners.at(ui->comboBox_adapter->currentIndex())->name);
		}
	}

	on_checkBox_asc1_clicked();
	for (int i = 1; i < 256; i++) {
		ui->tableWidget_diseqc_v12->setItem(i-1, 0, new QTableWidgetItem(mysettings->value("adapter"+QString::number(adp)+"_diseqc_v12_name_"+QString::number(i)).toString()));
		ui->tableWidget_diseqc_v12->setItem(i-1, 1, new QTableWidgetItem(mysettings->value("adapter"+QString::number(adp)+"_diseqc_v12_pos_"+QString::number(i)).toString()));
		ui->tableWidget_diseqc_v12->setItem(i-1, 2, new QTableWidgetItem(""));
		ui->tableWidget_diseqc_v12->setItem(i-1, 3, new QTableWidgetItem(""));
		ui->tableWidget_diseqc_v12->setItem(i-1, 4, new QTableWidgetItem(""));
	}

	ui->lineEdit_lnb_name->setText(mysettings->value("lnb"+QString::number(lnb)+"_name").toString());
	ui->checkBox_enabled->setChecked(mysettings->value("lnb"+QString::number(lnb)+"_enabled").toBool());
	ui->checkBox_tone->setChecked(mysettings->value("lnb"+QString::number(lnb)+"_tone").toBool());
	ui->comboBox_voltage->setCurrentIndex(mysettings->value("lnb"+QString::number(lnb)+"_voltage").toInt());
	
	ui->comboBox_committed->setCurrentIndex(mysettings->value("lnb"+QString::number(lnb)+"_committed").toInt());
	ui->comboBox_uncommitted->setCurrentIndex(mysettings->value("lnb"+QString::number(lnb)+"_uncommitted").toInt());

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
		ui->lineEdit_play->setText("/usr/bin/mpv /dev/dvb/adapter{}/dvr0");
	}

	if (mytuners.size() > ui->comboBox_adapter->currentIndex()) {
		update_status(mytuners.at(ui->comboBox_adapter->currentIndex())->name, STATUS_NOEXP);
        if (mytuners.at(ui->comboBox_adapter->currentIndex())->extended_caps) {
			ui->label_blindscan_step->show();
			ui->lineEdit_blindscan_step->show();
			if (mysettings->value("adapter"+QString::number(adp)+"_blindscan_step").toString() != "") {
				ui->lineEdit_blindscan_step->setText(mysettings->value("adapter"+QString::number(adp)+"_blindscan_step").toString());
			} else {
				ui->lineEdit_blindscan_step->setText("18");
			}
		} else {
			ui->label_blindscan_step->hide();
			ui->lineEdit_blindscan_step->hide();
		}
	}

	ui->comboBox_default_lnb->setCurrentIndex(mysettings->value("adapter"+QString::number(adp)+"_default_lnb").toInt());

	if (mysettings->value("motor_delay").toUInt() != 0) {
		ui->lineEdit_motor_delay->setText(mysettings->value("motor_delay").toString());
	} else {
		ui->lineEdit_motor_delay->setText("500");
	}

}

void settings::save_settings()
{
	if (nosave) {
		return;
	}
	
	mysettings->setValue("adapter"+QString::number(adp)+"_diseqc_v12", ui->checkBox_diseqc_v12->isChecked());
	mysettings->setValue("adapter"+QString::number(adp)+"_diseqc_v13", ui->checkBox_diseqc_v13->isChecked());
	mysettings->setValue("adapter"+QString::number(adp)+"_name", ui->lineEdit_adapter_name->text());
	mysettings->setValue("adapter"+QString::number(adp)+"_frontend"+QString::number(fnd)+"_name", ui->lineEdit_frontend_name->text());
	mysettings->setValue("adapter"+QString::number(adp)+"_default_lnb", ui->comboBox_default_lnb->currentIndex());
	mysettings->setValue("adapter"+QString::number(adp)+"_loop_delay", ui->lineEdit_loop_delay->text());
	mysettings->setValue("adapter"+QString::number(adp)+"_save_images", ui->checkBox_save_images->isChecked());

	for (int i = 1; i < 100; i++) {
		mysettings->setValue("adapter"+QString::number(adp)+"_diseqc_v12_name_"+QString::number(i), ui->tableWidget_diseqc_v12->item(i-1, 0)->text());
		mysettings->setValue("adapter"+QString::number(adp)+"_diseqc_v12_pos_"+QString::number(i), ui->tableWidget_diseqc_v12->item(i-1, 1)->text());
	}

	mysettings->setValue("lnb"+QString::number(lnb)+"_name", ui->lineEdit_lnb_name->text());
	mysettings->setValue("lnb"+QString::number(lnb)+"_enabled", ui->checkBox_enabled->isChecked());
	mysettings->setValue("lnb"+QString::number(lnb)+"_tone", ui->checkBox_tone->isChecked());
	mysettings->setValue("lnb"+QString::number(lnb)+"_voltage", ui->comboBox_voltage->currentIndex());

	mysettings->setValue("lnb"+QString::number(lnb)+"_committed", ui->comboBox_committed->currentIndex());
	mysettings->setValue("lnb"+QString::number(lnb)+"_uncommitted", ui->comboBox_uncommitted->currentIndex());
	
	mysettings->setValue("lnb"+QString::number(lnb)+"_type", ui->comboBox_type->currentText());

	mysettings->setValue("lnb"+QString::number(lnb)+"_freqlof", ui->lineEdit_f_lof->text().toInt());
	mysettings->setValue("lnb"+QString::number(lnb)+"_freqstart", ui->lineEdit_f_start->text().toInt());
	mysettings->setValue("lnb"+QString::number(lnb)+"_freqstop", ui->lineEdit_f_stop->text().toInt());

	mysettings->setValue("cmd_play", ui->lineEdit_play->text());

	mysettings->setValue("adapter"+QString::number(adp)+"_blindscan_step", ui->lineEdit_blindscan_step->text());

	mysettings->setValue("site_lat", ui->lineEdit_lat->text());
	mysettings->setValue("site_long", ui->lineEdit_long->text());
	mysettings->setValue("asc1_serialport", ui->lineEdit_asc1_serialport->text());

	mysettings->setValue("adapter"+QString::number(adp)+"_asc1", ui->checkBox_asc1->isChecked());
	mysettings->setValue("adapter"+QString::number(adp)+"_servo", ui->checkBox_servo->isChecked());

	mysettings->sync();

	lnb = ui->comboBox_lnb->currentIndex();
	adp = ui->comboBox_adapter->currentData().toInt();

	for (int i = 0; i < MAX_LNBS; i++) {
		if (!mysettings->value("lnb"+QString::number(i)+"_name").toString().isEmpty()) {
			ui->comboBox_lnb->setItemText(i, QString("%1 - %2").arg(i).arg(mysettings->value("lnb"+QString::number(i)+"_name").toString()));
			ui->comboBox_default_lnb->setItemText(i, QString("%1 - %2").arg(i).arg(mysettings->value("lnb"+QString::number(i)+"_name").toString()));
		} else {
			ui->comboBox_lnb->setItemText(i, QString("%1").arg(i));
			ui->comboBox_default_lnb->setItemText(i, QString("%1").arg(i));
		}
	}
	ui->comboBox_default_lnb->setCurrentIndex(mysettings->value("adapter"+QString::number(adp)+"_default_lnb").toInt());

	if (!mysettings->value("adapter"+QString::number(adp)+"_name").toString().isEmpty()) {
		ui->comboBox_adapter->setItemText(ui->comboBox_adapter->currentIndex(), QString("%1 - %2").arg(adp).arg(mysettings->value("adapter"+QString::number(adp)+"_name").toString()));
	} else {
		ui->comboBox_adapter->setItemText(ui->comboBox_adapter->currentIndex(), QString("%1").arg(adp));
	}

	mysettings->setValue("motor_delay", ui->lineEdit_motor_delay->text());

	update_status("Saved", 2);
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

	ui->comboBox_frontend->clear();
	QDir adapter_dir("/dev/dvb/adapter" + ui->comboBox_adapter->currentData().toString());
	adapter_dir.setFilter(QDir::System|QDir::NoDotAndDotDot);
	QStringList frontend_entries = adapter_dir.entryList();
	frontend_entries = frontend_entries.filter("frontend");
	for(int i = 0; i < frontend_entries.size(); i++) {
		QString frontend_name = frontend_entries.at(i);
		frontend_name.replace("frontend", "");
		ui->comboBox_frontend->insertItem(frontend_name.toInt(), frontend_name, frontend_name.toInt());
	}
}

void settings::on_comboBox_type_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	
	if (ui->comboBox_type->currentText() == "C-Band") {
		ui->lineEdit_f_lof->setText("-5150");
		ui->lineEdit_f_start->setText("3700");
		ui->lineEdit_f_stop->setText("4200");
	}
	if (ui->comboBox_type->currentText() == "C-Band: Extended") {
		ui->lineEdit_f_lof->setText("-5150");
		ui->lineEdit_f_start->setText("3200");
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
	if (ui->comboBox_type->currentText() == "Universal Low: Extended") {
		ui->lineEdit_f_lof->setText("9750");
		ui->lineEdit_f_start->setText("10700");
		ui->lineEdit_f_stop->setText("11700");
	}
	if (ui->comboBox_type->currentText() == "Universal High: Extended") {
		ui->lineEdit_f_lof->setText("10600");
		ui->lineEdit_f_start->setText("11700");
		ui->lineEdit_f_stop->setText("12750");
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
	if (ui->comboBox_type->currentText() == "DVBT") {
		ui->lineEdit_f_lof->setText("0");
		ui->lineEdit_f_start->setText("174000");
		ui->lineEdit_f_stop->setText("790000");
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

void settings::on_checkBox_diseqc_v13_clicked()
{
	if (ui->checkBox_diseqc_v13->isChecked()) {
		ui->label_lat->show();
		ui->lineEdit_lat->show();
		ui->label_long->show();
		ui->lineEdit_long->show();
	} else {
		ui->label_lat->hide();
		ui->lineEdit_lat->hide();
		ui->label_long->hide();
		ui->lineEdit_long->hide();
	}
	if (ui->checkBox_diseqc_v12->isChecked() || ui->checkBox_diseqc_v13->isChecked()) {
		ui->label_motor_delay->show();
		ui->lineEdit_motor_delay->show();
	} else {
		ui->label_motor_delay->hide();
		ui->lineEdit_motor_delay->hide();
	}
}

void settings::on_checkBox_diseqc_v12_clicked()
{
	if (ui->checkBox_diseqc_v12->isChecked()) {
		ui->tableWidget_diseqc_v12->show();
		ui->tableWidget_diseqc_v12->setColumnCount(2);
		ui->tableWidget_diseqc_v12->setColumnWidth(0, 175);
		ui->tableWidget_diseqc_v12->setColumnWidth(1, 90);
		ui->checkBox_asc1->show();
		on_checkBox_asc1_clicked();
	} else {
		ui->tableWidget_diseqc_v12->hide();
		ui->checkBox_asc1->hide();
		on_checkBox_asc1_clicked();
	}
	if (ui->checkBox_diseqc_v12->isChecked() || ui->checkBox_diseqc_v13->isChecked()) {
		ui->label_motor_delay->show();
		ui->lineEdit_motor_delay->show();
	} else {
		ui->label_motor_delay->hide();
		ui->lineEdit_motor_delay->hide();
	}
}

void settings::on_checkBox_asc1_clicked()
{
	if (ui->checkBox_asc1->isChecked() && ui->checkBox_diseqc_v12->isChecked()) {
		ui->gridWidget_asc1->show();
		ui->tableWidget_diseqc_v12->setColumnCount(5);
		ui->tableWidget_diseqc_v12->setColumnWidth(0, 175);
		ui->tableWidget_diseqc_v12->setColumnWidth(1, 90);
		ui->tableWidget_diseqc_v12->setColumnWidth(2, 50);
		ui->tableWidget_diseqc_v12->setColumnWidth(3, 50);
		ui->tableWidget_diseqc_v12->setColumnWidth(4, 50);
	} else {
		ui->gridWidget_asc1->hide();
		ui->tableWidget_diseqc_v12->setColumnCount(2);
		ui->tableWidget_diseqc_v12->setRowCount(256);
	}
}

void settings::on_pushButton_asc1_upload_clicked()
{
	QSerialPort myserial;
	QByteArray cmd;
	QByteArray data;
	QTime t;
	t.start();

	myserial.setPortName(ui->lineEdit_asc1_serialport->text());
	myserial.open(QIODevice::ReadWrite);
	myserial.setBaudRate(QSerialPort::Baud57600);
	myserial.setDataBits(QSerialPort::Data8);
	myserial.setParity(QSerialPort::NoParity);
	myserial.setStopBits(QSerialPort::OneStop);
	myserial.setFlowControl(QSerialPort::NoFlowControl);

	if (!myserial.isOpen()) {
		qDebug() << "unable to open";
		return;
	}

	cmd.clear();
	cmd.append(0x4a);
	myserial.write(cmd);
	myserial.waitForBytesWritten(1000);

	t.restart();
	data.clear();
	myserial.waitForReadyRead(1000);
	data += myserial.readAll();
	while (data.isEmpty() || (data.at(0) != 0x4b && t.elapsed() < 1000)) {
		QThread::msleep(10);
		myserial.waitForReadyRead(1000);
		data += myserial.readAll();
	}
	if (data.at(0) != 0x4b) {
		qDebug() << "timeout...";
		return;
	}

	ui->progressBar->show();
	for (int i = 1; i < 100; i++) {
		cmd.clear();
		cmd.append((char)0x4c);
		cmd.append((char)i);
		QString tmp = ui->tableWidget_diseqc_v12->item(i-1, 0)->text();
		tmp += QString(16, 0x00);
		tmp.resize(16);
		cmd.append(tmp);
		cmd.append((char)0x00);
		cmd.append((char)(ui->tableWidget_diseqc_v12->item(i-1, 1)->text().toUInt() & 0xff));
		cmd.append((char)((ui->tableWidget_diseqc_v12->item(i-1, 1)->text().toUInt() >> 8) & 0xff));
		cmd.append((char)ui->tableWidget_diseqc_v12->item(i-1, 2)->text().toShort());
		cmd.append((char)ui->tableWidget_diseqc_v12->item(i-1, 3)->text().toShort());

		myserial.write(cmd);
		myserial.flush();
		myserial.waitForBytesWritten(1000);

		t.restart();
		data.clear();
		myserial.waitForReadyRead(1000);
		data += myserial.readAll();
		while (data.isEmpty() || (data.at(0) != 0x4b && t.elapsed() < 1000)) {
			QThread::msleep(10);
			myserial.waitForReadyRead(1000);
			data += myserial.readAll();
		}
		if (data.at(0) != 0x4b) {
			qDebug() << "timeout...";
			continue;
		}

		ui->progressBar->setValue(i);
	}
	ui->progressBar->hide();

	myserial.close();
}

void settings::on_pushButton_asc1_download_clicked()
{
	asc1_data mydata;
	QSerialPort myserial;
	QByteArray cmd;
	QByteArray data;
	QTime t;
	t.start();

	myserial.setPortName(ui->lineEdit_asc1_serialport->text());
	myserial.open(QIODevice::ReadWrite);
	myserial.setBaudRate(QSerialPort::Baud57600);
	myserial.setDataBits(QSerialPort::Data8);
	myserial.setParity(QSerialPort::NoParity);
	myserial.setStopBits(QSerialPort::OneStop);
	myserial.setFlowControl(QSerialPort::NoFlowControl);

	if (!myserial.isOpen()) {
		qDebug() << "unable to open";
		return;
	}

	cmd.clear();
	cmd.append(0x48);
	myserial.write(cmd);
	myserial.flush();
	myserial.waitForBytesWritten(1000);

	data.clear();
	myserial.waitForReadyRead(1000);
	data += myserial.readAll();

	ui->progressBar->show();
	for (int i = 1; i < 100; i++) {
		QThread::msleep(5);

		cmd.clear();
		cmd.append(0x49);
		myserial.write(cmd);
		myserial.flush();
		myserial.waitForBytesWritten(1000);

		t.restart();

		data.clear();
		myserial.waitForReadyRead(1000);
		data += myserial.readAll();
		while (data.size() < 23 && t.elapsed() < 1000) {
			myserial.waitForReadyRead(1000);
			data += myserial.readAll();
		}

		if (data.size() < 23) {
			qDebug() << "timeout..." << t.elapsed();
			continue;
		}

		mydata.name		= data.mid(2, 16).data();
		mydata.counter	= (u_int8_t)data.mid(19, 1).at(0) << 8 | (u_int8_t)data.mid(20, 1).at(0);
		mydata.Hdeg		= (int8_t)data.mid(21, 1).at(0);
		mydata.Vdeg		= (int8_t)data.mid(22, 1).at(0);

		ui->tableWidget_diseqc_v12->setItem(i-1, 0, new QTableWidgetItem(mydata.name));
		ui->tableWidget_diseqc_v12->setItem(i-1, 1, new QTableWidgetItem(QString::number(mydata.counter)));
		ui->tableWidget_diseqc_v12->setItem(i-1, 2, new QTableWidgetItem(QString::number(mydata.Hdeg)));
		ui->tableWidget_diseqc_v12->setItem(i-1, 3, new QTableWidgetItem(QString::number(mydata.Vdeg)));

		ui->progressBar->setValue(i);
	}
	ui->progressBar->hide();

	myserial.close();
}

void settings::update_status(QString text, int time)
{
	if (time == STATUS_CLEAR) {
		mystatus.clear();
	}
	if (time == STATUS_REMOVE) {
		if (mystatus.contains(text)) {
			mystatus.remove(mystatus.indexOf(text));
		}
	}
	if (time == STATUS_NOEXP) {
		mystatus.append(text);
	}
	if (time > 0) {
		status_timer = new QTimer;
		status_timer->setSingleShot(true);
		connect(status_timer, SIGNAL(timeout()), &status_mapper, SLOT(map()));
		status_mapper.setMapping(status_timer, text);
		status_timer->start(time*1000);
		mystatus.append(text);
	}

	if (mystatus.size()) {
		mystatusbar->showMessage(mystatus.last(), 0);
	} else {
		mystatusbar->showMessage("", 0);
	}
}

void settings::on_comboBox_frontend_currentIndexChanged(int index)
{
	load_settings();

	if (adp < 0) {
		adp = ui->comboBox_adapter->currentData().toInt();
	}

	ui->lineEdit_frontend_name->setText(mysettings->value("adapter"+QString::number(adp)+"_frontend"+QString::number(index)+"_name").toString());

	if (mytuners.size() > ui->comboBox_adapter->currentIndex()) {
		mytuners.at(ui->comboBox_adapter->currentIndex())->closefd();
		mytuners.at(ui->comboBox_adapter->currentIndex())->frontend	= ui->comboBox_frontend->currentData().toInt();
		mytuners.at(ui->comboBox_adapter->currentIndex())->getops();
		update_status(mytuners.at(ui->comboBox_adapter->currentIndex())->name, STATUS_NOEXP);
	}
}

void settings::on_pushButton_play_browse_clicked()
{
	QString ppath = ui->lineEdit_play->text().section(' ', 0, 0);
	if (ppath.isEmpty()) {
		ppath = QFileDialog::getOpenFileName(this, "Select Media Player", "/usr/bin");
	} else {
		ppath = QFileDialog::getOpenFileName(this, "Select Media Player", ppath);
	}
	if (ppath.isEmpty()) {
		return;
	}
	ui->lineEdit_play->setText(ppath + " /dev/dvb/adapter{}/dvr0");
}
