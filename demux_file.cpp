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

#include "demux_file.h"
#include "ui_demux_file.h"

demux_file::demux_file(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::demux_file)
{
	ui->setupUi(this);
	connect(this, SIGNAL(finished(int)), this, SLOT(on_pushButton_stop_clicked()));

	mystatus = new QStatusBar;
	ui->verticalLayout->addWidget(mystatus);
	mystatus->setVisible(true);
	ui->lineEdit_filename->setText(QDir::currentPath() + "/test.ts");
}

demux_file::~demux_file()
{
	qDebug() << "~demux_file()";

	delete mystatus;
	delete ui;
}

void demux_file::init()
{
	connect(mytune, SIGNAL(demux_status(int)), this, SLOT(demux_status(int)));
}

void demux_file::on_pushButton_start_clicked()
{
	bytes_wrote = 0;
	mytune->close_dvr();
	mytune->out_name = ui->lineEdit_filename->text();
	mytune->loop = true;
	mytune->thread_function.append("demux_file");
	mytune->start();
}

void demux_file::on_pushButton_stop_clicked()
{
	if (mytune->thread_function.indexOf("demux_file") != -1)
		mytune->thread_function.remove(mytune->thread_function.indexOf("demux_file"));
	mytune->close_dvr();	
}

void demux_file::demux_status(int bytes)
{
	bytes_wrote += bytes;
	mystatus->showMessage(QString("%L1 KB").arg(bytes_wrote/1000), 0);
}
