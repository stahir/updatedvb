#include "iqplot.h"
#include "ui_iqplot.h"

iqplot::iqplot(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::iqplot)
{
	ui->setupUi(this);

	int gr = 255/MAX_GRADIANT;
	for (unsigned int i = 0; i < MAX_GRADIANT; i++) {
		scatter_symbol[i] = new QwtSymbol;
		scatter_symbol[i]->setStyle(QwtSymbol::Rect);
		scatter_symbol[i]->setSize(2,2);
		scatter_symbol[i]->setPen(QColor(0, gr*i + gr, 0));
		scatter_symbol[i]->setBrush(QColor(0, gr*i + gr, 0));
		curve[i] = new QwtPlotCurve("Curve");
		curve[i]->setStyle(QwtPlotCurve::NoCurve);
		curve[i]->attach(ui->qwtPlot);
		curve[i]->setSymbol(scatter_symbol[i]);
	}

	ui->qwtPlot->setAxisScale(QwtPlot::xBottom, -128, 128);
	ui->qwtPlot->setAxisScale(QwtPlot::yLeft, -128, 128);
	ui->qwtPlot->enableAxis(QwtPlot::xBottom ,0);
	ui->qwtPlot->enableAxis(QwtPlot::yLeft ,0);
	ui->qwtPlot->setCanvasBackground(Qt::black);

	scaleX = new QwtPlotScaleItem();
	scaleX->setAlignment(QwtScaleDraw::BottomScale);
	scale_eng = new QwtLinearScaleEngine();
	scaleX->setScaleDiv(scale_eng->divideScale(-128, 128, 10, 5));
	scaleX->attach(ui->qwtPlot);
	scaleY = new QwtPlotScaleItem();
	scaleY->setAlignment(QwtScaleDraw::LeftScale);
	scaleY->setScaleDiv(scale_eng->divideScale(-128, 128, 10, 5));
	scaleY->attach(ui->qwtPlot);
}

iqplot::~iqplot()
{
	qDebug() << "~iqplot()";
	mytune->loop = false;
	mytune->quit();
	mytune->wait(1000);
	while (mytune->isRunning()) {
		qDebug() << "mytune->isRunning()";
		mytune->loop = false;
		sleep(1);
	}

	ui->qwtPlot->detachItems();

	delete scale_eng;
	delete ui->qwtPlot;
	delete ui;
}

void iqplot::closeEvent(QCloseEvent *event)
{
	Q_UNUSED(event);
	this->deleteLater();
}

void iqplot::init()
{
	qDebug() << "init()";
	this->setWindowTitle("Tuning Adapter " + QString::number(mytune->adapter) + ", Frontend " + QString::number(mytune->frontend) + " : " + mytune->name);

	connect(mytune, SIGNAL(iqdraw(QVector<short int>, QVector<short int>)), this, SLOT(iqdraw(QVector<short int>, QVector<short int>)));

	mytune->start();
	on_pushButton_onoff_clicked();
}

void iqplot::delete_iqplot()
{
	qDebug() << "delete_iqplot()";
	this->deleteLater();
}

void iqplot::iqdraw(QVector<short int> x, QVector<short int> y)
{
	QVector<double> xs[MAX_GRADIANT];
	QVector<double> ys[MAX_GRADIANT];
	bool xys[MAX_GRADIANT][0xFFFF];

	int scale = 0;

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
				if (abs(x[i]) > scale) {
					scale = abs(x[i]);
				}
				if (abs(y[i]) > scale) {
					scale = abs(y[i]);
				}
				goto next;
			}
		}
		next:
		continue;
	}
	for (unsigned short int a = 0; a < MAX_GRADIANT; a++) {
		curve[a]->setSamples(xs[a], ys[a]);
	}
	scaleX->setScaleDiv(scale_eng->divideScale(scale * -1, scale, 10, 5));
	scaleY->setScaleDiv(scale_eng->divideScale(scale * -1, scale, 10, 5));
	ui->qwtPlot->setAxisScale(QwtPlot::xBottom, scale * -1, scale);
	ui->qwtPlot->setAxisScale(QwtPlot::yLeft, scale * -1, scale);
	ui->qwtPlot->replot();
}

void iqplot::on_pushButton_onoff_clicked()
{
	if (mytune->thread_function.indexOf("iqplot") != -1) {
		mytune->thread_function.remove(mytune->thread_function.indexOf("iqplot"));
		ui->pushButton_onoff->setText("Start");
	} else {
		erase();
		mytune->thread_function.append("iqplot");
		ui->pushButton_onoff->setText("Stop");
	}
}

void iqplot::erase()
{
	mytune->iq_options = ui->comboBox_mode->currentIndex() << 4 | ui->comboBox_point->currentIndex();
	mytune->iq_x.clear();
	mytune->iq_y.clear();
}

void iqplot::on_comboBox_mode_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	erase();
}

void iqplot::on_comboBox_point_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	erase();
}
