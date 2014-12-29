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

	status_timer = new QTimer;

	curve.append(new QwtPlotCurve("V"));
	curve.append(new QwtPlotCurve("H"));
	curve.append(new QwtPlotCurve("N"));

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
	connect(myscan, SIGNAL(signaldraw(QVector<double>, QVector<double>, int, int, int, unsigned int)), this, SLOT(qwt_draw(QVector<double>, QVector<double>, int, int, int, unsigned int)));
	connect(myscan, SIGNAL(update_status(QString,int)), this, SLOT(update_status(QString,int)));
	connect(myscan, SIGNAL(markers_draw()), this, SLOT(markers_draw()));

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
	noload = false;

	connect(&status_mapper, SIGNAL(mapped(QString)), this, SLOT(update_status(QString)));

	reload_settings();
	getadapters();
	reload_settings();

	if (mytuners.size()) {
		noload = true;
		ui->comboBox_adapter->clear();
		for (int i = 0; i < mytuners.size(); i++) {
			mytuners.at(i)->servo = mysettings->value("adapter" + QString::number(mytuners.at(i)->adapter) + "_servo").toBool();
			ui->comboBox_adapter->insertItem(mytuners.at(i)->adapter, QString::number(mytuners.at(i)->adapter) + " " + mysettings->value("adapter" + QString::number(mytuners.at(i)->adapter) + "_name").toString(), mytuners.at(i)->adapter);
		}
		ui->comboBox_adapter->setCurrentIndex(0);
		noload = false;
		setup_tuning_options();
	}
}

