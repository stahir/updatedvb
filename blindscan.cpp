#include "blindscan.h"
#include "ui_blindscan.h"

blindscan::blindscan(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::blindscan)
{
	ui->setupUi(this);
	ui->pushButton_save->setEnabled(false);
	ui->pushButton_tune->setEnabled(false);

	pindex = -1;
	cindex = -1;
	myprogress = new QProgressBar;
	myprogress->setMinimum(0);
	myprogress->setMaximum(100);
	myprogress->setVisible(true);
	ui->verticalLayout->addWidget(myprogress);
	mytuning_running = false;
}

void blindscan::init()
{
	mythread.mytune = mytune;
	mythread.mytune->thread_function.clear();
	connect(mythread.mytune, SIGNAL(update_signal()), this, SLOT(update_signal()), Qt::UniqueConnection);
	connect(&mythread, SIGNAL(update_progress(int)), this, SLOT(update_progress(int)));
	this->setWindowTitle("Blindscan, Adapter " + QString::number(mytune->adapter) + ", Frontend " + QString::number(mytune->frontend) + " : " + mytune->name);
}

blindscan::~blindscan()
{
	mythread.ready	= true;
	mythread.loop	= false;
	mythread.quit();
	mythread.wait(1000);
	while (mythread.isRunning()) {
		mythread.ready	= true;
		mythread.loop	= false;
		QThread::msleep(100);
	}

	mytune->loop = false;
	mytune->quit();
	mytune->wait(1000);
	while (mytune->isRunning()) {
		mytune->loop = false;
		QThread::msleep(100);
	}

	if (mytuning_running) {
		mytuning->deleteLater();
	}

	delete myprogress;
	delete ui;
}

void blindscan::closeEvent(QCloseEvent *event)
{
	Q_UNUSED(event);
	this->deleteLater();
}

void blindscan::scan()
{
	mythread.loop = false;
	mythread.thread_function.append("blindscan");
	mythread.start();
}

void blindscan::smartscan()
{
	mythread.loop = false;
	mythread.thread_function.append("smartscan");
	mythread.start();
}

void blindscan::update_progress(int i)
{
	if (i > myprogress->maximum()) {
		i = myprogress->maximum();
	}
	myprogress->setValue(i);
	if (myprogress->value() >= myprogress->maximum()) {
		myprogress->setVisible(false);
		ui->pushButton_save->setEnabled(true);
		ui->pushButton_tune->setEnabled(true);
	}
}

int blindscan::tree_create_root(QString text)
{
	pindex++;
	ptree[pindex] = new QTreeWidgetItem(ui->treeWidget);
	ptree[pindex]->setText(0, text);

	return pindex;
}

int blindscan::tree_create_child(int parent, QString text)
{
	cindex++;
	ctree[cindex] = new QTreeWidgetItem();
	ctree[cindex]->setText(0, text);
	ptree[parent]->addChild(ctree[cindex]);

	return cindex;
}

