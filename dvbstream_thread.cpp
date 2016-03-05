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
		socket->close();
		socket->waitForDisconnected(1000);
	}
	socket = server->nextPendingConnection();
	socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
	connect(socket, SIGNAL(disconnected()), this, SLOT(socket_close()), Qt::DirectConnection);
	connect(socket, SIGNAL(readyRead()), this, SLOT(read_data()), Qt::DirectConnection);
	emit update_status(QString("Streaming to %1").arg(socket->peerAddress().toString()), STATUS_NOEXP);
}

void dvbstream_thread::process_data()
{
	if (data.last() != "\r\n") {
		return;
	}
	qDebug() << data;

	if (data.at(0).contains("GET / HTTP")) {
		QString ua = user_agent();
		// VLC/MPV/MPlayer clients
		if (ua.contains("VLC") || ua.contains("mpv") || ua.contains("MPlayer") || ua.contains("Enigma2")) {
			socket->write("HTTP/1.0 200 OK\r\n");
			socket->write("Content-type: application/octet-stream\r\n");
			socket->write("Cache-Control : no-cache\r\n");
			socket->write("\r\n");
			socket->waitForBytesWritten(2000);

			emit update_status(QString("Streaming to %1 @ %2").arg(ua).arg(socket->peerAddress().toString()), STATUS_NOEXP);
			mytune->demux_stream(true);
		}
		// AppleTV
		if (ua.contains("AppleCoreMedia")) {
			QString output;
			output.append("#EXTM3U\r\n");
			output.append("#EXT-X-MEDIA:TYPE=AUDIO,GROUP-ID=\"audio\",LANGUAGE=\"und\",NAME=\"Original Audio\",DEFAULT=YES,AUTOSELECT=YES\r\n");
			output.append("#EXT-X-STREAM-INF:PROGRAM-ID=1,BANDWIDTH=20000000,CODECS=\"mp4a.40.2,avc1.640028\",AUDIO=\"audio\"\r\n");
			output.append("/stream/0.m3u8\r\n");
			output.append("#EXT-X-STREAM-INF:PROGRAM-ID=1,BANDWIDTH=20000000,CODECS=\"ac-3,avc1.640028\",AUDIO=\"audio\"\r\n");
			output.append("/stream/1.m3u8\r\n");
			output.append("#EXT-X-ENDLIST\r\n");
			output.append("\r\n");

			QString header;
			header.append("HTTP/1.1 206 Partial Content\r\n");
			header.append(QString("Content-Range: bytes 0-%1/%2\r\n").arg(output.size()-1).arg(output.size()));
			header.append(QString("Content-Length: %1\r\n").arg(output.size()));
			header.append("Accept-Ranges: bytes\r\n");
			header.append("Content-Type: application/vnd.apple.mpegurl\r\n");
			header.append(QString("Date: %1 GMT\r\n").arg(QDateTime::currentDateTime().toString("ddd, dd MMMM yyyy hh:mm:ss")));
			header.append("\r\n");

			socket->write(header.toStdString().c_str());
			socket->write(output.toStdString().c_str());
			socket->waitForBytesWritten(2000);
		}
	}

	// Apple TV
	if (data.at(0).contains("GET /stream/0.m3u8 HTTP")) {
		QString output;
		output.append("#EXTM3U\r\n");
		output.append("#EXT-X-TARGETDURATION:10\r\n");
		output.append("#EXT-X-PLAYLIST-TYPE:VOD\r\n");
		output.append("#EXT-X-ALLOW-CACHE:YES\r\n");
		output.append("#EXT-X-MEDIA-SEQUENCE:0\r\n");
		for (int a = 0; a < 100; a++) {
			output.append("#EXTINF:10,\r\n");
			output.append(QString("/stream/0/%1.ts\r\n").arg(a).toStdString().c_str());
		}
		output.append("#EXT-X-ENDLIST\r\n");
		output.append("\r\n");

		QString header;
		header.append("HTTP/1.1 200 OK\r\n");
		header.append("Accept-Ranges: bytes\r\n");
		header.append(QString("Content-Length: %1\r\n").arg(output.size()));
		header.append("Content-Type: application/vnd.apple.mpegurl\r\n");
		header.append(QString("Date: %1 GMT\r\n").arg(QDateTime::currentDateTime().toString("ddd, dd MMMM yyyy hh:mm:ss")));
		header.append("\r\n");

		socket->write(header.toStdString().c_str());
		socket->write(output.toStdString().c_str());
		socket->waitForBytesWritten(2000);
	}
	// Apple TV
	if (data.at(0).contains("GET /stream/1.m3u8 HTTP")) {
		QString output;
		output.append("#EXTM3U\r\n");
		output.append("#EXT-X-VERSION:3\r\n");
		output.append("#EXT-X-TARGETDURATION:10\r\n");
		output.append("#EXT-X-PLAYLIST-TYPE:VOD\r\n");
		output.append("#EXT-X-ALLOW-CACHE:YES\r\n");
		output.append("#EXT-X-MEDIA-SEQUENCE:0\r\n");
		for (int a = 0; a < 100; a++) {
			output.append("#EXTINF:10,\r\n");
			output.append(QString("/stream/1/%1.ts\r\n").arg(a).toStdString().c_str());
		}
		output.append("#EXT-X-ENDLIST\r\n");
		output.append("\r\n");

		QString header;
		header.append("HTTP/1.1 200 OK\r\n");
		header.append("Accept-Ranges: bytes\r\n");
		header.append(QString("Content-Length: %1\r\n").arg(output.size()));
		header.append("Content-Type: application/vnd.apple.mpegurl\r\n");
		header.append(QString("Date: %1 GMT\r\n").arg(QDateTime::currentDateTime().toString("ddd, dd MMMM yyyy hh:mm:ss")));
		header.append("\r\n");

		socket->write(header.toStdString().c_str());
		socket->write(output.toStdString().c_str());
		socket->waitForBytesWritten(2000);
	}
	// Apple TV
	if (data.at(0).contains("GET /stream/1/")) {
		QString ua = user_agent();
		if (ua.contains("AppleCoreMedia")) {
			unsigned long int bitrate = 0;
			for (int a = 0; a < mytune->pids.size(); a++) {
				bitrate += mytune->pids_rate.at(mytune->pids.at(a));
			}
			bitrate *= 5000;
			bitrate /= 8;

			QString header;		
			header.append("HTTP/1.1 200 OK\r\n");
			header.append("Accept-Ranges: bytes\r\n");
			header.append(QString("Content-Length: %1\r\n").arg(bitrate));
			header.append("Content-Type: video/MP2T\r\n");
			header.append(QString("Date: %1 GMT\r\n").arg(QDateTime::currentDateTime().toString("ddd, dd MMMM yyyy hh:mm:ss")));
			header.append("\r\n");

			socket->write(header.toStdString().c_str());
			socket->waitForBytesWritten(2000);

			emit update_status(QString("Streaming to %1 @ %2").arg(ua).arg(socket->peerAddress().toString()), STATUS_NOEXP);
			mytune->demux_stream(true);
		}
	}
	// Apple TV
	if (data.at(0).contains("GET /stream/0/")) {
		QString ua = user_agent();
		if (ua.contains("AppleCoreMedia")) {
			QString header;
			header.append("HTTP/1.1 200 OK\r\n");
			header.append("Accept-Ranges: bytes\r\n");
			header.append("Content-Length: 65424\r\n");
			header.append("Content-Type: video/MP2T\r\n");
			header.append(QString("Date: %1 GMT\r\n").arg(QDateTime::currentDateTime().toString("ddd, dd MMMM yyyy hh:mm:ss")));
			header.append("\r\n");

			socket->write(header.toStdString().c_str());
			socket->waitForBytesWritten(2000);

			emit update_status(QString("Streaming to %1 @ %2").arg(ua).arg(socket->peerAddress().toString()), STATUS_NOEXP);
			mytune->demux_stream(true);
		}
	}
	data.clear();
}

