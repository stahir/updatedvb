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

	list_pid.clear();
	list_item.clear();
	for (int i = 0; i <= 0x2000; i++) {
		list_pid.append(0);
		list_item.append(new QListWidgetItem(ui->listWidget));
		list_item.last()->setHidden(true);
	}
}

tuning::~tuning()
{
	mythread.ready				= true;
	mythread.loop				= false;
	mythread.parsetp_loop		= false;
	mytune->demux_packets_loop	= false;
	while (mythread.parsetp_running) {
		mythread.ready				= true;
		mythread.loop				= false;
		mythread.parsetp_loop		= false;
		mytune->demux_packets_loop	= false;
		QThread::msleep(10);
	}

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
		QThread::msleep(10);
	}

	mythread.quit();
	mythread.wait(1000);
	while (mythread.isRunning()) {
		mythread.ready			= true;
		mythread.loop			= false;
		QThread::msleep(10);
	}

	mytune->status = unsetbit(mytune->status, 0xff);
	emit adapter_status(mytune->adapter);

	mytune->loop		= false;
	mytune->stop_demux();
	mytune->close_demux();
	mytune->close_dvr();
	mytune->quit();
	mytune->wait(1000);
	while (mytune->isRunning()) {
		mytune->loop = false;
		QThread::msleep(10);
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
	connect(&mythread, SIGNAL(list_create()), this, SLOT(list_create()));
	connect(&mythread, SIGNAL(tree_create(tree_item *)), this, SLOT(tree_create(tree_item *)));
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
	parsetp_start();
}

void tuning::tree_select_children(QTreeWidgetItem *item)
{
	for(int a = 0; a < item->childCount(); a++) {
		item->child(a)->setSelected(item->isSelected());
		tree_select_children(item->child(a));
	}
}

void tuning::on_treeWidget_itemClicked(QTreeWidgetItem * item, int column)
{
	Q_UNUSED(column);

	for(int a = 0; a < list_item.size(); a++) {
		list_item.at(a)->setSelected(false);
	}

	tree_select_children(item);

	for(int a = 0; a < tree_items.size(); a++) {
		if (tree_items.at(a).tree->isSelected()) {
			if (tree_items.at(a).pid != 0xFFFF) {
				list_item.at(tree_items.at(a).pid)->setSelected(item->isSelected());
			}
		}
	}
}

void tuning::on_listWidget_itemClicked(QListWidgetItem *item)
{
	Q_UNUSED(item);

	for(int a = 0; a < tree_items.size(); a++) {
		tree_items.at(a).tree->setSelected(false);
	}

	for (int a = 0; a < list_item.size(); a++) {
		if (list_item.at(a)->isSelected()) {
			for (int i = 0; i < tree_items.size(); i++) {
				if (tree_items.at(i).pid == (unsigned int)a) {
					tree_items.at(i).tree->setSelected(true);
				}
			}
		}
	}
}

