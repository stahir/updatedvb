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
	curve.last()->setPen(QPen(GREEN));

	legend = new QwtLegend;
	ui->qwtPlot->insertLegend(legend, QwtPlot::RightLegend);
	ui->qwtPlot->setAxisTitle(QwtPlot::xBottom, "Frequency");
	ui->qwtPlot->setAxisTitle(QwtPlot::yLeft, "Amplitude");
	ui->qwtPlot->setCanvasBackground(Qt::black);

	grid = new QwtPlotGrid();
	grid->enableX(true);
	grid->enableY(true);
	grid->enableXMin(true);
	grid->enableYMin(true);
	grid->setMajorPen(QPen(GRAY, 0, Qt::DotLine));
	grid->setMinorPen(QPen(DGRAY, 0, Qt::DotLine));
	grid->attach(ui->qwtPlot);

    qwt_picker = new PlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, qobject_cast<QwtPlotCanvas*>(ui->qwtPlot->canvas()));
	qwt_picker->setStateMachine(new QwtPickerDragPointMachine());
	qwt_picker->setRubberBandPen(QColor(Qt::darkMagenta));
	qwt_picker->setRubberBand(QwtPicker::CrossRubberBand);
	qwt_picker->setTrackerPen(GREEN);
	connect(qwt_picker, SIGNAL(selected(QPointF)), this, SLOT(qwtPlot_selected(QPointF)));

	mysettings	= new QSettings("UDL", "updateDVB");
	myscan		= new scan;
	connect(myscan, SIGNAL(signaldraw(QVector<double>, QVector<double>, int, int, int)), this, SLOT(qwt_draw(QVector<double>, QVector<double>, int, int, int)));
	connect(myscan, SIGNAL(update_status(QString,int)), this, SLOT(update_status(QString,int)));

	noload = true;
	ui->comboBox_lnb->clear();
	for (int a = 0; a < MAX_LNBS; a++) {
		if (mysettings->value("lnb"+QString::number(a)+"_enabled").toBool()) {
			ui->comboBox_lnb->insertItem(a, QString::number(a) + " " + mysettings->value("lnb"+QString::number(a)+"_name").toString(), a);
		}
	}
	if (ui->comboBox_lnb->currentIndex() < 0) {
		ui->comboBox_lnb->insertItem(0, "0");
	}
	ui->comboBox_lnb->setCurrentIndex(0);
	noload = false;
	
	connect(&status_mapper, SIGNAL(mapped(QString)), this, SLOT(update_status(QString)));

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
	}

	delete mysettings;

	delete qwt_picker;
	delete legend;
	
	ui->qwtPlot->deleteLater();
	
	delete ui;
}

void MainWindow::closeEvent(QCloseEvent* ce)
{
	Q_UNUSED(ce);

	for(int i = 0; i < mytuning.size(); i++) {
		if (mytuning.at(i)->shutdown == false) {
			mytuning.at(i)->deleteLater();
		}
	}
}

void MainWindow::qwt_draw(QVector<double> x, QVector<double> y, int min, int max, int cindex)
{
	int lnb = ui->comboBox_lnb->currentData().toInt();
	ui->qwtPlot->setAxisScale(QwtPlot::xBottom, tune_ops[lnb].f_start, tune_ops[lnb].f_stop);
	ui->qwtPlot->setAxisScale(QwtPlot::yLeft, min, max);

	curve[cindex]->setTitle("LNB " + QString::number(lnb) + dvbnames.voltage[cindex]);
	curve[cindex]->attach(ui->qwtPlot);
	curve[cindex]->setSamples(x, y);

	set_colors();
	
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
	
	for (int i = 0; i < mytuning.size(); i++) {
		if (mytuning.at(i)->shutdown) {
			mytuning.remove(i);
		}
	}

	mytuning.append(new tuning);
	mytuning.last()->mytune = mytuners.at(ui->comboBox_adapter->currentIndex());
	mytuning.last()->mytune->tp.frequency	= (int)pos.x();
	mytuning.last()->mytune->tp.voltage		= ui->comboBox_voltage->currentIndex();

	if (ui->gridWidget_system->isVisible()) {
		mytuning.last()->mytune->tp.modulation	= dvbnames.modulation.indexOf(ui->comboBox_modulation->currentText());
		mytuning.last()->mytune->tp.system		= dvbnames.system.indexOf(ui->comboBox_system->currentText());
		mytuning.last()->mytune->tp.symbolrate	= ui->lineEdit_symbolrate->text().toInt();
		mytuning.last()->mytune->tp.fec			= dvbnames.fec.indexOf(ui->comboBox_fec->currentText());
	} else {
		mytuning.last()->mytune->tp.modulation	= QPSK;
		mytuning.last()->mytune->tp.system		= SYS_DVBS;
		mytuning.last()->mytune->tp.symbolrate	= 1000;
		mytuning.last()->mytune->tp.fec			= FEC_AUTO;
	}
	
	mytuning.last()->init();
	mytuning.last()->show();
}

