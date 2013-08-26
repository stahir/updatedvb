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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tuning.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	qRegisterMetaType<double>("double");
	qRegisterMetaType<QVector<double> >("QVector<double>");
	qRegisterMetaType<short int>("short int");
	qRegisterMetaType<QVector<short int> >("QVector<short int>");

	curve.append(new QwtPlotCurve("V"));
	curve.append(new QwtPlotCurve("H"));
	curve.append(new QwtPlotCurve("N"));

	legend = new QwtLegend;
	ui->qwtPlot->insertLegend(legend, QwtPlot::RightLegend);
	ui->qwtPlot->setAxisTitle(QwtPlot::xBottom, "Frequency");
	ui->qwtPlot->setAxisTitle(QwtPlot::yLeft, "Amplitude");

	qwt_picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, ui->qwtPlot->canvas());
	qwt_picker->setStateMachine(new QwtPickerDragPointMachine());
	qwt_picker->setRubberBandPen(QColor(Qt::darkMagenta));
	qwt_picker->setRubberBand(QwtPicker::CrossRubberBand);
	qwt_picker->setTrackerPen(QColor(Qt::black));
	connect(qwt_picker, SIGNAL(selected(QPointF)), this, SLOT(qwtPlot_selected(QPointF)));

	mysettings	= new QSettings("UDL", "updateDVB");
	myscan		= new scan;
	connect(myscan, SIGNAL(signaldraw(QVector<double>, QVector<double>, int, int, int)), this, SLOT(qwt_draw(QVector<double>, QVector<double>, int, int, int)));

	reload_settings();
	getadapters();
	reload_settings();
}

MainWindow::~MainWindow()
{
	qDebug() << "~MainWindow()";
	
	myscan->loop = false;
	myscan->quit();
	myscan->wait(1000);
	while (myscan->isRunning()) {
		qDebug() << "myscan->isRunning()";
		myscan->loop = false;
		sleep(1);
	}
	delete myscan;
	
	for(int i = 0; i < mytuners.size(); i++) {
		mytuners.at(i)->loop = false;
		mytuners.at(i)->quit();
		mytuners.at(i)->wait(1000);
		while (mytuners.at(i)->isRunning()) {
			qDebug().nospace() << "mytuners.at(" << i << ")->isRunning()";
			mytuners.at(i)->loop = false;
			sleep(1);
		}
		mytuners.at(i)->stop_demux();
		mytuners.at(i)->closefd();		
		delete mytuners.at(i);
	}
	
	for(int i = 0; i < curve.size(); i++) {
		curve.at(i)->detach();
		delete curve.at(i);
	}

	for(int i = 0; i < marker.size(); i++) {
		marker.at(i)->detach();
//		delete marker.at(i);
	}

	delete qwt_picker;
	delete legend;
	
	ui->qwtPlot->deleteLater();
	
	delete ui;
}

