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

#include "tuning.h"
#include "ui_tuning.h"

tuning::tuning(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::tuning)
{
	ui->setupUi(this);
	setWindowFlags(Qt::WindowStaysOnTopHint);

	mysettings = new QSettings("UDL", "updateDVB");
	ui->listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);	
		
	connect(this, SIGNAL(finished(int)), this, SLOT(delete_tuning()));
	
	shutdown = false;
}

tuning::~tuning()
{
	qDebug() << "~tuning()";
	
	mystream.loop = false;
	mystream.quit();
	mystream.wait(1000);
	while (mystream.isRunning()) {
		qDebug() << "mystream.isRunning()";
		mystream.loop = false;
		sleep(1);
	}

	mythread.ready	= true;
	mythread.loop	= false;
	mythread.quit();
	mythread.wait(1000);
	while (mythread.isRunning()) {
		qDebug() << "mythread.isRunning()";
		mythread.ready	= true;
		mythread.loop	= false;
		sleep(1);
	}

	mytune->loop = false;
	mytune->quit();
	mytune->wait(1000);
	while (mytune->isRunning()) {
		qDebug() << "mytune->isRunning()";
		mytune->loop = false;
		sleep(1);
	}
	
	delete mysettings;
	delete ui;

	shutdown = true;
	
	qDebug() << "~tuning() done";
}

void tuning::delete_tuning()
{
	qDebug() << "delete_tuning()";
	this->deleteLater();
}

void tuning::init()
{
	connect(&myProcess, SIGNAL(finished(int)), this, SLOT(stop_demux()));
	connect(mytune, SIGNAL(updatesignal()), this, SLOT(updatesignal()));
	connect(mytune, SIGNAL(updateresults()), this, SLOT(updateresults()));
	connect(&mythread, SIGNAL(list_create(QString, int)), this, SLOT(list_create(QString, int)));
	connect(&mythread, SIGNAL(tree_create_root(int *, QString, int)), this, SLOT(tree_create_root(int *, QString, int)));
	connect(&mythread, SIGNAL(tree_create_child(int *, QString, int)), this, SLOT(tree_create_child(int *, QString, int)));
	connect(&mythread, SIGNAL(setcolor(int,QColor)), this, SLOT(setcolor(int,QColor)));

	mytune->start();
	mytune->thread_function.append("tune");
	mythread.mytune = mytune;
	
	this->setWindowTitle("Tuning Adapter " + QString::number(mytune->adapter) + ", Frontend " + QString::number(mytune->frontend) + " : " + mytune->name);

	if (!(mytune->caps & FE_CAN_IQ)) {
		ui->pushButton_iqplot->hide();
		ui->label_iqplot->hide();
	}
}

void tuning::on_treeWidget_itemClicked(QTreeWidgetItem * item, int column)
{
	Q_UNUSED(item);
	Q_UNUSED(column);
	
	// If mythread is running, dont click on tree items
	if (mythread.loop) {
		return;
	}
	qDebug() << "on_treeWidget_itemClicked()";

	for(int a = 0; a < list_item.size(); a++) {
		list_item.at(a)->setSelected(false);
	}

	int pid;
	for(int a = 0; a < tree_item.size(); a++) {
		if (tree_item.at(a)->isSelected()) {
			pid = tree_pid.at(a);
			if (pid < 0)
				continue;
			list_item.at(list_pid.indexOf(tree_pid.at(a)))->setSelected(true);
		}
	}
}

void tuning::on_listWidget_itemClicked(QListWidgetItem *item)
{
	Q_UNUSED(item);
    
	// If mythread is running, dont click on tree items
	if (mythread.loop) {
		return;
	}
	
	for(int a = 0; a < tree_item.size(); a++) {
		tree_item.at(a)->setSelected(false);
	}

	int pid;
	for (int a = 0; a < list_item.size(); a++) {
		if (list_item.at(a)->isSelected()) {
			pid = list_pid.at(a);
			if (pid < 0)
				continue;
			int i = 0;
			while (tree_pid.indexOf(list_pid.at(a), i) != -1) {
				i = tree_pid.indexOf(list_pid.at(a), i);
				tree_item.at(i)->setSelected(true);
				i++;
			}
		}
	}
}