void MainWindow::update_status(QString text, int time)
{
	if (time == -1) {
		if (mystatus.indexOf(text) != -1) {
			mystatus.remove(mystatus.indexOf(text));
		}
	}
	if (time == 0) {
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

	ui->statusBar->showMessage(mystatus.last(), 0);
}

void MainWindow::on_pushButton_spectrumscan_clicked()
{
	if (mytuners.at(ui->comboBox_adapter->currentIndex())->is_tuned) {
		qDebug() << "adapter" << mytuners.at(ui->comboBox_adapter->currentIndex())->adapter << "is currently tuned";
		return;
	}

	myscan->mytune = mytuners.at(ui->comboBox_adapter->currentIndex());

	if (ui->checkBox_fast->isChecked()) {
		myscan->step = 5;
	} else {
		myscan->step = 1;
	}

	myscan->mytune->tune_ops = tune_ops.at(ui->comboBox_lnb->currentData().toInt());
	myscan->mytune->tp.system = dvbnames.system.indexOf(ui->comboBox_system->currentText());
	myscan->loop	= ui->checkBox_loop->isChecked();
	myscan->setup();
	myscan->start();
}

void MainWindow::on_pushButton_blindscan_clicked()
{
	if (mytuners.at(ui->comboBox_adapter->currentIndex())->is_tuned) {
		qDebug() << "adapter" << mytuners.at(ui->comboBox_adapter->currentIndex())->adapter << "is currently tuned";
		return;
	}

	myblindscan.append(new blindscan);
	myblindscan.last()->mytune = mytuners.at(ui->comboBox_adapter->currentIndex());
	myblindscan.last()->init();
	myblindscan.last()->show();

	qam myqam;
	atsc myatsc;

	tp_info tp;
	if (ui->gridWidget_system->isVisible()) {
		mytuners.at(ui->comboBox_adapter->currentIndex())->tp.system		= dvbnames.system.indexOf(ui->comboBox_system->currentText());
		mytuners.at(ui->comboBox_adapter->currentIndex())->tp.modulation	= dvbnames.modulation.indexOf(ui->comboBox_modulation->currentText());
		tp.system		= dvbnames.system.indexOf(ui->comboBox_system->currentText());
		tp.modulation	= dvbnames.modulation.indexOf(ui->comboBox_modulation->currentText());
	} else {
		mytuners.at(ui->comboBox_adapter->currentIndex())->tp.system		= SYS_DVBS;
		mytuners.at(ui->comboBox_adapter->currentIndex())->tp.modulation	= QPSK;
		tp.system		= SYS_DVBS;
		tp.modulation	= QPSK;
	}

	int f_start, f_stop;
	if (abs(mytuners.at(ui->comboBox_adapter->currentIndex())->tune_ops.f_start - abs(mytuners.at(ui->comboBox_adapter->currentIndex())->tune_ops.f_lof)) < abs(mytuners.at(ui->comboBox_adapter->currentIndex())->tune_ops.f_stop - abs(mytuners.at(ui->comboBox_adapter->currentIndex())->tune_ops.f_lof))) {
		f_start	= abs(mytuners.at(ui->comboBox_adapter->currentIndex())->tune_ops.f_start - abs(mytuners.at(ui->comboBox_adapter->currentIndex())->tune_ops.f_lof));
		f_stop	= abs(mytuners.at(ui->comboBox_adapter->currentIndex())->tune_ops.f_stop - abs(mytuners.at(ui->comboBox_adapter->currentIndex())->tune_ops.f_lof));
	} else {
		f_start	= abs(mytuners.at(ui->comboBox_adapter->currentIndex())->tune_ops.f_stop - abs(mytuners.at(ui->comboBox_adapter->currentIndex())->tune_ops.f_lof));
		f_stop	= abs(mytuners.at(ui->comboBox_adapter->currentIndex())->tune_ops.f_start - abs(mytuners.at(ui->comboBox_adapter->currentIndex())->tune_ops.f_lof));
	}

	if (f_start < mytuners.at(ui->comboBox_adapter->currentIndex())->fmin/1000) {
		f_start = mytuners.at(ui->comboBox_adapter->currentIndex())->fmin/1000;
	}
	if (f_start > mytuners.at(ui->comboBox_adapter->currentIndex())->fmax/1000) {
		f_start = mytuners.at(ui->comboBox_adapter->currentIndex())->fmax/1000;
	}
	if (f_stop < mytuners.at(ui->comboBox_adapter->currentIndex())->fmin/1000) {
		f_stop = mytuners.at(ui->comboBox_adapter->currentIndex())->fmin/1000;
	}
	if (f_stop > mytuners.at(ui->comboBox_adapter->currentIndex())->fmax/1000) {
		f_stop = mytuners.at(ui->comboBox_adapter->currentIndex())->fmax/1000;
	}
	if (!abs(f_stop-f_start)) {
		return;
	}

	if (ui->checkBox_smart->isChecked() && mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.size()) {
		switch (dvbnames.system.indexOf(ui->comboBox_system->currentText())) {
		case SYS_DVBC_ANNEX_B:
			qDebug() << "QAM";
			mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.clear();
			for (int i = 1; i < myscan->x.size(); i++) {
				if (myscan->y.at(i) <= myscan->min || myscan->x.at(i) < f_start || myscan->x.at(i) > f_stop) {
					continue;
				}
				if (myqam.freq.indexOf(myscan->x.at(i)) != -1) { // Quick search
					tp.frequency = myscan->x.at(i);
					mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.append(tp);
				} else { // Long search
					for (int ai = 0; ai < myqam.freq.size(); ai++) {
						if (abs(myqam.freq.at(ai) - (int)myscan->x.at(i)) < 3000) {
							if (tp.frequency != myqam.freq.at(ai)) {
								tp.frequency = myqam.freq.at(ai);
								mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.append(tp);
							}
						}
					}
				}
			}
			break;
		case SYS_ATSC:
		case SYS_ATSCMH:
			qDebug() << "ATSC";
			mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.clear();
			for (int i = 1; i < myscan->x.size(); i++) {
				if (myscan->y.at(i) <= myscan->min || myscan->x.at(i) < f_start || myscan->x.at(i) > f_stop) {
					continue;
				}
				if (myatsc.freq.indexOf(myscan->x.at(i)) != -1) { // Quick search
					tp.frequency = myscan->x.at(i);
					mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.append(tp);
				} else { // Long search
					for (int ai = 0; ai < myatsc.freq.size(); ai++) {
						if (abs(myatsc.freq.at(ai) - (int)myscan->x.at(i)) < 3000) {
							if (tp.frequency != myatsc.freq.at(ai)) {
								tp.frequency = myatsc.freq.at(ai);
								mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.append(tp);
							}
						}
					}
				}
			}
			break;
		}
		myblindscan.last()->smartscan();
	} else {
		switch (dvbnames.system.indexOf(ui->comboBox_system->currentText())) {
		case SYS_DVBC_ANNEX_B:
			qDebug() << "QAM";
			mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.clear();
			for (int i = 0; i < myqam.freq.size(); i++) {
				if (myqam.freq.at(i) >= f_start && myqam.freq.at(i) <= f_stop) {
					tp.frequency	= myqam.freq.at(i);
					mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.append(tp);
				}
			}
			myblindscan.last()->smartscan();
			break;
		case SYS_ATSC:
		case SYS_ATSCMH:
			qDebug() << "ATSC";
			mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.clear();
			for (int i = 0; i < myatsc.freq.size(); i++) {
				if (myatsc.freq.at(i) >= f_start && myatsc.freq.at(i) <= f_stop) {
					tp.frequency	= myatsc.freq.at(i);
					mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.append(tp);
				}
			}
			myblindscan.last()->smartscan();
			break;
		default:
			myblindscan.last()->scan();
			break;
		}
	}
}

void MainWindow::on_pushButton_usals_go_clicked()
{
	mytuners.at(ui->comboBox_adapter->currentIndex())->old_position = mysettings->value("adapter" + QString::number(ui->comboBox_adapter->currentData().toInt()) + "_usals_position").toDouble();
	mytuners.at(ui->comboBox_adapter->currentIndex())->usals_drive(ui->lineEdit_usals->text().toDouble());
	mysettings->setValue("adapter"+QString::number(ui->comboBox_adapter->currentData().toInt())+"_usals_position", ui->lineEdit_usals->text().toDouble());
}

void MainWindow::on_pushButton_gotox_go_clicked()
{
	mytuners.at(ui->comboBox_adapter->currentIndex())->gotox_drive(ui->comboBox_gotox->currentIndex());
}

void MainWindow::on_pushButton_gotox_save_clicked()
{
	mytuners.at(ui->comboBox_adapter->currentIndex())->gotox_save(ui->comboBox_gotox->currentIndex());
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

void MainWindow::on_comboBox_adapter_currentIndexChanged(int index)
{
	if (index < 0) {
		return;
	}

	ui->comboBox_frontend->clear();
	QDir adapter_dir("/dev/dvb/adapter" + ui->comboBox_adapter->currentData().toString());
	adapter_dir.setFilter(QDir::System|QDir::NoDotAndDotDot);
	QStringList frontend_entries = adapter_dir.entryList();
	for(QStringList::ConstIterator frontend_entry = frontend_entries.begin(); frontend_entry != frontend_entries.end(); frontend_entry++) {
		QString frontend_dirname = *frontend_entry;
		if (frontend_dirname.contains("frontend")) {
			frontend_dirname.replace("frontend", "");
			ui->comboBox_frontend->addItem(frontend_dirname);
		}
	}

	reload_settings();
}

void MainWindow::on_comboBox_frontend_currentIndexChanged(int index)
{
	if (index < 0) {
		return;
	}
	setup_tuning_options();
}

void MainWindow::on_comboBox_lnb_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	if (noload) {
		return;
	}

	mytuners.at(ui->comboBox_adapter->currentIndex())->tune_ops = tune_ops[ui->comboBox_lnb->currentData().toInt()];
	reload_settings();
}

void MainWindow::on_comboBox_voltage_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	if (mytuners.size() == 0) {
		return;
	}
	set_colors();
}