void MainWindow::reload_settings()
{
	mysettings->sync();
	tune_ops.clear();
	tuning_options tmp;
	for (int a = 0; a < 10; a++) {
		tmp.f_lof		= mysettings->value("lnb"+QString::number(a)+"_freqlof").toInt();
		tmp.f_start		= mysettings->value("lnb"+QString::number(a)+"_freqstart").toInt();
		tmp.f_stop		= mysettings->value("lnb"+QString::number(a)+"_freqstop").toInt();
		tmp.voltage		= mysettings->value("lnb"+QString::number(a)+"_voltage").toInt();
		tmp.tone		= mysettings->value("lnb"+QString::number(a)+"_tone").toBool();
//		tmp.mis			= mysettings->value("lnb"+QString::number(a)+"_mis").toInt();
		tmp.mis			= -1;
		tmp.diseqctype	= mysettings->value("lnb"+QString::number(a)+"_diseqctype").toString();
		tmp.diseqcport	= mysettings->value("lnb"+QString::number(a)+"_diseqcport").toInt();
		tmp.site_lat	= mysettings->value("site_lat").toDouble();
		tmp.site_long	= mysettings->value("site_long").toDouble();
		tune_ops.append(tmp);
	}

	if (ui->comboBox_adapter->currentIndex() < 0) {
		return;
	}
	
	QVariant d(0);
	QVariant e(1|32);
	qDebug() << "Adapter:" << ui->comboBox_adapter->currentIndex() << "lnb:" << ui->comboBox_lnb->currentIndex() << "Voltage setting:" << tune_ops[ui->comboBox_lnb->currentIndex()].voltage;
	switch(tune_ops[ui->comboBox_lnb->currentIndex()].voltage) {
	case 0:
		ui->gridLayoutWidget_6->hide(); // gridLayoutWidget_6 = gridLayout_voltage
		ui->comboBox_voltage->setCurrentIndex(0);
		ui->comboBox_voltage->setItemData(0, e, Qt::UserRole -1);
		ui->comboBox_voltage->setItemData(1, d, Qt::UserRole -1);
		ui->comboBox_voltage->setItemData(2, d, Qt::UserRole -1);
		break;
	case 1:
		ui->gridLayoutWidget_6->hide();
		ui->comboBox_voltage->setCurrentIndex(1);
		ui->comboBox_voltage->setItemData(0, d, Qt::UserRole -1);
		ui->comboBox_voltage->setItemData(1, e, Qt::UserRole -1);
		ui->comboBox_voltage->setItemData(2, d, Qt::UserRole -1);
		break;
	case 2:
		ui->gridLayoutWidget_6->hide();
		ui->comboBox_voltage->setCurrentIndex(2);
		ui->comboBox_voltage->setItemData(0, d, Qt::UserRole -1);
		ui->comboBox_voltage->setItemData(1, d, Qt::UserRole -1);
		ui->comboBox_voltage->setItemData(2, e, Qt::UserRole -1);
		break;
	case 3:
		ui->gridLayoutWidget_6->show();
		ui->comboBox_voltage->setCurrentIndex(1);
		ui->comboBox_voltage->setItemData(0, e, Qt::UserRole -1);
		ui->comboBox_voltage->setItemData(1, e, Qt::UserRole -1);
		ui->comboBox_voltage->setItemData(2, d, Qt::UserRole -1);
		break;
	}
	ui->qwtPlot->setAxisScale(QwtPlot::xBottom, tune_ops[ui->comboBox_lnb->currentIndex()].f_start, tune_ops[ui->comboBox_lnb->currentIndex()].f_stop);
	ui->qwtPlot->replot();
}

void MainWindow::qwt_draw(QVector<double> x, QVector<double> y, int min, int max, int cindex)
{
	int lnb = ui->comboBox_lnb->currentIndex();
	ui->qwtPlot->setAxisScale(QwtPlot::xBottom, tune_ops[lnb].f_start, tune_ops[lnb].f_stop);
	ui->qwtPlot->setAxisScale(QwtPlot::yLeft, min, max);

	curve[cindex]->setTitle("LNB " + QString::number(lnb) + dvbnames.voltage[cindex]);
	curve[cindex]->attach(ui->qwtPlot);
	curve[cindex]->setSamples(x, y);

	for (int i = 0; i < marker.size(); i++) {
		marker.at(i)->detach();
	}
	marker.clear();

	QwtSymbol *sym = new QwtSymbol(QwtSymbol::Diamond, QBrush(Qt::blue), QPen(Qt::blue), QSize(5,5));
	for (int i = 0; i < mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.size(); i++) {
		marker.append(new QwtPlotMarker);
		marker.at(i)->setSymbol(sym);
		marker.at(i)->setLabel(QwtText(QString::number(mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.at(i).frequency)));
		marker.at(i)->setLabelAlignment(Qt::AlignTop);
		marker.at(i)->setValue(mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.at(i).frequency, mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.at(i).spectrumscan_lvl);
		marker.at(i)->attach(ui->qwtPlot);
	}
	
	ui->qwtPlot->replot();
	
	myscan->ready = true;
}

