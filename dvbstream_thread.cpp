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
}

dvbstream_thread::~dvbstream_thread()
{
	server_close();
	if (!socket.isNull()) {
		socket->deleteLater();
	}
	if (!server.isNull()) {
		server->deleteLater();
	}
}

void dvbstream_thread::socket_new()
{
	if (!socket.isNull()) {
		socket_close();
	}
	socket = server->nextPendingConnection();
	socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
	connect(socket, SIGNAL(disconnected()), this, SLOT(socket_close()), Qt::DirectConnection);
	connect(socket, SIGNAL(readyRead()), this, SLOT(read_data()), Qt::DirectConnection);
	emit update_status(QString("Streaming to %1").arg(socket->peerAddress().toString()), STATUS_NOEXP);
}

void dvbstream_thread::process_data()
{
	if (data.last() == "\r\n") {
		qDebug() << data;
		if (data.at(0).contains("GET / HTTP")) {
			for (int i = 1; i < data.size(); i++) {
				// VLC/MPV/MPlayer clients
				if (data.at(i).section(" ", 0, 0) == "User-Agent:"
						&& (data.at(i).contains("VLC") || data.at(i).contains("mpv") || data.at(i).contains("MPlayer"))) {
					emit update_status(QString("Streaming to %1 @ %2").arg(data.at(i).section(" ", 1, 1)).arg(socket->peerAddress().toString()), STATUS_NOEXP);
					mytune->demux_stream(true);

					socket->write("HTTP/1.0 200 OK\r\n");
					socket->write("Content-type: application/octet-stream\r\n");
					socket->write("Cache-Control : no-cache\r\n");
					socket->write("\r\n");
					socket->waitForBytesWritten(2000);
				}
			}
		}
		data.clear();
	}
}

void dvbstream_thread::read_data()
{
	while (socket->bytesAvailable()) {
		data.append(socket->readLine());
	}
	process_data();
}

void dvbstream_thread::socket_close()
{
	if (!socket.isNull()) {
		mytune->demux_stream(false);
		socket->close();
		emit update_status(QString("Streaming on %1:%2").arg(IP.toString()).arg(port), STATUS_NOEXP);
	}
}

void dvbstream_thread::server_close()
{
	socket_close();
	if (!server.isNull()) {
		server->close();
	}

	IP		= QHostAddress::Null;
	port	= 0;
	emit update_status("", STATUS_CLEAR);
	emit update_status("Disconnected", 2);
}

void dvbstream_thread::server_new()
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
		server_close();
		return;
	}
	connect(server, SIGNAL(newConnection()), this, SLOT(socket_new()), Qt::DirectConnection);
	emit update_status(QString("Streaming on %1:%2").arg(IP.toString()).arg(port), STATUS_NOEXP);
}

void dvbstream_thread::stream(QByteArray data)
{
	if (socket && socket->state() == QAbstractSocket::ConnectedState) {
		qint64 len = socket->write(data);
		socket->waitForBytesWritten(2000);
		if (len != LIL_BUFSIZE) {
			qDebug() << "TCP write() issue:" << len << "of" << LIL_BUFSIZE;
		}
	}
}