void MainWindow::on_lineEdit_usals_returnPressed()
{
	on_pushButton_usals_go_clicked();
}

void MainWindow::on_checkBox_loop_stateChanged()
{
	myscan->loop = ui->checkBox_loop->isChecked();
}

void MainWindow::on_actionSettings_triggered()
{
	settings settings_dialog;
	settings_dialog.mytuners = mytuners;
	settings_dialog.setModal(true);
	settings_dialog.exec();

	noload = true;
	ui->comboBox_lnb->clear();
	for (int a = 0; a < MAX_LNBS; a++) {
		if (mysettings->value("lnb"+QString::number(a)+"_enabled").toBool()) {
			ui->comboBox_lnb->insertItem(a, QString::number(a) + " " + mysettings->value("lnb"+QString::number(a)+"_name").toString(), a);
		}
	}
	if (ui->comboBox_lnb->currentIndex() < 0) {
		ui->comboBox_lnb->insertItem(0, "0");
	}
	ui->comboBox_lnb->setCurrentIndex(0);
	noload = false;
	
	reload_settings();
}

void MainWindow::on_actionExit_triggered()
{
	qDebug() << "on_actionExit_triggered()";
	close();
}

void MainWindow::adapter_status(int adapter, bool is_busy)
{
	if (is_busy) {
		ui->comboBox_adapter->setItemText(adapter, QString("%1 Busy").arg(adapter));
		ui->comboBox_adapter->setItemData(adapter, QColor(Qt::red), Qt::TextColorRole);
	} else {
		ui->comboBox_adapter->setItemText(adapter, QString::number(adapter) + " " + mysettings->value("adapter" + QString::number(adapter) + "_name").toString());
		ui->comboBox_adapter->setItemData(adapter, QColor(Qt::black), Qt::TextColorRole);
	}
}

