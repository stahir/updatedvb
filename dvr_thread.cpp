#include "dvr_thread.h"

dvr_thread::dvr_thread()
{
	loop	= false;
	file_fd	= -1;
}

dvr_thread::~dvr_thread()
{
	loop = false;
}

void dvr_thread::run()
{
	loop = true;
	while (loop) {
		if (thread_function.contains("demux_file")) {
			demux_file();
		}
		if (thread_function.contains("demux_stream")) {
			demux_stream();
		}
		if (thread_function.isEmpty()) {
			msleep(100);
		}
	}
	loop = false;
	close_file();
}

void dvr_thread::close_file()
{
	if (file_fd > 0) {
		close(file_fd);
		file_fd = -1;
		file_name.clear();
	}
}

void dvr_thread::demux_file()
{
	if (!mytune->open_dvr()) {
		return;
	}
	if (file_fd < 0) {
		file_fd = open(file_name.toStdString().c_str(), O_CREAT|O_TRUNC|O_RDWR|O_NONBLOCK, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (file_fd < 0) {
			qDebug() << Q_FUNC_INFO << "Failed to open" << file_name;
			return;
		}
	}

	fd_set set;
	FD_ZERO(&set);
	FD_SET(mytune->dvr_fd, &set);

	int len = -1;
	char buf[LIL_BUFSIZE];
	memset(buf, 0, LIL_BUFSIZE);

	mytune->setbit(TUNER_RDING);
	if (select(mytune->dvr_fd + 1, &set, NULL, NULL, &mytune->fd_timeout) > 0) {
		len = read(mytune->dvr_fd, buf, LIL_BUFSIZE);
	}
	mytune->unsetbit(TUNER_RDING);
	if (len > 0) {
		ssize_t wlen = write(file_fd, buf, len);
		Q_UNUSED(wlen);
	}

	emit data_size(len);
}

void dvr_thread::demux_stream()
{
	if (!mytune->open_dvr()) {
		return;
	}

	fd_set set;
	FD_ZERO(&set);
	FD_SET(mytune->dvr_fd, &set);

	int len = -1;
	char buf[LIL_BUFSIZE];
	memset(buf, 0, LIL_BUFSIZE);

	mytune->setbit(TUNER_RDING);
	if (select(mytune->dvr_fd + 1, &set, NULL, NULL, &mytune->fd_timeout) > 0) {
		len = read(mytune->dvr_fd, buf, LIL_BUFSIZE);
	}
	mytune->unsetbit(TUNER_RDING);
	if (len > 0) {
		emit data(QByteArray(buf, len));
	}

	if (len == -1 || len != LIL_BUFSIZE) {
		qDebug() << Q_FUNC_INFO << "read issue:" << len << "of" << LIL_BUFSIZE;
		msleep(100);
	}
}
