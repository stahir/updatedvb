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

#include "blindscan_save.h"
#include "ui_blindscan_save.h"

blindscan_save::blindscan_save(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::blindscan_save)
{
	ui->setupUi(this);
	ui->lineEdit_filename->setText(QDir::currentPath() + "/dvb-scan.conf");

	mylayout = new QVBoxLayout(ui->widget);
    mystatus = new QStatusBar;
    mylayout->addWidget(mystatus);
	mylayout->setMargin(0);
	mylayout->setSpacing(0);
}

blindscan_save::~blindscan_save()
{
	qDebug() << "~blindscan_save()";

	delete ui;
}

void blindscan_save::on_pushButton_save_clicked()
{
	QFile *out_fd = new QFile(ui->lineEdit_filename->text());
	if (!out_fd->open(QIODevice::WriteOnly | QIODevice::Text)) {
		 return;
	}
	QTextStream out(out_fd);

	for(int i = 0; i < mytp_info.size(); i++) {
		if (mytp_info[i].system == SYS_DVBS2) {
			out << "S2 ";
		} else {
			out << "S ";
		}
		out << QString::number(mytp_info[i].frequency*1000);
		out << dvbnames.voltage[mytp_info[i].voltage];
		out << QString::number(mytp_info[i].symbolrate*1000) << " ";
		out << dvbnames.fec[mytp_info[i].fec] << " ";
		out << dvbnames.rolloff[mytp_info[i].rolloff] << " ";
		out << dvbnames.modulation[mytp_info[i].modulation] << "\n";
	}
	mystatus->showMessage("Saved");
	sleep(2);
	emit QDialog::accept();
}

void blindscan_save::on_pushButton_cancel_clicked()
{
	emit QDialog::accept();
}