void MainWindow::setup_tuning_options()
{
	mytuners.at(ui->comboBox_adapter->currentIndex())->tune_ops = tune_ops[ui->comboBox_lnb->currentData().toInt()];
	mytuners.at(ui->comboBox_adapter->currentIndex())->closefd();
	mytuners.at(ui->comboBox_adapter->currentIndex())->frontend	= ui->comboBox_frontend->currentText().toInt();
	mytuners.at(ui->comboBox_adapter->currentIndex())->getops();

	if (mysettings->value("adapter" + QString::number(ui->comboBox_adapter->currentData().toInt()) + "_diseqc_v12").toBool()) {
		ui->gridWidget_gotox->show();
	} else {
		ui->gridWidget_gotox->hide();
	}
	if (mysettings->value("adapter" + QString::number(ui->comboBox_adapter->currentData().toInt()) + "_diseqc_v13").toBool()) {
		ui->gridWidget_usals->show();
	} else {
		ui->gridWidget_usals->hide();
	}
	if (mysettings->value("adapter" + QString::number(ui->comboBox_adapter->currentData().toInt()) + "_diseqc_v12").toBool() || mysettings->value("adapter" + QString::number(ui->comboBox_adapter->currentData().toInt()) + "_diseqc_v13").toBool()) {
		ui->gridWidget_positioner->show();
	} else {
		ui->gridWidget_positioner->hide();
	}

	update_status(mytuners.at(ui->comboBox_adapter->currentIndex())->name, 0);

	if (mytuners.at(ui->comboBox_adapter->currentIndex())->caps & FE_CAN_SPECTRUMSCAN) {
		ui->gridWidget_spectrumscan->show();
	} else {
		ui->gridWidget_spectrumscan->hide();
	}

	ui->comboBox_system->clear();
	if (mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.size()) {
		for(int i = 0; i < mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.size(); i++) {
			ui->comboBox_system->addItem(dvbnames.system.at(mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.at(i)));
		}
	} else { // This should never happen, just in case some driver is seriously messed up
		ui->comboBox_system->addItem("DVB-S");
		ui->comboBox_system->addItem("DVB-S2");
		ui->comboBox_system->addItem("ATSC");
		ui->comboBox_system->addItem("DVB-C B");
	}

	ui->comboBox_modulation->clear();
	if (mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.indexOf(SYS_ATSC) != -1 || mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.indexOf(SYS_ATSCMH) != -1) {
		add_comboBox_modulation("VSB 8");
	}
	if (mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.indexOf(SYS_DVBC_ANNEX_A) != -1) { // DVB-C
		add_comboBox_modulation("QAM 16");
		add_comboBox_modulation("QAM 32");
		add_comboBox_modulation("QAM 64");
		add_comboBox_modulation("QAM 128");
		add_comboBox_modulation("QAM 256");
		add_comboBox_modulation("QAM AUTO");
	}
	if (mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.indexOf(SYS_DVBC_ANNEX_B) != -1) { // NA Cable
		add_comboBox_modulation("QAM 64");
		add_comboBox_modulation("QAM 256");
	}
	if (mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.indexOf(SYS_DVBT) != -1) {
		add_comboBox_modulation("QPSK");
		add_comboBox_modulation("QAM 16");
		add_comboBox_modulation("QAM 64");
	}
	if (mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.indexOf(SYS_DVBT2) != -1) {
		add_comboBox_modulation("QPSK");
		add_comboBox_modulation("QAM 16");
		add_comboBox_modulation("QAM 64");
		add_comboBox_modulation("QAM 256");
	}
	if (mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.indexOf(SYS_DCII) != -1) {
		add_comboBox_modulation("C QPSK");
		add_comboBox_modulation("I QPSK");
		add_comboBox_modulation("Q QPSK");
		add_comboBox_modulation("C OQPSK");
	}
	if (mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.indexOf(SYS_DVBS2) != -1 || mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.indexOf(SYS_TURBO) != -1) {
		add_comboBox_modulation("QPSK");
		add_comboBox_modulation("8PSK");
	}
	if (mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.indexOf(SYS_DVBS) != -1) {
		add_comboBox_modulation("QPSK");
	}

	ui->comboBox_fec->clear();
	ui->comboBox_fec->addItem("Auto");

	if (mytuners.at(ui->comboBox_adapter->currentIndex())->caps & FE_CAN_BLINDSEARCH) {
		ui->gridWidget_system->hide();
		ui->gridWidget_blindscan->show();
		ui->gridWidget_satellite->hide();
	} else {
		ui->gridWidget_system->show();
		ui->gridWidget_blindscan->hide();

		if (mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.indexOf(SYS_ATSC) != -1) {
			ui->gridWidget_blindscan->show();
			ui->gridWidget_satellite->hide();
		} else {
			ui->gridWidget_satellite->show();
		}
	}
}