void MainWindow::qwtPlot_selected(QPointF pos)
{
	if (myscan->loop) {
		qDebug() << "Spectrumscan loop in progress, stop spectrum scan then attempt to tune again...";
		return;
	}

	if (mytuners.at(ui->comboBox_adapter->currentIndex())->loop) {
		qDebug() << "Tuner is busy, attempt to tune again later when its no longer busy...";
		return;
	}
	
	for (int i = 0; i < tuningdialog.size(); i++) {
		if (tuningdialog.at(i)->shutdown) {
			tuningdialog.remove(i);
		}
	}
	
	tuningdialog.append(new tuning);
	tuningdialog.last()->setModal(false);
	tuningdialog.last()->mytune = mytuners.at(ui->comboBox_adapter->currentIndex());		
	tuningdialog.last()->mytune->tp.frequency	= (int)pos.x();
	tuningdialog.last()->mytune->tp.voltage		= ui->comboBox_voltage->currentIndex();

	if (ui->gridLayoutWidget->isVisible()) {
		tuningdialog.last()->mytune->tp.modulation	= dvbnames.modulation.indexOf(ui->comboBox_modulation->currentText());
		tuningdialog.last()->mytune->tp.system		= dvbnames.system.indexOf(ui->comboBox_system->currentText());
		tuningdialog.last()->mytune->tp.symbolrate	= ui->lineEdit_symbolrate->text().toInt();
		tuningdialog.last()->mytune->tp.fec			= dvbnames.fec.indexOf(ui->comboBox_fec->currentText());
	} else {
		tuningdialog.last()->mytune->tp.modulation	= QPSK;
		tuningdialog.last()->mytune->tp.system		= SYS_DVBS;
		tuningdialog.last()->mytune->tp.symbolrate	= 1000;
		tuningdialog.last()->mytune->tp.fec			= FEC_AUTO;
	}
	
	tuningdialog.last()->init();
	tuningdialog.last()->show();
}

void MainWindow::on_actionSettings_triggered()
{
	settings settings_dialog;
	settings_dialog.setModal(true);
	settings_dialog.exec();

	reload_settings();
}

void MainWindow::on_comboBox_lnb_currentIndexChanged()
{
	qDebug() << "on_comboBox_lnb_currentIndexChanged()";
	mytuners.at(ui->comboBox_adapter->currentIndex())->tune_ops = tune_ops[ui->comboBox_lnb->currentIndex()];
	reload_settings();
}

void MainWindow::on_comboBox_voltage_currentIndexChanged()
{
	switch(ui->comboBox_voltage->currentIndex())
	{
	case 0:
		curve[0]->setPen(QPen(Qt::black));
		curve[1]->setPen(QPen(Qt::gray));
		ui->qwtPlot->replot();
		break;
	case 1:
		curve[0]->setPen(QPen(Qt::gray));
		curve[1]->setPen(QPen(Qt::black));
		ui->qwtPlot->replot();
		break;
	}
}

void MainWindow::on_checkBox_loop_stateChanged()
{
	myscan->loop = ui->checkBox_loop->isChecked();
}

void MainWindow::on_updateButton_clicked()
{
	myscan->mytune = mytuners.at(ui->comboBox_adapter->currentIndex());
	
	if (ui->checkBox_fast->isChecked()) {
		myscan->step = 5;
	} else {
		myscan->step = 1;
	}
	if (dvbnames.system.indexOf(ui->comboBox_system->currentText()) == SYS_ATSC) {
		myscan->step *= 1000;
	}

	myscan->mytune->tp.system = dvbnames.system.indexOf(ui->comboBox_system->currentText());
	myscan->loop	= ui->checkBox_loop->isChecked();
	myscan->setup();
	myscan->start();
}

void MainWindow::on_pushButton_scan_clicked()
{
	blindscan blindscandialog;
	blindscandialog.mytune = mytuners.at(ui->comboBox_adapter->currentIndex());
	blindscandialog.init();

	if (dvbnames.system.indexOf(ui->comboBox_system->currentText()) == SYS_ATSC) {
		atsc myatsc;
		tp_info tp;
		mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.clear();
		for (int i = 0; i < myatsc.freq.size(); i++) {
			if (myatsc.freq.at(i) >= mytuners.at(ui->comboBox_adapter->currentIndex())->tune_ops.f_start && myatsc.freq.at(i) <= mytuners.at(ui->comboBox_adapter->currentIndex())->tune_ops.f_stop) {
				tp.frequency	= myatsc.freq.at(i);
				mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.append(tp);
			}
		}
		mytuners.at(ui->comboBox_adapter->currentIndex())->tp.system		= dvbnames.system.indexOf(ui->comboBox_system->currentText());
		mytuners.at(ui->comboBox_adapter->currentIndex())->tp.modulation	= dvbnames.modulation.indexOf(ui->comboBox_modulation->currentText());
	} else {
		mytuners.at(ui->comboBox_adapter->currentIndex())->tp.system		= SYS_DVBS;
		mytuners.at(ui->comboBox_adapter->currentIndex())->tp.modulation	= QPSK;
	}

	if (ui->checkBox_smart->isChecked() && mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.size()) {
		blindscandialog.smartscan();
	} else {
		blindscandialog.scan();
	}

	blindscandialog.init();
	blindscandialog.setModal(true);
	blindscandialog.exec();
}