void tuning::updatesignal()
{
	if (mytune->tp.status & FE_HAS_LOCK) {
		ui->label_lock->setText("Locked");
		ui->label_lock->setStyleSheet("QLabel { color : green; }");
	} else {
		ui->label_lock->setText("Unlocked");
		ui->label_lock->setStyleSheet("QLabel { color : red; }");
		if (isSatellite(mytune->tp.system) && mytune->tp.fec == FEC_AUTO) {
			ui->label_frequency->setText("");
			return;
		}
	}
	ui->label_signalS->setText(QString::number(mytune->tp.lvl) + "%");
	ui->label_signalQ->setText(QString::number(mytune->tp.snr, 'f', 1) + "dB");
	ui->label_ber->setText(QString::number(mytune->tp.ber));
	ui->label_system->setText(dvbnames.system[mytune->tp.system]);
	ui->label_modulation->setText(dvbnames.modulation[mytune->tp.modulation]);

	if (isSatellite(mytune->tp.system)) {
		ui->label_frequency->setText(QString::number(mytune->tp.frequency) + dvbnames.voltage[mytune->tp.voltage] + QString::number(mytune->tp.symbolrate));
		ui->label_fec->setText(dvbnames.fec[mytune->tp.fec]);
		ui->label_inversion->setText(dvbnames.inversion[mytune->tp.inversion]);
		ui->label_rolloff->setText(dvbnames.rolloff[mytune->tp.rolloff]);
		ui->label_pilot->setText(dvbnames.pilot[mytune->tp.pilot]);
		if (mytune->tune_ops.mis >= 0) {
			ui->label_mis->setText("True, filter = " + QString::number(mytune->tune_ops.mis));
		} else {
			ui->label_mis->setText("false");
		}
	
		if (!mytune->tp.matype) {
			return;
		}
	
		unsigned int matype = mytune->tp.matype >> 8;
		switch(mytune->maskbits(matype, 0xC0)) {
		case 0:
			ui->label_tsgs->setText("Generic Packetized");
			break;
		case 1:
			ui->label_tsgs->setText("Generic Continuous");
			break;
		case 2:
			ui->label_tsgs->setText("Reserved");
			break;
		case 3:
			ui->label_tsgs->setText("Transport Stream");
			break;
		}
		switch(mytune->maskbits(matype, 0x20)) {
		case 0:
			ui->label_sismis->setText("Multiple Input Stream");
			break;
		case 1:
			ui->label_sismis->setText("Single Input Stream");
			break;
		}
		switch(mytune->maskbits(matype, 0x10)) {
		case 0:
			ui->label_ccmacm->setText("ACM/VCM");
			break;
		case 1:
			ui->label_ccmacm->setText("CCM");
			break;
		}
	} else {
		qam myqam;
		atsc myatsc;
		switch (mytune->tp.system) {
		case SYS_ATSC:
		case SYS_ATSCMH:
			ui->label_frequency->setText(QString::number(mytune->tp.frequency/1000) + "mhz, Channel " + QString::number(myatsc.ch[myatsc.freq.indexOf(mytune->tp.frequency)]));			
			break;
		case SYS_DVBC_ANNEX_B:
			ui->label_frequency->setText(QString::number(mytune->tp.frequency/1000) + "mhz, Channel " + QString::number(myqam.ch[myqam.freq.indexOf(mytune->tp.frequency)]));			
			break;
		default:
			ui->label_frequency->setText(QString::number(mytune->tp.frequency/1000) + "mhz, Channel ");			
		}
	}
}

void tuning::list_create(QString text, int pid)
{
	list_pid.append(pid);
	list_item.append(new QListWidgetItem(ui->listWidget));
	list_item.last()->setText(text);
	if (pid == 0x00 || pid == 0x11 || pid == 0x1fff) {
		list_item.last()->setTextColor(QColor(Qt::green).darker(300));
	}
	if (tree_pid.indexOf(pid) != -1) {
		list_item.last()->setTextColor(tree_item.at(tree_pid.indexOf(pid))->textColor(0));
	}
}

void tuning::tree_create_root(int *parent, QString text, int pid)
{
	if (pid >= 0 && mytune->pids_rate.at(pid) == 0) {
		mytune->pids_rate[pid] = 1;
	}
	tree_pid.append(pid);
	tree_item.append(new QTreeWidgetItem(ui->treeWidget));
	tree_item.last()->setText(0, text);
	*parent = tree_item.size() - 1;
	mythread.ready = true;
}