void tuning::parsetp_done()
{
	update_status("", STATUS_CLEAR);
	update_status("Parsing transponder done");
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

void tuning::list_create()
{
	for (int i = 0; i < list_pid.size(); i++) {
		if (mytune->pids_rate.at(i) > 0) {
			list_pid[i] = mytune->pids_rate.at(i);
			list_item.at(i)->setHidden(false);
			list_item.at(i)->setText(QString("0x%1 - %2 kbit/s").arg(i,4,16,QChar('0')).arg(mytune->pids_rate.at(i),5,10,QChar(' ')));
			list_item.at(i)->setTextColor(QColor(Qt::gray));
			if (i == 0x1fff || i == 0x2000) {
				list_item.at(i)->setTextColor(QColor(Qt::green));
			}
		}
	}
	for (int i = 0; i < tree_items.size(); i++) { // 0x1fff padding packets and 0x2000 entire mux should show as green
		if (tree_items.at(i).pid != 0xFFFF) {
			list_item.at(tree_items.at(i).pid)->setTextColor(tree_items.at(i).tree->textColor(0));
		}
	}
}

void tuning::set_tree_color(QTreeWidgetItem *item, QColor color)
{
	if (item->childCount() > 0) {
		for (int i = 0; i < item->childCount(); i++) {
			set_tree_color(item->child(i), color);
		}
	}
	item->setTextColor(0, color);
}

void tuning::tree_create(tree_item *item)
{
	if (item->pid != 0xFFFF && mytune->pids_rate.at(item->pid) == 0) {
		mytune->pids_rate[item->pid] = 1;
	}

	if (item->pid != 0xFFFF) {
		for (int i = 0; i < tree_items.size(); i++) {
			if (item->pid == tree_items.at(i).pid && item->text == tree_items.at(i).text) {
				item->parent = i;
				mythread.ready = true;
				return;
			}
		}
	}

	item->current = tree_items.size();
	tree_items.append(*item);
	if (item->parent == -1) { // Root
		tree_items.last().tree = new QTreeWidgetItem(ui->treeWidget);
		tree_items.last().tree->setText(0, item->text);
		tree_items.last().tree->setExpanded(item->expanded);
		tree_items.last().tree->setTextColor(0, item->color);
	} else { // Child
		tree_items.last().tree = new QTreeWidgetItem();
		tree_items.last().tree->setText(0, item->text);
		if (tree_items.at(item->parent).tree->childCount() == 0) { // Can't expand an item with no children
			tree_items.at(item->parent).tree->setExpanded(item->expanded);
		} else {
			tree_items.at(item->parent).tree->setExpanded(tree_items.at(item->parent).tree->isExpanded());
		}
		if (item->color == QColor(Qt::red)) {
			tree_item tmp = tree_items.at(item->parent);
			while (tmp.parent != -1) {
				tmp = tree_items.at(tmp.parent);
			}
			set_tree_color(tmp.tree, item->color);
			tree_items.last().tree->setTextColor(0, item->color);
		} else {
			tree_items.last().tree->setTextColor(0, tree_items.at(item->parent).tree->textColor(0));
		}
		tree_items.at(item->parent).tree->addChild(tree_items.last().tree);
	}
	if (item->return_parent) {
		item->parent = item->current;
	}
	mythread.ready = true;
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

void tuning::setup_demux(QString type)
{
	mytune->pids.clear();
	for(int a = 0; a < list_pid.size(); a++) {
		if (list_item.at(a)->isSelected()) {
			mytune->pids.append(a);
		}
	}

	if (!mytune->pids.isEmpty() && !mytune->pids.contains(0x00)) { // VLC needs the PAT
		mytune->pids.append(0x00);
	} else {
		mytune->pids.append(0x2000);
	}
	if (type == "BBFrame") {
		mytune->demux_bbframe();
	} else {
		mytune->demux_video();
	}
}

void tuning::on_pushButton_ipcleaner_clicked()
{
	while (mythread.parsetp_running) {
		mythread.ready				= true;
		mythread.parsetp_loop		= false;
		mytune->demux_packets_loop	= false;
		QThread::msleep(10);
	}
	if (myProcess.pid()) {
		myProcess.terminate();
		myProcess.waitForFinished();
	}

	mytune->close_dvr();
	setup_demux();
	QString cmd = mysettings->value("cmd_ipcleaner").toString();
	cmd.replace("{}", QString::number(mytune->adapter));
	myProcess.start(cmd);

	parsetp_start();
}

void tuning::on_pushButton_play_clicked()
{
	while (mythread.parsetp_running) {
		mythread.ready				= true;
		mythread.parsetp_loop		= false;
		mytune->demux_packets_loop	= false;
		QThread::msleep(10);
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
	while (mythread.parsetp_running) {
		mythread.ready				= true;
		mythread.parsetp_loop		= false;
		mytune->demux_packets_loop	= false;
		QThread::msleep(10);
	}

	mytune->close_dvr();
	setup_demux();

	demux_dvr demux_dvr_dialog;
	demux_dvr_dialog.setModal(true);
	demux_dvr_dialog.updatetxt("Saving data to /dev/dvb/adapter"+QString::number(mytune->adapter)+"/dvr0");
	demux_dvr_dialog.exec();

	parsetp_start();
}

void tuning::delete_demux_file()
{
	ui->pushButton_file->setEnabled(true);
	parsetp_start();
}

void tuning::on_pushButton_file_clicked()
{
	while (mythread.parsetp_running) {
		mythread.ready				= true;
		mythread.parsetp_loop		= false;
		mytune->demux_packets_loop	= false;
		QThread::msleep(10);
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
	for(int a = 0; a < tree_items.size(); a++) {
		tree_items.at(a).tree->setExpanded(true);
	}
}

void tuning::on_pushButton_unexpand_clicked()
{
	for(int a = 0; a < tree_items.size(); a++) {
		tree_items.at(a).tree->setExpanded(false);
	}
}

void tuning::on_pushButton_stream_clicked()
{
	while (mythread.parsetp_running) {
		mythread.ready				= true;
		mythread.parsetp_loop		= false;
		mytune->demux_packets_loop	= false;
		QThread::msleep(10);
	}

	if (mystream.server && mystream.server->isListening()) {
		mystream.server_close();
		parsetp_start();
	} else {
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

void tuning::parsetp_start()
{
	mythread.thread_function.append("parsetp");
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
		if (mystatus.contains(text)) {
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

void tuning::on_pushButton_bbframe_clicked()
{
	if (mytune->status & TUNER_DEMUX) {
		return;
	}

	ui->pushButton_file->setEnabled(false);

	setup_demux("BBFrame");

	mydemux_file = new demux_file;
	connect(mydemux_file, SIGNAL(destroyed()), this, SLOT(delete_demux_file()));

	mydemux_file->mytune = mytune;
	mydemux_file->init();
	mydemux_file->show();
}

void tuning::save_children(QTreeWidgetItem *item, QTextStream *out)
{
	static QString indent;
	*out << indent << item->text(0) << "\n";
	for (int i = 0; i < item->childCount(); i++) {
		indent.append("\t");
		save_children(item->child(i), out);
		indent.chop(1);
	}
}

void tuning::on_pushButton_save_tree_clicked()
{
	static QString filename;
	if (filename.isEmpty()) {
		filename = QFileDialog::getSaveFileName(this, "Save Parsed Output", QDir::currentPath() + "/output.txt");
	} else {
		filename = QFileDialog::getSaveFileName(this, "Save Parsed Output", filename);
	}
	if (filename.isEmpty()) {
		return;
	}
	QFile output(filename);
	output.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream out(&output);

	for (int i = 0; i < tree_items.size(); i++) {
		if (tree_items.at(i).parent == -1) {
			save_children(tree_items.at(i).tree, &out);
		}
	}

	QMessageBox mbox;
	mbox.setText("Parsed tree output saved");
	mbox.exec();
	output.close();
}
