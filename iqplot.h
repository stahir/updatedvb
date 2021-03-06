#ifndef IQPLOT_H
#define IQPLOT_H

#include <QWidget>
#include <QKeyEvent>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_plot_scaleitem.h>
#include <qwt_scale_engine.h>
#include <qwt_plot_marker.h>
#include "dvbtune.h"

const unsigned int MAX_GRADIANT = 6;

namespace Ui {
class iqplot;
}

class iqplot : public QWidget
{
	Q_OBJECT

public:
	explicit iqplot(QWidget *parent = 0);
	~iqplot();

	void init();
	void erase();

	dvbtune *mytune;

private slots:
	void delete_iqplot();
	void iqdraw(QVector<short int> x, QVector<short int> y);
	void on_pushButton_onoff_clicked();
	void on_comboBox_mode_currentIndexChanged(int index);
	void on_comboBox_point_currentIndexChanged(int index);

private:
	Ui::iqplot *ui;
	QwtPlotCurve *curve[MAX_GRADIANT];
	QwtPlotScaleItem *scaleX;
	QwtPlotScaleItem *scaleY;
	QwtLinearScaleEngine *scale_eng;
	QwtSymbol *scatter_symbol[MAX_GRADIANT];
	QVector<QString> modcod_name;
	QVector<QwtPlotMarker *> modcod_marker;

protected:
	void closeEvent(QCloseEvent *event);
	void keyPressEvent(QKeyEvent *event);
};

#endif // IQPLOT_H