void MainWindow::on_comboBox_adapter_currentIndexChanged()
{
	qDebug() << "on_comboBox_adapter_currentIndexChanged() Adapter:" << ui->comboBox_adapter->currentIndex();
	
	ui->comboBox_frontend->clear();
	QDir adapter_dir("/dev/dvb/adapter" + ui->comboBox_adapter->currentText());
	adapter_dir.setFilter(QDir::System|QDir::NoDotAndDotDot);
	QStringList frontend_entries = adapter_dir.entryList();
	for(QStringList::ConstIterator frontend_entry = frontend_entries.begin(); frontend_entry != frontend_entries.end(); frontend_entry++) {
		QString frontend_dirname = *frontend_entry;
		if (frontend_dirname.contains("frontend")) {
			frontend_dirname.replace("frontend", "");
			ui->comboBox_frontend->addItem(frontend_dirname);			
		}
	}
	
	if (mysettings->value("adapter" + QString::number(ui->comboBox_adapter->currentIndex()) + "_diseqc_v12").toBool()) {
		ui->gridLayoutWidget_9->show(); // gridLayoutWidget_9 = gridLayout_gotox
	} else {
		ui->gridLayoutWidget_9->hide();
	}
	if (mysettings->value("adapter" + QString::number(ui->comboBox_adapter->currentIndex()) + "_diseqc_v13").toBool()) {
		ui->gridLayoutWidget_11->show(); // gridLayoutWidget_11 = gridLayout_usals
	} else {
		ui->gridLayoutWidget_11->hide();
	}
	if (mysettings->value("adapter" + QString::number(ui->comboBox_adapter->currentIndex()) + "_diseqc_v12").toBool() || mysettings->value("adapter" + QString::number(ui->comboBox_adapter->currentIndex()) + "_diseqc_v13").toBool()) {
		ui->gridLayoutWidget_13->show(); // gridLayoutWidget_13 = gridLayout_positioner
	} else {
		ui->gridLayoutWidget_13->hide();
	}
	
	ui->statusBar->showMessage(mytuners.at(ui->comboBox_adapter->currentIndex())->name, 0);

	if (mytuners.at(ui->comboBox_adapter->currentIndex())->caps & FE_CAN_SPECTRUMSCAN) {
		ui->gridLayoutWidget_3->show(); // gridLayoutWidget_3 = spectrumscan
	} else {
		ui->gridLayoutWidget_3->hide();
	}

	ui->comboBox_system->clear();
	if (mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.size()) {
		for(int i = 0; i < mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.size(); i++) {
			ui->comboBox_system->addItem(dvbnames.system[mytuners.at(ui->comboBox_adapter->currentIndex())->delsys[i]]);
		}
	} else { // This should never happen, just in case some driver is seriously messed up
		ui->comboBox_system->addItem("DVB-S");
		ui->comboBox_system->addItem("DVB-S2");
		ui->comboBox_system->addItem("ATSC");
		ui->comboBox_system->addItem("DVB-C B");
	}
	
	ui->comboBox_modulation->clear();
	if (mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.indexOf(SYS_ATSC) != -1) {
		ui->comboBox_modulation->addItem("VSB 8");
		ui->comboBox_modulation->addItem("QAM 64");
		ui->comboBox_modulation->addItem("QAM 256");
	}
	if (mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.indexOf(SYS_DCII) != -1) {
		ui->comboBox_modulation->addItem("C QPSK");
		ui->comboBox_modulation->addItem("I QPSK");
		ui->comboBox_modulation->addItem("Q QPSK");
		ui->comboBox_modulation->addItem("C OQPSK");
	}
	if (mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.indexOf(SYS_DVBS2) != -1 || mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.indexOf(SYS_TURBO) != -1) {
		ui->comboBox_modulation->addItem("QPSK");
		ui->comboBox_modulation->addItem("8PSK");
	} else {
		if (mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.indexOf(SYS_DVBS) != -1) {
			ui->comboBox_modulation->addItem("QPSK");
		}
	}

	ui->comboBox_fec->clear();
	ui->comboBox_fec->addItem("Auto");

	if (mytuners.at(ui->comboBox_adapter->currentIndex())->caps & FE_CAN_BLINDSEARCH) {
		ui->gridLayoutWidget->hide();   // gridLayoutWidget   = tuning
		ui->gridLayoutWidget_2->show(); // gridLayoutWidget_2 = blindscan
		ui->gridLayoutWidget_7->hide(); // gridLayoutWidget_7 = satellite
	} else {
		ui->gridLayoutWidget->show();
		ui->gridLayoutWidget_2->hide();
		
		if (mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.indexOf(SYS_ATSC) != -1) {
			ui->gridLayoutWidget_2->show();
			ui->gridLayoutWidget_7->hide();
		} else {
			ui->gridLayoutWidget_7->show();			
		}
	}
}

