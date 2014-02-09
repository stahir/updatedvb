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
	mytune	= NULL;
	server	= NULL;
	socket	= NULL;
	IP		= QHostAddress::Null;
	port	= 0;
	socket_connected = false;
	server_connected = false;
}

dvbstream_thread::~dvbstream_thread()
{
	qDebug() << "~dvbstream_thread()";
	socket_close();
}

void dvbstream_thread::socket_new()
{
	if (!port) {
		return;
	}
	socket = server->nextPendingConnection();
	socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
	socket_connected = true;
	connect(socket, SIGNAL(disconnected()), this, SLOT(socket_close()), Qt::DirectConnection);
	connect(socket, SIGNAL(readyRead()), this, SLOT(read_data()), Qt::DirectConnection);
	emit update_status(QString("Streaming to %1").arg(socket->peerAddress().toString()), 0);
	stream();
}

void dvbstream_thread::read_data()
{
	QString line;
	while (socket->bytesAvailable()) {
		line = socket->readLine();
		if (line.section(" ", 0, 0) == "User-Agent:") {
			emit update_status(QString("Streaming to %1 @ %2").arg(line.section(" ", 1, 1)).arg(socket->peerAddress().toString()), 0);
		}
	}
}

void dvbstream_thread::socket_close()
{
	if (socket_connected) {
		socket_connected = false;
		socket->close();
		socket->deleteLater();
	}
	if (server_connected) {
		server_connected = false;
		server->close();
		server->deleteLater();
	}
	mytune->close_dvr();

	IP		= QHostAddress::Null;
	port	= 0;
	emit update_status("Disconnected", 1);
}

void dvbstream_thread::setup_server()
{
	QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
	for (int i = 0; i < ipAddressesList.size(); ++i) {
		if (ipAddressesList.at(i) != QHostAddress::LocalHost && ipAddressesList.at(i).toIPv4Address()) {
			IP = ipAddressesList.at(i);
			break;
		}
	}
	if (IP.isNull()) {
		IP = QHostAddress(QHostAddress::LocalHost);
	}
	port = 1230 + mytune->adapter;
	server = new QTcpServer();
	if (!server->listen(QHostAddress::Any, port)) {
		qDebug() << "Server could not start";
		return;
	}
	server_connected = true;
	connect(server, SIGNAL(newConnection()), this, SLOT(socket_new()), Qt::DirectConnection);
	emit update_status(QString("Streaming on %1:%2").arg(IP.toString()).arg(port), 0);
}

void dvbstream_thread::stream()
{
    QTextStream resultStream(socket);
    resultStream << "HTTP/1.0 200 Ok\r\n";
    resultStream << "Content-Type: video/MP2T\r\n";
    resultStream << "\r\n";
    resultStream.flush();

	while (socket_connected && socket->state() == QAbstractSocket::ConnectedState) {
		qint64 len = socket->write(mytune->demux_stream());
		// Fix this, this causes much better performance, but randomly crash's with sigpipe, catch the error and ignore it
		//socket->flush();
		socket->waitForBytesWritten(2000);
		if (len != TCP_BUFSIZE) {
			qDebug() << "TCP write() issue:" << len << "of" << TCP_BUFSIZE;
		}
	}
}
