#ifndef DVR_THREAD_H
#define DVR_THREAD_H

#include <QDebug>
#include <QThread>
#include <QByteArray>
#include <unistd.h>
#include <fcntl.h>
#include "dvb_settings.h"
#include "dvbtune.h"

class dvbtune;

class dvr_thread : public QThread
{
	Q_OBJECT
public:
	dvr_thread();
	~dvr_thread();
	void demux_file();
	void demux_stream();
	void close_file();
	void run();
	QVector<QString> thread_function;
	bool loop;
	QString file_name;
	int file_fd;
	dvbtune *mytune;

signals:
	void data(QByteArray);
	void data_size(int);

public slots:
};

#endif // DVR_THREAD_H
