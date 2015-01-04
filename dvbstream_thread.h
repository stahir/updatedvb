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

#ifndef DVBSTREAM_THREAD_H
#define DVBSTREAM_THREAD_H

#include <QThread>
#include <QPointer>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkInterface>
#include "dvbtune.h"
#include <signal.h>

class dvbstream_thread : public QObject
{
	Q_OBJECT
public:
	dvbstream_thread();
	~dvbstream_thread();
	QString user_agent();

	dvbtune *mytune;
	QHostAddress IP;
	int port;
	QPointer<QTcpServer> server;
	QPointer<QTcpSocket> socket;
	QVector<QString> data;

public slots:
	void socket_new();
	void socket_close();
	void appletv_new();
	void server_new();
	void server_close();
	void read_data();
	void stream(QByteArray data);
	void process_data();

signals:
	void update_status(QString text, int time);
	void send_stream();
};

#endif // DVBSTREAM_THREAD_H