void MainWindow::getadapters()
{
	QVector<int> adaps;
	QDir dvb_dir("/dev/dvb");
	dvb_dir.setFilter(QDir::Dirs|QDir::NoDotAndDotDot);
	QStringList adapter_entries = dvb_dir.entryList();
	for(QStringList::ConstIterator adapter_entry = adapter_entries.begin(); adapter_entry != adapter_entries.end(); adapter_entry++) {
		QString adapter_dirname = *adapter_entry;
		adapter_dirname.replace("adapter", "");
		adaps.append(adapter_dirname.toInt());
	}
	qSort(adaps);

	for (int i = 0; i < adaps.size(); i++) {
		mytuners.append(new dvbtune);
		connect(mytuners.last(), SIGNAL(adapter_status(int,bool)), this, SLOT(adapter_status(int,bool)));
		mytuners.last()->adapter	= adaps.at(i);
		mytuners.last()->frontend	= ui->comboBox_frontend->currentText().toInt();
		mytuners.last()->tune_ops	= tune_ops[ui->comboBox_lnb->currentData().toInt()];
		mytuners.last()->getops();
	}

	if (!mytuners.size()) {
		return;
	}
}

void MainWindow::reload_settings()
{
	if (noload) {
		return;
	}

	mysettings->sync();
	tune_ops.clear();
	tuning_options tmp;

	for (int a = 0; a < MAX_LNBS; a++) {
		tmp.f_lof		= mysettings->value("lnb"+QString::number(a)+"_freqlof").toInt();
		tmp.f_start		= mysettings->value("lnb"+QString::number(a)+"_freqstart").toInt();
		tmp.f_stop		= mysettings->value("lnb"+QString::number(a)+"_freqstop").toInt();
		tmp.voltage		= mysettings->value("lnb"+QString::number(a)+"_voltage").toInt();
		tmp.tone		= mysettings->value("lnb"+QString::number(a)+"_tone").toBool();
//		tmp.mis			= mysettings->value("lnb"+QString::number(a)+"_mis").toInt();
		tmp.mis			= -1;
		tmp.committed	= mysettings->value("lnb"+QString::number(a)+"_committed").toInt();
		tmp.uncommitted	= mysettings->value("lnb"+QString::number(a)+"_uncommitted").toInt();
		tmp.site_lat	= mysettings->value("site_lat").toDouble();
		tmp.site_long	= mysettings->value("site_long").toDouble();
		tmp.name		= mysettings->value("name").toString();
		tune_ops.append(tmp);
	}

	if (ui->comboBox_adapter->currentData().toInt() < 0) {
		return;
	}

	if (mytuners.size()) {
		noload = true;
		int current_adap = 0;
		if (ui->comboBox_adapter->currentIndex() >= 0) {
			current_adap = ui->comboBox_adapter->currentIndex();
		}
		ui->comboBox_adapter->clear();
		for (int i = 0; i < mytuners.size(); i++) {
			mytuners.at(i)->servo = mysettings->value("adapter" + QString::number(mytuners.at(i)->adapter) + "_servo").toBool();
			ui->comboBox_adapter->insertItem(i, QString::number(mytuners.at(i)->adapter) + " " + mysettings->value("adapter" + QString::number(mytuners.at(i)->adapter) + "_name").toString(), mytuners.at(i)->adapter);
		}
		ui->comboBox_adapter->setCurrentIndex(current_adap);
		setup_tuning_options();
		noload = false;
	}

	int gotox_i = ui->comboBox_gotox->currentIndex();
	ui->comboBox_gotox->clear();
	ui->comboBox_gotox->addItem("");
	for (int i = 1; i < 256; i++) {
		QString text = mysettings->value("adapter"+QString::number(ui->comboBox_adapter->currentData().toInt())+"_diseqc_v12_name_"+QString::number(i)).toString();
		if (text != "") {
			ui->comboBox_gotox->addItem(text);
		}
	}
	ui->comboBox_gotox->setCurrentIndex(gotox_i);

	QVariant d(0);
	QVariant e(1|32);
	qDebug() << "Adapter:" << ui->comboBox_adapter->currentData().toInt() << "lnb:" << ui->comboBox_lnb->currentData().toInt() << "Voltage setting:" << tune_ops[ui->comboBox_lnb->currentData().toInt()].voltage;
	switch(tune_ops[ui->comboBox_lnb->currentData().toInt()].voltage) {
	case 0:
		ui->gridWidget_voltage->hide();
		ui->comboBox_voltage->setCurrentIndex(0);
		ui->comboBox_voltage->setItemData(0, e, Qt::UserRole -1);
		ui->comboBox_voltage->setItemData(1, d, Qt::UserRole -1);
		ui->comboBox_voltage->setItemData(2, d, Qt::UserRole -1);
		break;
	case 1:
		ui->gridWidget_voltage->hide();
		ui->comboBox_voltage->setCurrentIndex(1);
		ui->comboBox_voltage->setItemData(0, d, Qt::UserRole -1);
		ui->comboBox_voltage->setItemData(1, e, Qt::UserRole -1);
		ui->comboBox_voltage->setItemData(2, d, Qt::UserRole -1);
		break;
	case 2:
		ui->gridWidget_voltage->hide();
		ui->comboBox_voltage->setCurrentIndex(2);
		ui->comboBox_voltage->setItemData(0, d, Qt::UserRole -1);
		ui->comboBox_voltage->setItemData(1, d, Qt::UserRole -1);
		ui->comboBox_voltage->setItemData(2, e, Qt::UserRole -1);
		break;
	case 3:
		ui->gridWidget_voltage->show();
		ui->comboBox_voltage->setCurrentIndex(1);
		ui->comboBox_voltage->setItemData(0, e, Qt::UserRole -1);
		ui->comboBox_voltage->setItemData(1, e, Qt::UserRole -1);
		ui->comboBox_voltage->setItemData(2, d, Qt::UserRole -1);
		break;
	}

	ui->qwtPlot->setAxisScale(QwtPlot::xBottom, tune_ops[ui->comboBox_lnb->currentData().toInt()].f_start, tune_ops[ui->comboBox_lnb->currentData().toInt()].f_stop);
	ui->qwtPlot->replot();
}

