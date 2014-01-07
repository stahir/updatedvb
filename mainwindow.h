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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QSettings>
#include <QString>
#include <QDir>
#include <QTimer>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>
#include <qwt_picker_machine.h>
#include <qwt_legend.h>
#include "dvb_settings.h"
#include "scan.h"
#include "settings.h"
#include "blindscan.h"

namespace Ui {
    class MainWindow;
}

class PlotPicker : public QwtPlotPicker
{
public:
	PlotPicker(int xAxis, int yAxis, RubberBand rubberBand, DisplayMode trackerMode, QwtPlotCanvas* canvas) : QwtPlotPicker(xAxis, yAxis, rubberBand, trackerMode, canvas)
	{
	}

private:
	QwtText trackerText( const QPoint &pos ) const
	{
		return trackerTextF( invTransform( pos ) );
	}

	QwtText trackerTextF(const QPointF &pos) const
	{
		QwtText text(QString("%L1").arg(pos.x(), 0, 'f', 0) + " - " + QString("%L1").arg(pos.y(), 0, 'f', 0));
		return text;
	}
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
	void qwtPlot_selected(QPointF pos);
	void qwt_draw(QVector<double> x, QVector<double> y, int min, int max, int cindex);
	void update_status(QString text, int time = -1);

private slots:
	void on_updateButton_clicked();
	void on_checkBox_loop_stateChanged();
	void on_actionSettings_triggered();
	void on_comboBox_lnb_currentIndexChanged(int index);
	void on_comboBox_adapter_currentIndexChanged(int index);
	void on_comboBox_frontend_currentIndexChanged(int index);
	void on_comboBox_voltage_currentIndexChanged(int index);
	void on_pushButton_scan_clicked();
	void on_pushButton_usals_go_clicked();
	void on_lineEdit_usals_returnPressed();
	void on_pushButton_drive_east_L_clicked();
	void on_pushButton_drive_east_S_clicked();
	void on_pushButton_drive_west_S_clicked();
	void on_pushButton_drive_west_L_clicked();
	void on_pushButton_gotox_go_clicked();
	void on_pushButton_gotox_save_clicked();
	void on_lineEdit_gotox_returnPressed();
	void on_actionExit_triggered();
	void adapter_status(int adapter, bool is_busy);
	void setup_tuning_options();
	
private:
	Ui::MainWindow *ui;
	QVector<tuning*> mytuning;
    scan *myscan;
	PlotPicker *qwt_picker;
	QVector<QwtPlotCurve*> curve;
	QVector<QwtPlotMarker*> marker;
	QwtLegend *legend;
	QSettings *mysettings;
	QVector<tuning_options> tune_ops;
	dvb_settings dvbnames;
	QVector<dvbtune*> mytuners;
	QVector<blindscan*> myblindscan;
	bool noload;
	QVector<QString> mystatus;
	QSignalMapper status_mapper;
	QTimer *status_timer;

	void getadapters();
	void reload_settings();
	void add_comboBox_modulation(QString name);

protected:
	void closeEvent(QCloseEvent *event);	
};

#endif // MAINWINDOW_H
