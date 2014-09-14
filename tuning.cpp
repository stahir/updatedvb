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
	QWidget(parent),
	ui(new Ui::tuning)
{
	ui->setupUi(this);

	QPalette black_palette;
	black_palette = ui->listWidget->palette();
	black_palette.setColor(QPalette::Base, Qt::black);
	ui->listWidget->setPalette(black_palette);
	ui->listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

	mystatusbar = new QStatusBar;
	mystatusbar->setVisible(true);
	ui->verticalLayout->addWidget(mystatusbar);

	mysettings = new QSettings("UDL", "updateDVB");

	shutdown = false;
	parsetp_started = false;

	ui->treeWidget->setColumnCount(1);
	ui->treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
	black_palette = ui->treeWidget->palette();
	black_palette.setColor(QPalette::Base, Qt::black);
	ui->treeWidget->setPalette(black_palette);

	status_timer = new QTimer;
}

tuning::~tuning()
{
	if (!myiqplot.isNull()) {
		myiqplot->deleteLater();
	}
	if (!mydemux_file.isNull()) {
		mydemux_file->on_pushButton_stop_clicked();
		mydemux_file->deleteLater();
	}

	mystream.server_close();
	mystream_thread.quit();
	mystream_thread.wait(1000);
	while (mystream_thread.isRunning()) {
		QThread::msleep(100);
	}

	mythread.ready	= true;
	mythread.loop	= false;
	mythread.quit();
	mythread.wait(1000);
	while (mythread.isRunning()) {
		mythread.ready	= true;
		mythread.loop	= false;
		QThread::msleep(100);
	}

	mytune->status = unsetbit(mytune->status, 0xff);
	emit adapter_status(mytune->adapter);

	mytune->loop		= false;
	mytune->quit();
	mytune->wait(1000);
	while (mytune->isRunning()) {
		mytune->loop = false;
		QThread::msleep(100);
	}

	status_timer->deleteLater();

	delete mysettings;
	delete mystatusbar;
	delete ui;

	shutdown = true;
}

void tuning::closeEvent(QCloseEvent *event)
{
	Q_UNUSED(event);
	shutdown = true;
	this->deleteLater();
}

void tuning::init()
{
	connect(&myProcess, SIGNAL(finished(int)), this, SLOT(myProcess_finished()));
	connect(mytune, SIGNAL(update_signal()), this, SLOT(update_signal()));
	connect(mytune, SIGNAL(update_results()), this, SLOT(update_results()));
	connect(mytune, SIGNAL(update_status(QString,int)), this, SLOT(update_status(QString,int)));
	connect(&mythread, SIGNAL(list_create(QString, int)), this, SLOT(list_create(QString, int)));
	connect(&mythread, SIGNAL(tree_create_root(int *, QString, int)), this, SLOT(tree_create_root(int *, QString, int)));
	connect(&mythread, SIGNAL(tree_create_child(int *, QString, int)), this, SLOT(tree_create_child(int *, QString, int)));
	connect(&mythread, SIGNAL(setcolor(int,QColor)), this, SLOT(setcolor(int,QColor)));
	connect(&mythread, SIGNAL(parsetp_done()), this, SLOT(parsetp_done()));
	connect(&mystream, SIGNAL(update_status(QString,int)), this, SLOT(update_status(QString,int)));
	connect(this, SIGNAL(server_new()), &mystream, SLOT(server_new()));
	connect(mytune->mydvr, SIGNAL(data(QByteArray)), &mystream, SLOT(stream(QByteArray)));
	connect(&status_mapper, SIGNAL(mapped(QString)), this, SLOT(update_status(QString)));

	update_status("Tuning...", STATUS_NOEXP);
	mytune->start();
	mytune->thread_function.append("tune");
	mytune->status = setbit(mytune->status, TUNER_TUNED);
	emit adapter_status(mytune->adapter);
	mythread.mytune = mytune;

	mystream.mytune	= mytune;
	mystream.moveToThread(&mystream_thread);
	mystream_thread.start();

	unlock_t.start();

	this->setWindowTitle("Tuning Adapter " + QString::number(mytune->adapter) + ", Frontend " + QString::number(mytune->frontend) + " : " + mytune->name);

	ui->pushButton_demux->setEnabled(false);
	ui->pushButton_file->setEnabled(false);
	ui->pushButton_ipcleaner->setEnabled(false);
	ui->pushButton_play->setEnabled(false);
	ui->pushButton_stream->setEnabled(false);
	ui->pushButton_iqplot->setEnabled(false);
}

void tuning::myProcess_finished()
{
	mytune->close_dvr();
}