QString dvbstream_thread::user_agent()
{
	for (int i = 1; i < data.size(); i++) {
		if (data.at(i).section(" ", 0, 0) == "User-Agent:") {
			return data.at(i).section(" ", 1, 1);
		}
	}
	return "";
}

void dvbstream_thread::appletv_new()
{
	socket = new QTcpSocket;
	socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
	socket->connectToHost("10.0.1.62", 7000);
	if (!socket->waitForConnected(1000)) {
		return;
	}
	connect(socket, SIGNAL(disconnected()), this, SLOT(socket_close()), Qt::DirectConnection);
	connect(socket, SIGNAL(readyRead()), this, SLOT(read_data()), Qt::DirectConnection);

	QString output;
	output.append(QString("Content-Location: http://%1:%2/\r\n").arg(IP.toString()).arg(port));
	output.append("Start-Position: 0.000000\r\n");
	output.append("\r\n");

	QString header;
	header.append("POST /play HTTP/1.1\r\n");
	header.append("User-Agent: iTunes/11.0.2\r\n");
	header.append(QString("Content-Length: %1\r\n").arg(output.size()));
	header.append("\r\n");

	socket->write(header.toStdString().c_str());
	socket->write(output.toStdString().c_str());
	socket->waitForBytesWritten(1000);
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
	if (!socket.isNull() && socket->state() == QAbstractSocket::ConnectedState) {
		qint64 len = socket->write(data);
		socket->waitForBytesWritten(2000);
		if (len != LIL_BUFSIZE) {
			qDebug() << "TCP write() issue:" << len << "of" << LIL_BUFSIZE;
		}
	}
}
