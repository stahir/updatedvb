#ifndef IQPLOT_H
#define IQPLOT_H

#include <QDialog>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_plot_scaleitem.h>
#include <qwt_scale_engine.h>
#include "dvbtune.h"

const unsigned int MAX_GRADIANT = 6;

namespace Ui {
class IQplot;
}

class IQplot : public QDialog
{
	Q_OBJECT
	
public:
	explicit IQplot(QWidget *parent = 0);
	~IQplot();

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
	QwtPlotCurve *curve[MAX_GRADIANT];
	QwtPlotScaleItem *scaleX;
	QwtPlotScaleItem *scaleY;
	QwtSymbol *scatter_symbol[MAX_GRADIANT];

	Ui::IQplot *ui;
};

#endif // IQPLOT_H