void tuning::on_treeWidget_itemClicked(QTreeWidgetItem * item, int column)
{
	Q_UNUSED(item);
	Q_UNUSED(column);

	// If mythread is running, dont click on tree items
	if (mythread.loop) {
		return;
	}

	for(int a = 0; a < list_item.size(); a++) {
		list_item.at(a)->setSelected(false);
	}

	for(int a = 0; a < tree_item.size(); a++) {
		if (tree_item.at(a)->isSelected()) {
			for (int i = 0; i < tree_item.at(a)->childCount(); i++) {
				tree_item.at(a)->child(i)->setSelected(true);
			}
			if (tree_pid.at(a) < 0) {
				tree_item.at(a)->setSelected(false);
				continue;
			}
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

	for (int a = 0; a < list_item.size(); a++) {
		if (list_item.at(a)->isSelected()) {
			if (list_pid.at(a) < 0)
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

void tuning::parsetp_done()
{
	update_status("", STATUS_CLEAR);
	update_status("Parsing transponder done", 2);
}

void tuning::update_signal()
{
	if (mytune->tp.status & FE_HAS_LOCK) {
		if (parsetp_started) {
			unlock_t.restart();
		} else if (unlock_t.elapsed() > 5000) {
			update_results();
		}
		ui->pushButton_demux->setEnabled(true);
		if (mydemux_file.isNull()) {
			ui->pushButton_file->setEnabled(true);
		}
		ui->pushButton_ipcleaner->setEnabled(true);
		ui->pushButton_play->setEnabled(true);
		ui->pushButton_stream->setEnabled(true);
		ui->label_lock->setText("Locked");
		ui->label_lock->setStyleSheet("QLabel { color : green; }");
	} else {
		unlock_t.restart();
		ui->pushButton_demux->setEnabled(false);
		ui->pushButton_file->setEnabled(false);
		ui->pushButton_ipcleaner->setEnabled(false);
		ui->pushButton_play->setEnabled(false);
		ui->pushButton_stream->setEnabled(false);
		ui->label_lock->setText("Unlocked");
		ui->label_lock->setStyleSheet("QLabel { color : red; }");
		if (isSatellite(mytune->tp.system) && mytune->tp.fec == FEC_AUTO) {
			ui->label_frequency->setText("");
			return;
		}
	}
	if (mytune->caps & FE_CAN_IQ) {
		if (myiqplot.isNull()) {
			ui->pushButton_iqplot->setEnabled(true);
		}
	} else {
		ui->pushButton_iqplot->setEnabled(false);
	}

	if (mytune->tp.lvl_scale == FE_SCALE_DECIBEL) {
		ui->label_signalS->setText(QString::number(mytune->tp.lvl, 'f', 1) + "dBm");
	} else {
		ui->label_signalS->setText(QString::number(mytune->tp.lvl) + "%");
	}
	if (mytune->tp.snr_scale == FE_SCALE_DECIBEL) {
		ui->label_signalQ->setText(QString::number(mytune->tp.snr, 'f', 1) + "dB");
		ui->label_signalQ->setToolTip("min snr: " + mytune->min_snr() + "dB");
	} else {
		ui->label_signalQ->setText(QString::number(mytune->tp.snr, 'f', 1) + "%");
	}
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
			ui->label_tsgs->setText("GP");
			break;
		case 1:
			ui->label_tsgs->setText("GS");
			break;
		case 2:
			ui->label_tsgs->setText("Reserved");
			break;
		case 3:
			ui->label_tsgs->setText("TS");
			break;
		}
		switch(mytune->maskbits(matype, 0x20)) {
		case 0:
			ui->label_sismis->setText("MIS");
			break;
		case 1:
			ui->label_sismis->setText("SIS");
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
		ui->label_frequency->setText(mytune->format_freq(mytune->tp.frequency, mytune->tp.system));
	}
}

void tuning::list_create(QString text, int pid)
{
	list_pid.append(pid);
	list_item.append(new QListWidgetItem(ui->listWidget));
	list_item.last()->setText(text);
	list_item.last()->setTextColor(QColor(Qt::gray));
	if (pid == 0x00 || pid == 0x11 || pid == 0x1fff) {
		list_item.last()->setTextColor(QColor(Qt::green));
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
	tree_item.last()->setTextColor(0, QColor(Qt::gray));
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
	tree_item.last()->setTextColor(0, QColor(Qt::gray));
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

void tuning::update_results()
{
	if (mytune->tp.system == SYS_DSS) {
		return;
	}

	mytune->pids.clear();
	mytune->pids.append(0x2000);
	mytune->demux_video();

	parsetp_started = true;
	mythread.loop	= false;
	mythread.thread_function.append("parsetp");
	mythread.start();

	update_status("", STATUS_CLEAR);
	update_status("Parsing transponder...", STATUS_NOEXP);
}

void tuning::stop_demux()
{
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
	if (mytune->status & TUNER_DEMUX) {
		return;
	}
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
	if (mytune->status & TUNER_DEMUX) {
		return;
	}
	if (myProcess.pid()) {
		qDebug() << "stopping previous player first...";
		myProcess.kill();
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
	mytune->close_dvr();
}

void tuning::delete_demux_file()
{
	ui->pushButton_file->setEnabled(true);

	stop_demux();
	mytune->close_dvr();
}

void tuning::on_pushButton_file_clicked()
{
	if (mytune->status & TUNER_DEMUX) {
		return;
	}

	ui->pushButton_file->setEnabled(false);

	setup_demux();

	mydemux_file = new demux_file;
	connect(mydemux_file, SIGNAL(destroyed()), this, SLOT(delete_demux_file()));

	mydemux_file->mytune = mytune;
	mydemux_file->init();
	mydemux_file->show();
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
	if (mystream.server && mystream.server->isListening()) {
		mystream.server_close();
	} else {
		if (mytune->status & TUNER_DEMUX) {
			return;
		}
		setup_demux();
		emit server_new();
	}
}

void tuning::delete_iqplot()
{
	ui->pushButton_iqplot->setEnabled(true);
}

void tuning::on_pushButton_iqplot_clicked()
{
	myiqplot = new iqplot;

	connect(myiqplot, SIGNAL(destroyed()), this, SLOT(delete_iqplot()));

	myiqplot->mytune = mytune;
	myiqplot->init();
	myiqplot->show();

	ui->pushButton_iqplot->setEnabled(false);
}

void tuning::update_status(QString text, int time)
{
	if (shutdown) {
		return;
	}
	if (time == STATUS_CLEAR) {
		mystatus.clear();
	}
	if (time == STATUS_REMOVE) {
		if (mystatus.indexOf(text) != -1) {
			mystatus.remove(mystatus.indexOf(text));
		}
	}
	if (time == STATUS_NOEXP) {
		mystatus.append(text);
	}
	if (time > 0) {
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

void tuning::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape) {
		this->close();
	}
}
