#ifndef DEMUX_FILE_H
#define DEMUX_FILE_H

#include <QWidget>
#include <QDir>
#include <QStatusBar>
#include <QFileDialog>
#include "dvbtune.h"

namespace Ui {
class demux_file;
}

class demux_file : public QWidget
{
	Q_OBJECT

public:
	explicit demux_file(QWidget *parent = 0);
	~demux_file();

	void init();
	dvbtune *mytune;

public slots:
	void on_pushButton_stop_clicked();

private slots:
	void on_pushButton_start_clicked();
	void demux_status(int bytes);

	void on_pushButton_browse_clicked();

private:
	Ui::demux_file *ui;
	QStatusBar *mystatus;
	unsigned long int bytes_wrote;

protected:
	void closeEvent(QCloseEvent *event);
};

#endif // DEMUX_FILE_H
