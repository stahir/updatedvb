#ifndef BLINDSCAN_H
#define BLINDSCAN_H

#include <QWidget>
#include <QtCore>
#include <QProcess>
#include <QProgressBar>
#include <QDebug>
#include <QPointer>
#include "dvbtune.h"
#include "tuning.h"
#include "dvb_settings.h"
#include "blindscan_save.h"
#include "blindscan_thread.h"

namespace Ui {
class blindscan;
}

class blindscan : public QWidget
{
	Q_OBJECT

public:
	explicit blindscan(QWidget *parent = 0);
	~blindscan();
	void init();
	void scan();
	void smartscan();

	dvbtune *mytune;
	tuning *mytuning;
	bool mytuning_running;

private slots:
	void update_signal();
	void on_pushButton_tune_clicked();
	void on_pushButton_save_clicked();
	void on_pushButton_expand_clicked();
	void on_pushButton_unexpand_clicked();
	void update_progress(int i);
	void mytuning_destroyed();

private:
	Ui::blindscan *ui;

	int tree_create_root(QString text);
	int tree_create_child(int parent, QString text);

	blindscan_thread mythread;
	int pindex;
	int cindex;
	QTreeWidgetItem *ptree[65535];
	QTreeWidgetItem *ctree[65535];
	dvb_settings dvbnames;
	QVector<tp_info> mytp_info;
	QPointer<QProgressBar> myprogress;

protected:
	void closeEvent(QCloseEvent *event);
	void keyPressEvent(QKeyEvent *event);
};

#endif // BLINDSCAN_H