void tuning::tree_create_child(int *parent, QString text, int pid)
{
	if (pid >= 0 && mytune->pids_rate.at(pid) == 0) {
		mytune->pids_rate[pid] = 1;
	}
	tree_pid.append(pid);
	tree_item.append(new QTreeWidgetItem());
	tree_item.last()->setText(0, text);
	tree_item.last()->setExpanded(true);
	tree_item.last()->setTextColor(0, tree_item.at(*parent)->textColor(0));
	tree_item.at(*parent)->addChild(tree_item.last());
	tree_item.at(*parent)->setExpanded(true);
	*parent = tree_item.size() - 1;
	mythread.ready = true;
}

void tuning::setcolor(int index, QColor color)
{
	tree_item.at(index)->setForeground(0, QBrush(color));
	for (int i = 0; i < tree_item.at(index)->childCount(); i++) {
		setcolor(tree_item.indexOf(tree_item.at(index)->child(i)), color);
	}
}

void tuning::updateresults()
{
	ui->treeWidget->setColumnCount(1);
	ui->treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

	if (mytune->tp.system == SYS_DSS) {
		return;
	}
	
	mytune->pids.clear();
	mytune->pids.append(0x2000);
	mytune->demux_video();

	mythread.loop	= false;
	mythread.thread_function.append("parsetp");
	mythread.start();
}

void tuning::stop_demux()
{
	qDebug() << "stop_demux()";

	setup_demux();
	mytune->pids.clear();
	mytune->pids.append(0x2000);
	mytune->demux_video();
}

void tuning::setup_demux()
{
	mytune->pids.clear();
	for(int a = 0; a < list_pid.size(); a++) {
		if (list_item.at(a)->isSelected()) {
			mytune->pids.append(list_pid.at(a));
		}
	}

	if (mytune->pids.size()) {
		mytune->pids.append(0x00);
		mytune->pids.append(0x10);
	} else {
		mytune->pids.append(0x2000);
	}
	mytune->demux_video();
}

void tuning::on_pushButton_ipcleaner_clicked()
{
	if (myProcess.pid()) {
		myProcess.terminate();
		myProcess.waitForFinished();
	}

	setup_demux();
	QString cmd = mysettings->value("cmd_ipcleaner").toString();
	cmd.replace("{}", QString::number(mytune->adapter));
	myProcess.start(cmd);

	return;
}

void tuning::on_pushButton_play_clicked()
{
	if (myProcess.pid()) {
		myProcess.terminate();
		myProcess.waitForFinished();
	}

	mytune->close_dvr();
	setup_demux();
	QString cmd = mysettings->value("cmd_play").toString();
	cmd.replace("{}", QString::number(mytune->adapter));
	myProcess.start(cmd);

	return;
}

void tuning::on_pushButton_demux_clicked()
{
	mytune->close_dvr();	
	setup_demux();

	demux_dvr demux_dvr_dialog;
	demux_dvr_dialog.setModal(true);
	demux_dvr_dialog.updatetxt("Saving data to /dev/dvb/adapter"+QString::number(mytune->adapter)+"/dvr0");
	demux_dvr_dialog.exec();

	stop_demux();
}

void tuning::on_pushButton_file_clicked()
{
	setup_demux();

	demux_file demux_file_dialog;
	demux_file_dialog.mytune = mytune;
	demux_file_dialog.setModal(true);
	demux_file_dialog.exec();

	stop_demux();
}

void tuning::on_pushButton_expand_clicked()
{
	for(int a = 0; a < tree_item.size(); a++) {
		tree_item.at(a)->setExpanded(true);
	}
}

void tuning::on_pushButton_unexpand_clicked()
{
	for(int a = 0; a < tree_item.size(); a++) {
		tree_item.at(a)->setExpanded(false);
	}
}

void tuning::on_pushButton_stream_clicked()
{
	mystream.loop = false;
	mystream.quit();
	mystream.wait(1000);
	while (mystream.isRunning()) {
		qDebug() << "mystream.isRunning()";
		mystream.loop = false;
		sleep(1);
	}
	
	setup_demux();
	
	mystream.mytune	= mytune;
	mystream.start();
}

void tuning::delete_iqplot()
{
	qDebug() << "delete_iqplot()";
	ui->pushButton_iqplot->show();
}

void tuning::on_pushButton_iqplot_clicked()
{
	qDebug() << "on_pushButton_iqplot_clicked()";
	myiqplot = new IQplot;
	
	connect(myiqplot, SIGNAL(destroyed()), this, SLOT(delete_iqplot()));
	
	myiqplot->mytune = mytune;
	myiqplot->setModal(false);
	myiqplot->init();
	myiqplot->show();

	ui->pushButton_iqplot->hide();
}
