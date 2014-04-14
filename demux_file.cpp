#include "demux_file.h"
#include "ui_demux_file.h"

demux_file::demux_file(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::demux_file)
{
	ui->setupUi(this);

	mystatus = new QStatusBar;
	ui->verticalLayout->addWidget(mystatus);
	mystatus->setVisible(true);
	ui->lineEdit_filename->setText(QDir::currentPath() + "/test.ts");
}

demux_file::~demux_file()
{
	qDebug() << "~demux_file()";
	on_pushButton_stop_clicked();

	delete mystatus;
	delete ui;
}

void demux_file::init()
{
	connect(mytune, SIGNAL(demux_status(int)), this, SLOT(demux_status(int)));
}

void demux_file::on_pushButton_start_clicked()
{
	bytes_wrote = 0;
	mytune->close_dvr();
	mytune->out_name = ui->lineEdit_filename->text();
	mytune->loop = true;
	mytune->thread_function.append("demux_file");
	mytune->start();
}

void demux_file::on_pushButton_stop_clicked()
{
	if (mytune->thread_function.indexOf("demux_file") != -1) {
		mytune->thread_function.remove(mytune->thread_function.indexOf("demux_file"));
	}
	mytune->close_dvr();
}

void demux_file::demux_status(int bytes)
{
	bytes_wrote += bytes;
	mystatus->showMessage(QString("%L1 KB").arg(bytes_wrote/1000), 0);
}

void demux_file::closeEvent(QCloseEvent *event)
{
	Q_UNUSED(event);
	this->deleteLater();
}