void MainWindow::add_comboBox_modulation(QString name)
{
	if (ui->comboBox_modulation->findText(name) < 0) {
		ui->comboBox_modulation->addItem(name);
	}
}

void MainWindow::set_colors()
{
	curve.at(0)->detach();
	curve.at(1)->detach();
	switch(ui->comboBox_voltage->currentIndex())
	{
	case 0:
		curve[1]->setPen(QPen(DGREEN));
		curve[1]->attach(ui->qwtPlot);
		curve[0]->setPen(QPen(GREEN));
		curve[0]->attach(ui->qwtPlot);
		break;
	case 1:
		curve[0]->setPen(QPen(DGREEN));
		curve[0]->attach(ui->qwtPlot);
		curve[1]->setPen(QPen(GREEN));
		curve[1]->attach(ui->qwtPlot);
		break;
	}

	for (int i = 0; i < marker.size(); i++) {
		marker.at(i)->detach();
	}
	marker.clear();

	for (int i = 0; i < mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.size(); i++) {
		marker.append(new QwtPlotMarker);
		QwtText text = QString::number(mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.at(i).frequency);
		if (mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.at(i).voltage == ui->comboBox_voltage->currentIndex()) {
			marker.at(i)->setSymbol(new QwtSymbol(QwtSymbol::Diamond, QBrush(GREEN), QPen(GREEN), QSize(5,5)));
			text.setColor(GREEN);
		} else {
			marker.at(i)->setSymbol(new QwtSymbol(QwtSymbol::Diamond, QBrush(DGREEN), QPen(DGREEN), QSize(5,5)));
			text.setColor(DGREEN);
		}
		marker.at(i)->setLabel(text);
		marker.at(i)->setLabelOrientation(Qt::Vertical);
		marker.at(i)->setLabelAlignment(Qt::AlignTop);
		marker.at(i)->setValue(mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.at(i).frequency, mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.at(i).spectrumscan_lvl);
		marker.at(i)->attach(ui->qwtPlot);
	}
	ui->qwtPlot->replot();
}
