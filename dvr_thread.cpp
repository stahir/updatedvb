#include "dvr_thread.h"

dvr_thread::dvr_thread()
{
	loop	= false;
	is_busy	= false;
}

dvr_thread::~dvr_thread()
{
	qDebug() << Q_FUNC_INFO;
	while (is_busy) {
		loop = false;
		msleep(100);
	}
}

void dvr_thread::run()
{
	loop = true;
	while (loop) {
		if (thread_function.indexOf("demux_file") != -1) {
			demux_file();
		}
		if (thread_function.indexOf("demux_stream") != -1) {
			demux_stream();
		}
		if (thread_function.size() == 0) {
			msleep(100);
		}
	}
}

void dvr_thread::demux_file()
{
	if (dvr_name == "") {
		dvr_name = "/dev/dvb/adapter" + QString::number(adapter) + "/dvr0";
		qDebug() << "demux_file() opening" << dvr_name;
		dvr_fd = open(dvr_name.toStdString().c_str(), O_RDONLY);
		if (dvr_fd < 0) {
			qDebug() << "Failed to open" << dvr_name;
			return;
		}
	}
	if (!file_fd) {
		qDebug() << "open file_fd";
		file_fd = open(file_name.toStdString().c_str(), O_CREAT|O_TRUNC|O_RDWR|O_NONBLOCK, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (file_fd < 0) {
			qDebug() << "Failed to open" << file_name;
			return;
		}
	}

	char buf[LIL_BUFSIZE];
	memset(buf, 0, LIL_BUFSIZE);

	is_busy = true;
	int len = read(dvr_fd, buf, LIL_BUFSIZE);
	ssize_t wlen = write(file_fd, buf, len);
	is_busy = false;

	emit data_size(len);
	Q_UNUSED(wlen);
}

void dvr_thread::demux_stream()
{
	if (dvr_name.isEmpty()) {
		dvr_name = "/dev/dvb/adapter" + QString::number(adapter) + "/dvr0";
		qDebug() << "demux_stream() opening" << dvr_name;
		dvr_fd = open(dvr_name.toStdString().c_str(), O_RDONLY);
		if (dvr_fd < 0) {
			qDebug() << "Failed to open" << dvr_name;
			return;
		}
	}

	char buf[LIL_BUFSIZE];
	memset(buf, 0, LIL_BUFSIZE);

	is_busy = true;
	int len = read(dvr_fd, buf, LIL_BUFSIZE);
	is_busy = false;

	if (len == -1 || len != LIL_BUFSIZE) {
		qDebug() << Q_FUNC_INFO << "read issue:" << len << "of" << LIL_BUFSIZE;
		return;
	}

	emit data(QByteArray(buf, len));
}