void blindscan::update_signal()
{
	if (mythread.ready) {
		return;
	}
	if (!(mythread.mytune->tp.status & FE_HAS_LOCK)) {
		mythread.ready = true;
		return;
	}
	if (!(mythread.mytune->tp.status & (0xFF ^ FE_TIMEDOUT))) {
		mythread.ready = true;
		return;
	}

	int parent_1;

	mytp_info.append(mythread.mytune->tp);

	freq_list myfreq;
	QString text;
	if (isSatellite(mytune->tp.system)) {
		text = QString::number(mytune->tp.frequency) + dvbnames.voltage[mytune->tp.voltage] + QString::number(mytune->tp.symbolrate);
	} else if (isATSC(mytune->tp.system)) {
		myfreq.atsc();
		text = QString::number(mytune->tp.frequency/1000) + "mhz, Channel " + QString::number(myfreq.ch.at(myfreq.freq.indexOf(mytune->tp.frequency)));
	} else if (isQAM(mytune->tp.system)) {
		myfreq.qam();
		text = QString::number(mytune->tp.frequency/1000) + "mhz, Channel " + QString::number(myfreq.ch.at(myfreq.freq.indexOf(mytune->tp.frequency)));
	} else if (isDVBT(mytune->tp.system)) {
		myfreq.dvbt();
		text = QString::number(mytune->tp.frequency/1000) + "mhz, Channel " + QString::number(myfreq.ch.at(myfreq.freq.indexOf(mytune->tp.frequency)));
	} else {
		text = QString::number(mytune->tp.frequency/1000) + "mhz";
	}

	parent_1 = tree_create_root(text);
	if (mytune->tp.status & FE_HAS_LOCK) {
		ptree[parent_1]->setForeground(0, QBrush(Qt::green));
	} else {
		ptree[parent_1]->setForeground(0, QBrush(Qt::red));
	}
	text = "System: " + dvbnames.system[mytune->tp.system];
	tree_create_child(parent_1, text);
	text = "Modulation: " + dvbnames.modulation[mytune->tp.modulation];
	tree_create_child(parent_1, text);

	if (isSatellite(mytune->tp.system)) {
		text = "FEC: " + dvbnames.fec[mytune->tp.fec];
		tree_create_child(parent_1, text);
		text = "Inversion: " + dvbnames.inversion[mytune->tp.inversion];
		tree_create_child(parent_1, text);
		text = "Rolloff: " + dvbnames.rolloff[mytune->tp.rolloff];
		tree_create_child(parent_1, text);
		text = "Pilot: " + dvbnames.pilot[mytune->tp.pilot];
		tree_create_child(parent_1, text);
	}

	if (mytune->tp.lvl_scale == FE_SCALE_DECIBEL) {
		text = "Signal S: " + QString::number(mytune->tp.lvl, 'f', 1) + "dBm";
	} else {
		text = "Signal S: " + QString::number(mytune->tp.lvl) + "%";
	}
	tree_create_child(parent_1, text);
	if (mytune->tp.snr_scale == FE_SCALE_DECIBEL) {
		text = "Signal Q: " + QString::number(mytune->tp.snr, 'f', 1) + "dB";
		tree_create_child(parent_1, text);
		ctree[cindex]->setToolTip(0, "min snr: " + mytune->min_snr() + "dB");
	} else {
		text = "Signal Q: " + QString::number(mytune->tp.snr) + "%";
		tree_create_child(parent_1, text);
	}

	mythread.ready = true;
}

void blindscan::on_pushButton_tune_clicked()
{
	if (mythread.loop == true) {
		qDebug() << "Not done scanning yet";
		return;
	}
	if (mytuning_running) {
		qDebug() << "Already tuned";
		return;
	}
	int i = -1;
	for(int a = 0; a <= pindex; a++) {
		if (ptree[a]->isSelected()) {
			i = a;
		}
	}
	if (i == -1) {
		qDebug() << "No frequency selected";
		return;
	}
	mytuning				= new tuning;
	connect(mytuning, SIGNAL(destroyed()), this, SLOT(mytuning_destroyed()));
	mytuning_running		= true;
	mytuning->mytune		= mytune;
	mytuning->mytune->tp	= mytp_info.at(i);
	mytuning->init();
	mytuning->show();
}

void blindscan::mytuning_destroyed()
{
	mytuning_running = false;
}

void blindscan::on_pushButton_save_clicked()
{
	blindscan_save mysave;
	mysave.mytp_info	= mytp_info;

	mysave.setModal(true);
	mysave.exec();
}

void blindscan::on_pushButton_expand_clicked()
{
	for(int a = 0; a <= pindex; a++) {
		ptree[a]->setExpanded(true);
	}
}

void blindscan::on_pushButton_unexpand_clicked()
{
	for(int a = 0; a <= pindex; a++) {
		ptree[a]->setExpanded(false);
	}
}
void blindscan::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape) {
		this->close();
	}
}

