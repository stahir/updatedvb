#ifndef DVR_THREAD_H
#define DVR_THREAD_H

#include <QDebug>
#include <QThread>
#include <QVector>
#include <QByteArray>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "dvb_settings.h"

class dvr_thread : public QThread
{
	Q_OBJECT
public:
	dvr_thread();
	~dvr_thread();
	void demux_file();
	void demux_stream();
	void run();
	QVector<QString> thread_function;
	bool loop;
	bool is_busy;
	QString dvr_name, file_name;
	int dvr_fd, file_fd;
	int adapter;

signals:
	void data(QByteArray);
	void data_size(int);

public slots:

};

#endif // DVR_THREAD_H