MainWindow::~MainWindow()
{
	myscan->loop = false;
	myscan->quit();
	myscan->wait(1000);
	while (myscan->isRunning()) {
		myscan->loop = false;
		QThread::msleep(100);
	}
	delete myscan;
	
	for(int i = 0; i < mytuners.size(); i++) {
		mytuners.at(i)->loop = false;
		mytuners.at(i)->quit();
		mytuners.at(i)->wait(1000);
		while (mytuners.at(i)->isRunning()) {
			mytuners.at(i)->loop = false;
			QThread::msleep(100);
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

	status_timer->deleteLater();

	delete mysettings;
	delete qwt_picker;
	delete legend;
	
	ui->qwtPlot->deleteLater();
	
	delete ui;
}

void MainWindow::clear_qwtplot()
{
	for(int i = 0; i < curve.size(); i++) {
		curve.at(i)->detach();
	}
	for(int i = 0; i < marker.size(); i++) {
		marker.at(i)->detach();
	}
	marker.clear();
	for(int i = 0; i < waterfall_curve_V.size(); i++) {
		waterfall_curve_V.at(i)->detach();
	}
	waterfall_curve_V.clear();
	for(int i = 0; i < waterfall_curve_H.size(); i++) {
		waterfall_curve_H.at(i)->detach();
	}
	waterfall_curve_H.clear();
	for(int i = 0; i < waterfall_curve_N.size(); i++) {
		waterfall_curve_N.at(i)->detach();
	}
	waterfall_curve_N.clear();
	waterfall_x_V.clear();
	waterfall_x_H.clear();
	waterfall_x_N.clear();
	waterfall_y_V.clear();
	waterfall_y_H.clear();
	waterfall_y_N.clear();
}

void MainWindow::closeEvent(QCloseEvent* ce)
{
	Q_UNUSED(ce);

	for (int i = 0; i < mytuning.size(); i++) {
		if (!mytuning.at(i).isNull()) {
			mytuning.at(i)->deleteLater();
		}
	}
}

void MainWindow::qwt_draw(QVector<double> x, QVector<double> y, int min, int max, int cindex, unsigned int scale)
{
	int lnb = ui->comboBox_lnb->currentData().toInt();

	ui->qwtPlot->setAxisScale(QwtPlot::xBottom, tune_ops[lnb].f_start, tune_ops[lnb].f_stop);
	ui->qwtPlot->setAxisScale(QwtPlot::yLeft, min, max);

	QVector< QwtPlotCurve* > *waterfall_curve;
	QVector< QVector<double> > *waterfall_x;
	QVector< QVector<double> > *waterfall_y;

	switch (cindex) {
	case 0:
		waterfall_curve = &waterfall_curve_V;
		waterfall_x		= &waterfall_x_V;
		waterfall_y		= &waterfall_y_V;
		break;
	case 1:
		waterfall_curve = &waterfall_curve_H;
		waterfall_x		= &waterfall_x_H;
		waterfall_y		= &waterfall_y_H;
		break;
	case 2:
	default:
		waterfall_curve = &waterfall_curve_N;
		waterfall_x		= &waterfall_x_N;
		waterfall_y		= &waterfall_y_N;
		break;
	}

	QString scale_text;
	switch (scale) {
	case SC_DB:
		scale_text = "dB (dB x 100)";
		break;
	case SC_DBM:
		scale_text = "dBm (dB x 100)";
		break;
	case SC_GAIN:
		scale_text = "AGC Gain (0 - (dB x 100)";
		break;
	default:
		break;
	}

	if (ui->checkBox_waterfall->isChecked()) {
		double max_waterfall = ui->comboBox_waterfall_points->currentText().toInt();
		double scale_x = abs(tune_ops[lnb].f_start - tune_ops[lnb].f_stop)/max_waterfall/3;
		double scale_y = (max-min)/max_waterfall/3;

		waterfall_x->prepend(x);
		if (waterfall_x->size() > max_waterfall) {
			waterfall_x->removeLast();
		}
		waterfall_y->prepend(y);
		if (waterfall_y->size() > max_waterfall) {
			waterfall_y->removeLast();
		}
		waterfall_curve->prepend(new QwtPlotCurve());
		if (waterfall_curve->size() > max_waterfall) {
			waterfall_curve->last()->detach();
			waterfall_curve->removeLast();
		}
		waterfall_curve->first()->setItemAttribute(QwtPlotItem::Legend, true);
		waterfall_curve->first()->setTitle(mysettings->value("lnb" + ui->comboBox_lnb->currentData().toString() + "_name").toString() + " -" + dvbnames.voltage[cindex]);
		waterfall_curve->first()->setSamples(waterfall_x->first(), waterfall_y->first());
		waterfall_curve->first()->attach(ui->qwtPlot);

		for (int c = 1; c < waterfall_curve->size(); c++) {
			QVector<double> new_x = waterfall_x->at(c);
			QVector<double> new_y = waterfall_y->at(c);
			for (int i = 0; i < new_x.size(); i++) {
				new_x[i] += scale_x * c;
				new_y[i] += scale_y * c;
			}
			waterfall_curve->at(c)->setItemAttribute(QwtPlotItem::Legend, false);
			waterfall_curve->at(c)->setSamples(new_x, new_y);
			waterfall_curve->at(c)->attach(ui->qwtPlot);
		}
	} else {
		curve[cindex]->setTitle(mysettings->value("lnb" + ui->comboBox_lnb->currentData().toString() + "_name").toString() + " -" + dvbnames.voltage[cindex]);
		curve[cindex]->setSamples(x, y);
	}
	ui->qwtPlot->setAxisTitle(QwtPlot::yLeft, scale_text);
	set_colors();

	if (mysettings->value("adapter" + ui->comboBox_adapter->currentData().toString() + "_save_images").toBool() && ui->checkBox_loop->isChecked() && !ui->checkBox_waterfall->isChecked()) {
		on_actionSave_Screenshot_triggered();
	}
}

void MainWindow::qwtPlot_selected(QPointF pos)
{
	if (!mytuners.at(ui->comboBox_adapter->currentIndex())->openfd()) {
		return;
	}
	if (myscan->loop) {
		update_status("Please wait for spectrum scan to finish first", 1);
		return;
	}

	if (mytuners.at(ui->comboBox_adapter->currentIndex())->loop) {
		update_status("Tuner is currently busy", 1);
		return;
	}

	for (int i = 0; i < mytuning.size(); i++) {
		if (mytuning.at(i).isNull()) {
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
	
	connect(mytuning.last(), SIGNAL(adapter_status(int)), this, SLOT(adapter_status(int)));
	mytuning.last()->init();
	mytuning.last()->show();
}

void MainWindow::update_status(QString text, int time)
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
		status_timer->setSingleShot(true);
		connect(status_timer, SIGNAL(timeout()), &status_mapper, SLOT(map()));
		status_mapper.setMapping(status_timer, text);
		status_timer->start(time*1000);
		mystatus.append(text);
	}

	if (mystatus.size()) {
		ui->statusBar->showMessage(mystatus.last(), 0);
	} else {
		ui->statusBar->showMessage("", 0);
	}
}

void MainWindow::on_pushButton_spectrumscan_clicked()
{
	if (!mytuners.at(ui->comboBox_adapter->currentIndex())->openfd()) {
		return;
	}

	clear_qwtplot();

	myscan->mytune = mytuners.at(ui->comboBox_adapter->currentIndex());

	if (ui->checkBox_fast->isChecked()) {
		myscan->step = 5;
	} else {
		myscan->step = 1;
	}

	myscan->mytune->tune_ops	= tune_ops.at(ui->comboBox_lnb->currentData().toInt());
	myscan->mytune->tp.system	= dvbnames.system.indexOf(ui->comboBox_system->currentText());
	myscan->loop				= ui->checkBox_loop->isChecked();
	myscan->loop_delay			= mysettings->value("adapter" + QString::number(ui->comboBox_adapter->currentData().toInt()) + "_loop_delay").toInt();
	myscan->setup();
	myscan->start();
}

void MainWindow::on_pushButton_blindscan_clicked()
{
	if (!mytuners.at(ui->comboBox_adapter->currentIndex())->openfd()) {
		return;
	}

	myblindscan.append(new blindscan);
	myblindscan.last()->mytune = mytuners.at(ui->comboBox_adapter->currentIndex());
	myblindscan.last()->init();
	myblindscan.last()->show();

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

	freq_list myfreq;
	if (ui->checkBox_smart->isChecked() && mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.size()) {
		if (!isSatellite(dvbnames.system.indexOf(ui->comboBox_system->currentText()))) {
			if (isQAM(dvbnames.system.indexOf(ui->comboBox_system->currentText()))) {
				qDebug() << "QAM";
				myfreq.qam();
			} else if (isATSC(dvbnames.system.indexOf(ui->comboBox_system->currentText()))) {
				myfreq.atsc();
				qDebug() << "ATSC";
			} else if (isDVBT(dvbnames.system.indexOf(ui->comboBox_system->currentText()))) {
				myfreq.dvbt();
				qDebug() << "DVBT";
			}
			mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.clear();
			for (int i = 1; i < myscan->x.size(); i++) {
				if (myscan->y.at(i) <= myscan->min || myscan->x.at(i) < f_start || myscan->x.at(i) > f_stop) {
					continue;
				}
				if (myfreq.freq.contains(myscan->x.at(i))) { // Quick search
					tp.frequency = myscan->x.at(i);
					mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.append(tp);
				} else { // Long search
					for (int ai = 0; ai < myfreq.freq.size(); ai++) {
						if (abs(myfreq.freq.at(ai) - (int)myscan->x.at(i)) < 3000) {
							if (tp.frequency != myfreq.freq.at(ai)) {
								tp.frequency = myfreq.freq.at(ai);
								mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.append(tp);
							}
						}
					}
				}
			}
		}
		myblindscan.last()->smartscan();
	} else {
		if (!isSatellite(dvbnames.system.indexOf(ui->comboBox_system->currentText()))) {
			if (isQAM(dvbnames.system.indexOf(ui->comboBox_system->currentText()))) {
				qDebug() << "QAM";
				myfreq.qam();
			} else if (isATSC(dvbnames.system.indexOf(ui->comboBox_system->currentText()))) {
				qDebug() << "ATSC";
				myfreq.atsc();
			} else if (isDVBT(dvbnames.system.indexOf(ui->comboBox_system->currentText()))) {
				qDebug() << "DVBT";
				myfreq.dvbt();
			}
			mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.clear();
			for (int i = 0; i < myfreq.freq.size(); i++) {
				if (myfreq.freq.at(i) >= f_start && myfreq.freq.at(i) <= f_stop) {
					tp.frequency	= myfreq.freq.at(i);
					mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.append(tp);
				}
			}
			myblindscan.last()->smartscan();
		} else {
			myblindscan.last()->scan();
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
	mytuners.at(ui->comboBox_adapter->currentIndex())->gotox_drive(ui->comboBox_gotox->currentData().toInt());
}

void MainWindow::on_pushButton_gotox_save_clicked()
{
	mytuners.at(ui->comboBox_adapter->currentIndex())->gotox_save(ui->comboBox_gotox->currentData().toInt());
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

void MainWindow::on_comboBox_adapter_activated(int index)
{
	if (index < 0) {
		return;
	}

	ui->comboBox_frontend->clear();
	QDir adapter_dir("/dev/dvb/adapter" + ui->comboBox_adapter->currentData().toString());
	adapter_dir.setFilter(QDir::System|QDir::NoDotAndDotDot);
	QStringList frontend_entries = adapter_dir.entryList();
	frontend_entries = frontend_entries.filter("frontend");
	for(int i = 0; i < frontend_entries.size(); i++) {
		QString frontend_name = frontend_entries.at(i);
		frontend_name.replace("frontend", "");
		ui->comboBox_frontend->insertItem(frontend_name.toInt(), frontend_name + " " + mysettings->value("adapter" + ui->comboBox_adapter->currentData().toString() + "_frontend" + frontend_name + "_name").toString(), frontend_name.toInt());
	}

	reload_settings();

	for (int i = 0; i < mytuning.size(); i++) {
		if (!mytuning.at(i).isNull() && mytuning.at(i)->mytune->adapter == ui->comboBox_adapter->currentData().toUInt()) {
			mytuning.at(i)->raise();
		}
	}
	for (int i = 0; i < mytuners.size(); i++) {
		if (!(mytuners.at(i)->status & TUNER_AVAIL)) {
			mytuners.at(i)->closefd();
		}
	}

	ui->qwtPlot->setAxisTitle(QwtPlot::yLeft, "Amplitude");
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

	clear_qwtplot();
	ui->qwtPlot->replot();
}

void MainWindow::on_comboBox_voltage_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	if (mytuners.isEmpty()) {
		return;
	}
	set_colors();
	markers_draw();
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
	noload = false;
	ui->comboBox_lnb->setCurrentIndex(mysettings->value("adapter"+ui->comboBox_adapter->currentData().toString()+"_default_lnb").toInt());

	reload_settings();
	setup_tuning_options();
}

void MainWindow::on_actionExit_triggered()
{
	close();
}

void MainWindow::adapter_status(int adapter)
{
	int index = ui->comboBox_adapter->findData(adapter);
	if (index < 0) {
		return;
	}
	if (mytuners.at(index)->status & TUNER_AVAIL) {
		ui->comboBox_adapter->setItemText(index, QString::number(adapter) + " " + mysettings->value("adapter" + QString::number(adapter) + "_name").toString());
		ui->comboBox_adapter->setItemData(index, QColor(Qt::black), Qt::TextColorRole);
	} else {
		ui->comboBox_adapter->setItemText(index, QString::number(adapter) + " Unavailible");
		ui->comboBox_adapter->setItemData(index, QColor(Qt::red), Qt::TextColorRole);
	}
	if (mytuners.at(index)->status & TUNER_TUNED) {
		ui->comboBox_adapter->setItemText(index, QString::number(adapter) + " Tuned");
		ui->comboBox_adapter->setItemData(index, QColor(Qt::red), Qt::TextColorRole);
	}
	if (mytuners.at(index)->status & TUNER_DEMUX) {
		ui->comboBox_adapter->setItemText(index, QString::number(adapter) + " Busy");
		ui->comboBox_adapter->setItemData(index, QColor(Qt::red), Qt::TextColorRole);
	}
}

void MainWindow::setup_tuning_options()
{
	mytuners.at(ui->comboBox_adapter->currentIndex())->tune_ops = tune_ops[ui->comboBox_lnb->currentData().toInt()];
	mytuners.at(ui->comboBox_adapter->currentIndex())->frontend	= ui->comboBox_frontend->currentData().toInt();

	if (!mytuners.at(ui->comboBox_adapter->currentIndex())->openfd()) {
		return;
	}
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

	update_status(mytuners.at(ui->comboBox_adapter->currentIndex())->name, STATUS_NOEXP);

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
	if (mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.contains(SYS_ATSC) || mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.contains(SYS_ATSCMH)) {
		add_comboBox_modulation("VSB 8");
	}
	if (mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.contains(SYS_DVBC_ANNEX_A)) { // DVB-C
		add_comboBox_modulation("QAM 16");
		add_comboBox_modulation("QAM 32");
		add_comboBox_modulation("QAM 64");
		add_comboBox_modulation("QAM 128");
		add_comboBox_modulation("QAM 256");
		add_comboBox_modulation("QAM AUTO");
	}
	if (mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.contains(SYS_DVBC_ANNEX_B)) { // NA Cable
		add_comboBox_modulation("QAM 64");
		add_comboBox_modulation("QAM 256");
	}
	if (mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.contains(SYS_DVBT)) {
		add_comboBox_modulation("QPSK");
		add_comboBox_modulation("QAM 16");
		add_comboBox_modulation("QAM 64");
	}
	if (mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.contains(SYS_DVBT2)) {
		add_comboBox_modulation("QPSK");
		add_comboBox_modulation("QAM 16");
		add_comboBox_modulation("QAM 64");
		add_comboBox_modulation("QAM 256");
	}
	if (mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.contains(SYS_DCII)) {
		add_comboBox_modulation("C QPSK");
		add_comboBox_modulation("I QPSK");
		add_comboBox_modulation("Q QPSK");
		add_comboBox_modulation("C OQPSK");
	}
	if (mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.contains(SYS_DVBS2) || mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.contains(SYS_TURBO)) {
		add_comboBox_modulation("QPSK");
		add_comboBox_modulation("8PSK");
	}
	if (mytuners.at(ui->comboBox_adapter->currentIndex())->delsys.contains(SYS_DVBS)) {
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
		if (isVectorQAM(mytuners.at(ui->comboBox_adapter->currentIndex())->delsys) || isVectorATSC(mytuners.at(ui->comboBox_adapter->currentIndex())->delsys) || isVectorDVBT(mytuners.at(ui->comboBox_adapter->currentIndex())->delsys)) {
			ui->gridWidget_blindscan->show();
			ui->gridWidget_satellite->hide();
		} else {
			ui->gridWidget_satellite->show();
		}
	}

	if (!noload && mysettings->value("lnb"+mysettings->value("adapter"+ui->comboBox_adapter->currentData().toString()+"_default_lnb").toString()+"_enabled").toBool()) {
		ui->comboBox_lnb->setCurrentIndex(mysettings->value("adapter"+ui->comboBox_adapter->currentData().toString()+"_default_lnb").toInt());
	}

	qDebug() << "Adapter:" << ui->comboBox_adapter->currentData().toInt() << "Frontend:" << ui->comboBox_frontend->currentData().toInt() << "lnb:" << ui->comboBox_lnb->currentData().toInt() << "Voltage setting:" << dvbnames.voltage[tune_ops[ui->comboBox_lnb->currentData().toInt()].voltage];
}

void MainWindow::getadapters()
{
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
		mytuners.append(new dvbtune);
		connect(mytuners.last(), SIGNAL(adapter_status(int)), this, SLOT(adapter_status(int)));
		connect(mytuners.last(), SIGNAL(update_status(QString,int)), this, SLOT(update_status(QString,int)));
		mytuners.last()->adapter	= adaps.at(i);
		mytuners.last()->frontend	= ui->comboBox_frontend->currentData().toInt();
		mytuners.last()->tune_ops	= tune_ops[ui->comboBox_lnb->currentData().toInt()];
		mytuners.last()->getops();

		ss_filename.append(QString());
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

	for (int i = 0; i < mytuners.size(); i++) {
		adapter_status(mytuners.at(i)->adapter);
	}

	int gotox_i = ui->comboBox_gotox->currentIndex();
	ui->comboBox_gotox->clear();
	ui->comboBox_gotox->addItem("");
	for (int i = 1; i < 256; i++) {
		QString text = mysettings->value("adapter"+QString::number(ui->comboBox_adapter->currentData().toInt())+"_diseqc_v12_name_"+QString::number(i)).toString();
		if (text != "") {
			ui->comboBox_gotox->addItem(text, i);
		}
	}
	ui->comboBox_gotox->setCurrentIndex(gotox_i);

	QVariant d(0);
	QVariant e(1|32);
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

void MainWindow::markers_draw()
{
	for(int i = 0; i < marker.size(); i++) {
		marker.at(i)->detach();
	}
	marker.clear();

	for (int i = 0; i < mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.size(); i++) {
		marker.append(new QwtPlotMarker);
		QwtText text;
		if (isSatellite(mytuners.at(ui->comboBox_adapter->currentIndex())->tp.system)) {
			text = QString::number(mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.at(i).frequency);
		} else {
			text = mytuners.at(ui->comboBox_adapter->currentIndex())->format_freq(mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.at(i).frequency, mytuners.at(ui->comboBox_adapter->currentIndex())->tp_try.at(i).system);
		}
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
	ui->qwtPlot->updateLegend();
	ui->qwtPlot->replot();

	myscan->ready = true;
}

void MainWindow::set_colors()
{
	Qt::BrushStyle pattern_H = Qt::SolidPattern;
	Qt::BrushStyle pattern_V = Qt::SolidPattern;
	Qt::BrushStyle pattern_N = Qt::SolidPattern;
	double max_waterfall = ui->comboBox_waterfall_points->currentText().toInt();

	if (ui->checkBox_waterfall->isChecked()) {
		int pen_color;
		int brush_color;
		switch(ui->comboBox_voltage->currentIndex())
		{
		case 0:
			pattern_H = Qt::Dense4Pattern;
			pattern_V = Qt::SolidPattern;
			break;
		case 1:
			pattern_H = Qt::SolidPattern;
			pattern_V = Qt::Dense4Pattern;
			break;
		case 2:
			pattern_N = Qt::SolidPattern;
			break;
		}
		for (int c = 0; c < waterfall_curve_V.size(); c++) {
			pen_color = azero(100 * (1 - c / max_waterfall));
			brush_color = azero(255 * (1 - c / max_waterfall));
			waterfall_curve_V.at(c)->setPen(QPen(QColor(0, pen_color, 0), 2));
			waterfall_curve_V.at(c)->setBaseline(-32767);
			waterfall_curve_V.at(c)->setBrush(QBrush(QColor(0, brush_color, 0), pattern_V));
		}
		for (int c = 0; c < waterfall_curve_H.size(); c++) {
			pen_color = azero(100 * (1 - c / max_waterfall));
			brush_color = azero(255 * (1 - c / max_waterfall));
			waterfall_curve_H.at(c)->setPen(QPen(QColor(0, pen_color, 0), 2));
			waterfall_curve_H.at(c)->setBaseline(-32767);
			waterfall_curve_H.at(c)->setBrush(QBrush(QColor(0, brush_color, 0), pattern_H));
		}
		for (int c = 0; c < waterfall_curve_N.size(); c++) {
			pen_color = azero(100 * (1 - c / max_waterfall));
			brush_color = azero(255 * (1 - c / max_waterfall));
			waterfall_curve_N.at(c)->setPen(QPen(QColor(0, pen_color, 0), 2));
			waterfall_curve_N.at(c)->setBaseline(-32767);
			waterfall_curve_N.at(c)->setBrush(QBrush(QColor(0, brush_color, 0), pattern_N));
		}
	} else {
		if (mytuners.at(ui->comboBox_adapter->currentIndex())->tune_ops.voltage == 0) {
			curve[0]->setPen(QPen(GREEN));
			curve[0]->detach();
			curve[0]->attach(ui->qwtPlot);
		}
		if (mytuners.at(ui->comboBox_adapter->currentIndex())->tune_ops.voltage == 1) {
			curve[1]->setPen(QPen(GREEN));
			curve[1]->detach();
			curve[1]->attach(ui->qwtPlot);
		}
		if (mytuners.at(ui->comboBox_adapter->currentIndex())->tune_ops.voltage == 2) {
			curve[2]->setPen(QPen(GREEN));
			curve[2]->detach();
			curve[2]->attach(ui->qwtPlot);
		}
		if (mytuners.at(ui->comboBox_adapter->currentIndex())->tune_ops.voltage == 3) {
			if (ui->comboBox_voltage->currentIndex() == 0) {
				curve[0]->setPen(QPen(GREEN));
				curve[1]->setPen(QPen(DGREEN));
				curve[1]->detach();
				curve[1]->attach(ui->qwtPlot);
				curve[0]->detach();
				curve[0]->attach(ui->qwtPlot);
			} else {
				curve[0]->setPen(QPen(DGREEN));
				curve[1]->setPen(QPen(GREEN));
				curve[0]->detach();
				curve[0]->attach(ui->qwtPlot);
				curve[1]->detach();
				curve[1]->attach(ui->qwtPlot);
			}
		}
	}
	ui->qwtPlot->replot();
}

void MainWindow::on_checkBox_waterfall_clicked()
{
	ui->checkBox_loop->setChecked(ui->checkBox_waterfall->isChecked());
}

void MainWindow::on_actionSave_Screenshot_triggered()
{
	QString filename;

	if (mysettings->value("adapter" + ui->comboBox_adapter->currentData().toString() + "_save_images").toBool() && ui->checkBox_loop->isChecked()) {
		filename = QDir::currentPath() + "/rfscan_adapter" + ui->comboBox_adapter->currentData().toString();
		filename.append(QDateTime::currentDateTime().toString("_yyyyMMdd_hhmmss"));
		filename.append(".png");
	} else {
		filename = ss_filename.at(ui->comboBox_adapter->currentIndex());
		if (filename.isEmpty()) {
			filename = QDir::currentPath() + "/rfscan_adapter" + ui->comboBox_adapter->currentData().toString() + ".png";
		}
		filename = QFileDialog::getSaveFileName(this, "Save Screen Shot", filename, tr("Image Files (*.png)"));
		ss_filename[ui->comboBox_adapter->currentIndex()] = filename;
	}

	if (filename.isEmpty()) {
		return;
	}

	QPixmap p = ui->qwtPlot->grab();
	p.save(filename, "PNG");
}
