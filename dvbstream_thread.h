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
#include <QNetworkInterface>
#include "dvbtune.h"
#include <signal.h>

class dvbstream_thread : public QObject
{
	Q_OBJECT
public:
	dvbstream_thread();
	~dvbstream_thread();
	void stream();

	dvbtune *mytune;
	QHostAddress IP;
	int port;

public slots:
	void socket_new();
	void socket_close();
	void setup_server();
	void read_data();

signals:
	void update_status(QString text, int time);

private:
	QTcpServer *server;
	QTcpSocket *socket;
	bool socket_connected;
	bool server_connected;
};

#endif // DVBSTREAM_THREAD_H
