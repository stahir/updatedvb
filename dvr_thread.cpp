#include "dvr_thread.h"

dvr_thread::dvr_thread()
{
	loop	= false;
	file_fd	= 0;
}

dvr_thread::~dvr_thread()
{
	loop = false;
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
	close_file();
}

void dvr_thread::close_file()
{
	mytune->close_dvr();
	if (file_fd) {
		close(file_fd);
		file_fd = 0;
		file_name.clear();
	}
}

void dvr_thread::demux_file()
{
	if (mytune->dvr_name.isEmpty()) {
		mytune->dvr_name = "/dev/dvb/adapter" + QString::number(mytune->adapter) + "/dvr0";
		mytune->dvr_fd = open(mytune->dvr_name.toStdString().c_str(), O_RDONLY);
		if (mytune->dvr_fd < 0) {
			qDebug() << "Failed to open" << mytune->dvr_name;
			return;
		}
	}
	if (!file_fd) {
		file_fd = open(file_name.toStdString().c_str(), O_CREAT|O_TRUNC|O_RDWR|O_NONBLOCK, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (file_fd < 0) {
			qDebug() << "Failed to open" << file_name;
			return;
		}
	}

	fd_set set;
	FD_ZERO(&set);
	FD_SET(mytune->dvr_fd, &set);

	int len = 0;
	char buf[LIL_BUFSIZE];
	memset(buf, 0, LIL_BUFSIZE);

	mytune->status = setbit(mytune->status, TUNER_RDING);
	if (select(mytune->dvr_fd + 1, &set, NULL, NULL, &mytune->fd_timeout) > 0) {
		len = read(mytune->dvr_fd, buf, LIL_BUFSIZE);
	} else {
		qDebug() << "read(dvr_fd) timeout";
	}
	ssize_t wlen = write(file_fd, buf, len);
	mytune->status = unsetbit(mytune->status, TUNER_RDING);

	emit data_size(len);
	Q_UNUSED(wlen);
}

void dvr_thread::demux_stream()
{
	if (mytune->dvr_name.isEmpty()) {
		mytune->dvr_name = "/dev/dvb/adapter" + QString::number(mytune->adapter) + "/dvr0";
		mytune->dvr_fd = open(mytune->dvr_name.toStdString().c_str(), O_RDONLY);
		if (mytune->dvr_fd < 0) {
			qDebug() << "Failed to open" << mytune->dvr_name;
			return;
		}
	}

	fd_set set;
	FD_ZERO(&set);
	FD_SET(mytune->dvr_fd, &set);

	int len = -1;
	char buf[LIL_BUFSIZE];
	memset(buf, 0, LIL_BUFSIZE);

	mytune->status = setbit(mytune->status, TUNER_RDING);
	if (select(mytune->dvr_fd + 1, &set, NULL, NULL, &mytune->fd_timeout) > 0) {
		len = read(mytune->dvr_fd, buf, LIL_BUFSIZE);
	} else {
		qDebug() << "read(dvr_fd) timeout";
	}
	mytune->status = unsetbit(mytune->status, TUNER_RDING);

	if (len == -1 || len != LIL_BUFSIZE) {
		qDebug() << Q_FUNC_INFO << "read issue:" << len << "of" << LIL_BUFSIZE;
		return;
	}

	emit data(QByteArray(buf, len));
}
