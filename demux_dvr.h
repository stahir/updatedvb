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

#ifndef DEMUX_DVR_H
#define DEMUX_DVR_H

#include <QDialog>
#include "dvbtune.h"

namespace Ui {
class demux_dvr;
}

class demux_dvr : public QDialog
{
	Q_OBJECT
	
public:
	explicit demux_dvr(QWidget *parent = 0);
	~demux_dvr();
	void updatetxt(QString text);
	
private:
	Ui::demux_dvr *ui;
};

#endif // DEMUX_DVR_H