void MainWindow::adapter_status(int adapter, bool is_busy)
{
	if (is_busy) {
		ui->comboBox_adapter->setItemText(adapter, QString("%1 Busy").arg(adapter));
		ui->comboBox_adapter->setItemData(adapter, Qt::red, Qt::TextColorRole);
	} else {
		ui->comboBox_adapter->setItemText(adapter, QString("%1").arg(adapter));
		ui->comboBox_adapter->setItemData(adapter, Qt::black, Qt::TextColorRole);
	}
}

void MainWindow::getadapters()
{
	QDir dvb_dir("/dev/dvb");
	dvb_dir.setFilter(QDir::Dirs|QDir::NoDotAndDotDot);
	QStringList adapter_entries = dvb_dir.entryList();
	for(QStringList::ConstIterator adapter_entry = adapter_entries.begin(); adapter_entry != adapter_entries.end(); adapter_entry++) {
		QString adapter_dirname = *adapter_entry;
		adapter_dirname.replace("adapter", "");

		mytuners.insert(adapter_dirname.toInt(), new dvbtune);
		connect(mytuners.last(), SIGNAL(adapter_status(int,bool)), this, SLOT(adapter_status(int,bool)));
		mytuners.at(adapter_dirname.toInt())->adapter	= adapter_dirname.toInt();
		mytuners.at(adapter_dirname.toInt())->frontend	= 0;
		mytuners.at(adapter_dirname.toInt())->tune_ops	= tune_ops[ui->comboBox_lnb->currentIndex()];
		mytuners.at(adapter_dirname.toInt())->getops();

		ui->comboBox_adapter->addItem(adapter_dirname);
	}
	if (!mytuners.size()) {
		return;
	}
}

void MainWindow::on_pushButton_usals_go_clicked()
{
	mytuners.at(ui->comboBox_adapter->currentIndex())->old_position = mysettings->value("adapter" + QString::number(ui->comboBox_adapter->currentIndex()) + "_usals_position").toDouble();
	mytuners.at(ui->comboBox_adapter->currentIndex())->usals_drive(ui->lineEdit_usals->text().toDouble());
	mysettings->setValue("adapter"+QString::number(ui->comboBox_adapter->currentIndex())+"_usals_position", ui->lineEdit_usals->text().toDouble());
}

void MainWindow::on_lineEdit_usals_returnPressed()
{
	on_pushButton_usals_go_clicked();
}

void MainWindow::on_pushButton_drive_east_L_clicked()
{
	mytuners.at(ui->comboBox_adapter->currentIndex())->step_motor(0, 5);
}

void MainWindow::on_pushButton_drive_east_S_clicked()
{
	mytuners.at(ui->comboBox_adapter->currentIndex())->step_motor(0, 1);    
}

void MainWindow::on_pushButton_drive_west_S_clicked()
{
	mytuners.at(ui->comboBox_adapter->currentIndex())->step_motor(1, 1);
}

void MainWindow::on_pushButton_drive_west_L_clicked()
{
	mytuners.at(ui->comboBox_adapter->currentIndex())->step_motor(1, 5);
}

void MainWindow::on_pushButton_gotox_go_clicked()
{
    mytuners.at(ui->comboBox_adapter->currentIndex())->gotox_drive(ui->lineEdit_gotox->text().toInt());
}

void MainWindow::on_lineEdit_gotox_returnPressed()
{
	on_pushButton_gotox_go_clicked();
}

void MainWindow::on_pushButton_gotox_save_clicked()
{
	mytuners.at(ui->comboBox_adapter->currentIndex())->gotox_save(ui->lineEdit_gotox->text().toInt());
}

void MainWindow::on_actionExit_triggered()
{
    exit(0);
}
