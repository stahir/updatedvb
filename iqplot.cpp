#include "iqplot.h"
#include "ui_iqplot.h"

IQplot::IQplot(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::IQplot)
{
	ui->setupUi(this);

	connect(this, SIGNAL(finished(int)), this, SLOT(delete_iqplot()));
	
	int bc = 180;
	int gr = bc/MAX_GRADIANT;
	for (unsigned int i = 0; i < MAX_GRADIANT; i++) {
		scatter_symbol[i] = new QwtSymbol;
		scatter_symbol[i]->setStyle(QwtSymbol::Rect);
		scatter_symbol[i]->setSize(1,1);
		scatter_symbol[i]->setPen(QColor(bc-(gr*i), bc-(gr*i), bc-(gr*i)));
	    scatter_symbol[i]->setBrush(QColor(bc-(gr*i), bc-(gr*i), bc-(gr*i)));
		curve[i] = new QwtPlotCurve("Curve");
		curve[i]->setStyle(QwtPlotCurve::NoCurve);
		curve[i]->attach(ui->qwtPlot);
	    curve[i]->setSymbol(scatter_symbol[i]);
	}

	ui->qwtPlot->setAxisScale(QwtPlot::xBottom, -75, 75);
	ui->qwtPlot->setAxisScale(QwtPlot::yLeft, -75, 75);
	ui->qwtPlot->enableAxis(QwtPlot::xBottom ,0);
	ui->qwtPlot->enableAxis(QwtPlot::yLeft ,0);

	scaleX = new QwtPlotScaleItem();
	scaleX->setAlignment(QwtScaleDraw::BottomScale);
	scaleX->setScaleDiv((new QwtLinearScaleEngine())->divideScale(-75, 75, 10, 5));
	scaleX->attach(ui->qwtPlot);
	scaleY = new QwtPlotScaleItem();
	scaleY->setAlignment(QwtScaleDraw::LeftScale);
	scaleY->setScaleDiv((new QwtLinearScaleEngine())->divideScale(-75, 75, 10, 5));
	scaleY->attach(ui->qwtPlot);
}

IQplot::~IQplot()
{
	mytune->loop = false;
	mytune->quit();
	mytune->wait(1000);
	while (mytune->isRunning()) {
		qDebug() << "mytune->isRunning()";
		mytune->loop = false;
		sleep(1);
	}
	
	ui->qwtPlot->detachItems();
	delete ui->qwtPlot;
	delete ui;
}

void IQplot::init()
{
	qDebug() << "init()";
	this->setWindowTitle("Tuning Adapter " + QString::number(mytune->adapter) + ", Frontend " + QString::number(mytune->frontend) + " : " + mytune->name);	

	connect(mytune, SIGNAL(iqdraw(QVector<short int>, QVector<short int>)), this, SLOT(iqdraw(QVector<short int>, QVector<short int>)));

	mytune->start();
	on_pushButton_onoff_clicked();
}

void IQplot::delete_iqplot()
{
	qDebug() << "delete_iqplot()";
	this->deleteLater();
}

void IQplot::iqdraw(QVector<short int> x, QVector<short int> y)
{
	qDebug() << "iqdraw()";
	
	QVector<double> xs[MAX_GRADIANT];
	QVector<double> ys[MAX_GRADIANT];
	bool xys[MAX_GRADIANT][0xFFFF];

	for (unsigned short int a = 0; a < MAX_GRADIANT; a++) {
		memset(xys[a], false, 0xFFFF);
	}
	for (unsigned short int i = 0; i < x.size(); i++) {
		// x[i] 0xF0 + y[i] 0x0F makes a unique hash, >>1 is simply /2 to make the hash match anything in a 2x2 pixel block vs a 1 for 1 pixel block
		unsigned short int xy_tmp = ((unsigned char)x[i]<<8) + (unsigned char)y[i];
		for (unsigned short int a = 0; a < MAX_GRADIANT; a++) {			
			if (!xys[a][xy_tmp]) {
				xys[a][xy_tmp] = true;
				xs[a].append(x[i]);
				ys[a].append(y[i]);
				goto next;
			}
		}
		next:
		continue;
	}
	for (unsigned short int a = 0; a < MAX_GRADIANT; a++) {
		curve[a]->setSamples(xs[a], ys[a]);
	}
	ui->qwtPlot->replot();
}

void IQplot::on_pushButton_onoff_clicked()
{
	if (mytune->thread_function.indexOf("iqplot") != -1) {
		mytune->thread_function.remove(mytune->thread_function.indexOf("iqplot"));
		ui->pushButton_onoff->setText("Start");
	} else {
		mytune->thread_function.append("iqplot");
		ui->pushButton_onoff->setText("Stop");
	}
}
