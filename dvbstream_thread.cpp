/*	
 *	updateDVB, a DVB/ATSC spectrum analyzer, tuning and stream analyzing application.
 *	Copyright (C) 2013  Chris Lee (updatelee@gmail.com)
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *	
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *	
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dvbstream_thread.h"

dvbstream_thread::dvbstream_thread()
{
	signal(SIGPIPE, SIG_IGN);	
}

dvbstream_thread::~dvbstream_thread()
{
	qDebug() << "~dvbstream_thread()";
	qDebug() << "~dvbstream_thread() done";
}

void dvbstream_thread::socket_new()
{
	qDebug() << "socket_new()";
	socket = server->nextPendingConnection();
	socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
	connect(socket, SIGNAL(disconnected()), this, SLOT(socket_close()), Qt::DirectConnection);
	
	msleep(100);

	QTextStream resultStream(socket);
	resultStream << "HTTP/1.0 200 Ok\r\n";
	resultStream << "Content-Type: video/MP2T\r\n";
	resultStream << "\r\n";
	resultStream.flush();

	loop = true;
	while (loop && socket->state() == QAbstractSocket::ConnectedState) {
		socket->write(mytune->demux_stream());
		// Fix this, this causes much better performance, but randomly crash's with sigpipe, catch the error and ignore it
		socket->flush();
		socket->waitForBytesWritten(1000);
	}

	qDebug() << "All Done.";
	socket->close();
	server->close();
	mytune->close_dvr();
	
	delete socket;
	delete server;
}

void dvbstream_thread::socket_close()
{
	qDebug() << "socket_close()";
	loop = false;
}

void dvbstream_thread::run()
{
	int port = 1230 + mytune->adapter;
	
	server = new QTcpServer();
	if (server->listen(QHostAddress::Any, port)) {
		qDebug() << "Server started on port:" << port;
	} else {
		qDebug() << "Server could not start";
		return;
	}

	connect(server, SIGNAL(newConnection()), this, SLOT(socket_new()), Qt::DirectConnection);
	
	exec();
}