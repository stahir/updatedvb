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
		scatter_symbol[i]->setSize(1,1);
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
	ui->qwtPlot->setCanvasBackground(Qt::darkGray);

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
	if (mytune->thread_function.contains("iqplot")) {
		mytune->thread_function.remove(mytune->thread_function.indexOf("iqplot"));
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

void iqplot::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape) {
		this->close();
	}
}

void iqplot::init()
{
	this->setWindowTitle("IQ Adapter " + QString::number(mytune->adapter) + ", Frontend " + QString::number(mytune->frontend) + " : " + mytune->name);

	connect(mytune, SIGNAL(iqdraw(QVector<short int>, QVector<short int>)), this, SLOT(iqdraw(QVector<short int>, QVector<short int>)));

	modcod_name.append("QPSK 1/4");
	modcod_name.append("QPSK 1/3");
	modcod_name.append("QPSK 2/5");
	modcod_name.append("QPSK 1/2");
	modcod_name.append("QPSK 3/5");
	modcod_name.append("QPSK 2/3");
	modcod_name.append("QPSK 3/4");
	modcod_name.append("QPSK 4/5");
	modcod_name.append("QPSK 5/6");
	modcod_name.append("QPSK 8/9");
	modcod_name.append("QPSK 9/10");
	modcod_name.append("8PSK 3/5");
	modcod_name.append("8PSK 2/3");
	modcod_name.append("8PSK 3/4");
	modcod_name.append("8PSK 5/6");
	modcod_name.append("8PSK 8/9");
	modcod_name.append("8PSK 9/10");
	modcod_name.append("16PSK 2/3");
	modcod_name.append("16PSK 3/4");
	modcod_name.append("16PSK 4/5");
	modcod_name.append("16PSK 5/6");
	modcod_name.append("16PSK 8/9");
	modcod_name.append("16PSK 9/10");
	modcod_name.append("32PSK 3/4");
	modcod_name.append("32PSK 4/5");
	modcod_name.append("32PSK 5/6");
	modcod_name.append("32PSK 8/9");
	modcod_name.append("32PSK 9/10");

	int x = -119;
	for(int i = 0; i < modcod_name.size(); i++) {
		modcod_marker.append(new QwtPlotMarker);
		modcod_marker.last()->setLabel(modcod_name.at(i));
		modcod_marker.last()->setLabelAlignment(Qt::AlignCenter|Qt::AlignBottom);
		modcod_marker.last()->setLabelOrientation(Qt::Vertical);
		modcod_marker.last()->setLineStyle(QwtPlotMarker::VLine);
		modcod_marker.last()->setLinePen(Qt::blue,0,Qt::DotLine);
		modcod_marker.last()->setValue(x,0);
		x += 8;
	}
	mytune->start();
	on_pushButton_onoff_clicked();
}

void iqplot::delete_iqplot()
{
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
	if (mytune->thread_function.contains("iqplot")) {
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

	sleep(1);

	mytune->iq_x.clear();
	mytune->iq_y.clear();

	if (ui->comboBox_point->currentIndex() == 14)
	{
		ui->qwtPlot->setMinimumWidth(680);
		scaleY->detach();
		for (int i = 0; i < modcod_marker.size(); i++) {
			modcod_marker.at(i)->attach(ui->qwtPlot);
		}
	} else {
		ui->qwtPlot->setMinimumWidth(384);
		ui->iqplot_widget->resize(ui->iqplot_widget->minimumWidth(), ui->iqplot_widget->height());
		scaleY->attach(ui->qwtPlot);
		for (int i = 0; i < modcod_marker.size(); i++) {
			modcod_marker.at(i)->detach();
		}
	}
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
