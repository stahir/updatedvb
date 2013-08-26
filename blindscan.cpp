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

#include "blindscan.h"
#include "ui_blindscan.h"

blindscan::blindscan(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::blindscan)
{
	ui->setupUi(this);

	pindex = -1;
	cindex = -1;
	mylayout = new QVBoxLayout(ui->widget);
	mystatus = new QStatusBar;
	mystatus->setVisible(false);
	mylayout->addWidget(mystatus);
	myprogress = new QProgressBar;
	myprogress->setMinimum(0);
	myprogress->setMaximum(100);
	mylayout->addWidget(myprogress);
	mylayout->setMargin(0);
	mylayout->setSpacing(0);
}

void blindscan::init()
{
	mythread.mytune = mytune;
	connect(mytune, SIGNAL(updateresults()), this, SLOT(updateresults()));
	connect(&mythread, SIGNAL(updateprogress(int)), this, SLOT(updateprogress(int)));
}

blindscan::~blindscan()
{
	qDebug() << "~blindscan()";
	
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

	delete ui;
}

void blindscan::scan()
{
	mythread.loop = false;
	mythread.thread_function.append("blindscan");
	mythread.start();
}

void blindscan::smartscan()
{
	mythread.loop = false;
	mythread.thread_function.append("smartscan");
	mythread.start();
}

void blindscan::updateprogress(int i)
{
	if (i > myprogress->maximum()) {
		i = myprogress->maximum();
	}
	myprogress->setValue(i);
	if (myprogress->value() >= myprogress->maximum()) {
		myprogress->setVisible(false);
		mystatus->setVisible(true);
		mystatus->showMessage("Done.", 4000);
	}
}

int blindscan::tree_create_root(QString text)
{
	pindex++;
	ptree[pindex] = new QTreeWidgetItem(ui->treeWidget);
	ptree[pindex]->setText(0, text);

	return pindex;
}

int blindscan::tree_create_child(int parent, QString text)
{
	cindex++;
	ctree[cindex] = new QTreeWidgetItem();
	ctree[cindex]->setText(0, text);
	ptree[parent]->addChild(ctree[cindex]);

	return cindex;
}

void blindscan::updateresults()
{
	if (mythread.ready) {
		return;
	}

	int parent_1;

	mytp_info.append(mythread.mytune->tp);

	QString text;
	if (mytune->tp.system == SYS_ATSC) {
		atsc myatsc;
		text = QString::number(mytune->tp.frequency/1000) + "mhz, Channel " + QString::number(myatsc.ch[myatsc.freq.indexOf(mytune->tp.frequency)]);
		parent_1 = tree_create_root(text);
		text = "System: " + dvbnames.system[mytune->tp.system];
		tree_create_child(parent_1, text);
		text = "Modulation: " + dvbnames.modulation[mytune->tp.modulation];
		tree_create_child(parent_1, text);
		text = "Signal S: " + QString::number(mytune->tp.lvl) + "%";
		tree_create_child(parent_1, text);
		text = "Signal Q: " + QString::number(mytune->tp.snr) + "dB";
		tree_create_child(parent_1, text);
	} else {
		text = QString::number(mytune->tp.frequency) + dvbnames.voltage[mytune->tp.voltage] + QString::number(mytune->tp.symbolrate);
		parent_1 = tree_create_root(text);
		text = "FEC: " + dvbnames.fec[mytune->tp.fec];
		tree_create_child(parent_1, text);
		text = "System: " + dvbnames.system[mytune->tp.system];
		tree_create_child(parent_1, text);
		text = "Modulation: " + dvbnames.modulation[mytune->tp.modulation];
		tree_create_child(parent_1, text);
		text = "Inversion: " + dvbnames.inversion[mytune->tp.inversion];
		tree_create_child(parent_1, text);
		text = "Rolloff: " + dvbnames.rolloff[mytune->tp.rolloff];
		tree_create_child(parent_1, text);
		text = "Pilot: " + dvbnames.pilot[mytune->tp.pilot];
		tree_create_child(parent_1, text);
		text = "Signal S: " + QString::number(mytune->tp.lvl) + "%";
		tree_create_child(parent_1, text);
		text = "Signal Q: " + QString::number(mytune->tp.snr, 'f', 1) + "dB";
		tree_create_child(parent_1, text);
	}
	mythread.ready = true;
}

void blindscan::on_pushButton_tune_clicked()
{
	int i = -1;
	for(int a = 0; a <= pindex; a++) {
		if (ptree[a]->isSelected()) {
			i = a;
		}
	}
	if (i == -1) {
		qDebug() << "No frequency selected";
		return;
	}
	tuningdialog				= new tuning;
	tuningdialog->mytune		= mytune;
	tuningdialog->mytune->tp	= mytp_info.at(i);
	tuningdialog->init();
	tuningdialog->setModal(true);
	tuningdialog->exec();
}

void blindscan::on_pushButton_save_clicked()
{
	blindscan_save mysave;
	mysave.mytp_info	= mytp_info;

	mysave.setModal(true);
	mysave.exec();
}

void blindscan::on_pushButton_expand_clicked()
{
	for(int a = 0; a <= pindex; a++) {
		ptree[a]->setExpanded(true);
	}
}

void blindscan::on_pushButton_unexpand_clicked()
{
	for(int a = 0; a <= pindex; a++) {
		ptree[a]->setExpanded(false);
	}
}
