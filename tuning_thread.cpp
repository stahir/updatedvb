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

#include "tuning_thread.h"

tuning_thread::tuning_thread()
{
	mytune	= NULL;
	loop	= false;
	ready	= false;
	parsetp_loop	= false;
	parsetp_running	= false;
	pid_parent.fill(-1, 0x2000);
}

tuning_thread::~tuning_thread()
{
	parsetp_loop	= false;
	while (parsetp_running) {
		QThread::msleep(100);
	}
}

unsigned int tuning_thread::dtag_convert(unsigned int temp)
{
	int ret = 0;
	ret += ((temp >> 28) & 0x0F) * qPow(10,7);
	ret += ((temp >> 24) & 0x0F) * qPow(10,6);
	ret += ((temp >> 20) & 0x0F) * qPow(10,5);
	ret += ((temp >> 16) & 0x0F) * qPow(10,4);
	ret += ((temp >> 12) & 0x0F) * qPow(10,3);
	ret += ((temp >> 8) & 0x0F) * qPow(10,2);
	ret += ((temp >> 4) & 0x0F) * qPow(10,1);
	ret += ((temp >> 0) & 0x0F) * qPow(10,0);
	return ret;
}

void tuning_thread::tree_create_root_wait(int *parent, QString text, int pid = -1)
{
	if (!loop || *parent != -1) {
		return;
	}
	
	ready = false;
	emit tree_create_root(parent, text, pid);
	while (!ready) {
		msleep(10);
	}
}

void tuning_thread::tree_create_child_wait(int *parent, QString text, int pid = -1)
{
	if (!loop) {
		return;
	}
	
	ready = false;
	emit tree_create_child(parent, text, pid);
	while (!ready) {
		msleep(10);
	}
}

int tuning_thread::parse_etm(int parent, QString desc)
{
	int parent_t;

	unsigned int num_str = mytune->read8();
	for (unsigned int i = 0; i < num_str; i++) {
		QString lang = mytune->readstr(3);
		unsigned int num_seg = mytune->read8();
		for (unsigned int j = 0; j < num_seg; j++) {
			mytune->index += 2;
			unsigned int num_bytes = mytune->read8();
			QString msg = mytune->readstr(num_bytes);
			parent_t = parent;
			tree_create_child_wait(&parent_t, QString("Language: %1").arg(lang));
			parent_t = parent;
			tree_create_child_wait(&parent_t, QString("%1: %2").arg(desc).arg(msg));
		}
	}
	return 1;
}

int tuning_thread::parse_descriptor(int parent)
{
	int desc_tag = mytune->read8();
	int desc_len = mytune->read8();
	int desc_end = desc_len + mytune->index;
	if (dvbnames.dvb_descriptortag.at(desc_tag) == "") {
		tree_create_child_wait(&parent, QString("Unknown descriptor: 0x%1").arg(desc_tag,2,16,QChar('0')));
		qDebug().nospace() << "Unkown descriptor: 0x" << hex << desc_tag;
		mytune->index = desc_end;
		return 0;
	}
	int tmp[0xFF];
	int parent_t;

	switch(desc_tag) {
	case 0x09: // CA Descriptor
		emit setcolor(parent, Qt::red);
		tmp[0] = mytune->read16();
		tmp[1] = mytune->read16(0x1FFF);

		tree_create_child_wait(&parent, QString("Descriptor: 0x%1 - %2 Descriptor").arg(desc_tag,2,16,QChar('0')).arg(dvbnames.dvb_descriptortag[desc_tag]));
		tree_create_child_wait(&parent, QString("CA PID: 0x%1 - system_id: 0x%2 (%3)").arg(tmp[1],4,16,QChar('0')).arg(tmp[0],4,16,QChar('0')).arg(dvbnames.ca_name[tmp[0]]), tmp[1]);
		break;
	case 0x40: // network_name_descriptor
		tree_create_child_wait(&parent, QString("Network Name: %1").arg(mytune->readstr(desc_len)));
		break;
	case 0x41: // service_list_descriptor
		tree_create_child_wait(&parent, QString("Descriptor: 0x%1 - %2 Descriptor").arg(desc_tag,2,16,QChar('0')).arg(dvbnames.dvb_descriptortag[desc_tag]));
		while(mytune->index < desc_len) {
			parent_t = parent;
			tree_create_child_wait(&parent_t, QString("Service ID: %1 - %2").arg(mytune->read16()).arg(dvbnames.stream_type[mytune->read8()]));
		}
		break;
	case 0x43: //satellite_delivery_system_descriptor
		tree_create_child_wait(&parent, QString("Descriptor: 0x%1 - %2 Descriptor").arg(desc_tag,2,16,QChar('0')).arg(dvbnames.dvb_descriptortag[desc_tag]));

		unsigned int frequency;
		unsigned int symbol_rate;
		unsigned int temp;
		int orbital_pos;
		int east_west_flag;
		int polarization;
		int roll_off;
		int modulation_system;
		int modulation_type;
		int fec_inner;

		frequency = dtag_convert(mytune->read32());
		orbital_pos = dtag_convert(mytune->read16());

		temp = mytune->read8();
		east_west_flag = mytune->maskbits(temp, 0x80);
		polarization = mytune->maskbits(temp, 0x60);
		roll_off = mytune->maskbits(temp, 0x18);
		modulation_system = mytune->maskbits(temp, 0x04);
		modulation_type = mytune->maskbits(temp, 0x03);

		temp = mytune->read32();
		symbol_rate = dtag_convert(mytune->maskbits(temp, 0xFFFFFFF0));
		fec_inner = mytune->maskbits(temp, 0x0F);

		parent_t = parent;
		tree_create_child_wait(&parent_t, QString("Frequency: %1 %2 %3 , fec = %4").arg(frequency/100).arg(dvbnames.dtag_polarization[polarization]).arg(symbol_rate/10).arg(dvbnames.dtag_fec[fec_inner]));

		parent_t = parent;
		tree_create_child_wait(&parent_t, QString("Modulation System: %1").arg(modulation_system ? "DVB-S2" : "DVB-S"));

		parent_t = parent;
		tree_create_child_wait(&parent_t, QString("Modulation Type: %1").arg(dvbnames.dtag_modulation[modulation_type]));

		if (modulation_system) {
			parent_t = parent;
			tree_create_child_wait(&parent_t, QString("Roll Off: %1").arg(dvbnames.dtag_rolloff[roll_off]));
		}
		parent_t = parent;
		tree_create_child_wait(&parent_t, QString("Orbital Position: %L1%2").arg(QString::number(orbital_pos/10.0, 'f', 1)).arg(east_west_flag ? "e" : "w"));
		break;
	case 0x48: // service_descriptor
		mytune->index += 1;
		mysdt.pname.append(mytune->readstr(mytune->read8()));
		mysdt.sname.append(mytune->readstr(mytune->read8()));
		if (mysdt.sname.last() != "") {
			parent_t = parent;
			tree_create_child_wait(&parent_t, QString("Service Name: %1").arg(mysdt.sname.last()));
		}
		if (mysdt.pname.last() != "") {
			parent_t = parent;
			tree_create_child_wait(&parent_t, QString("Provider Name: %1").arg(mysdt.pname.last()));
		}
		break;
	case 0x4d:
		tree_create_child_wait(&parent, QString("Descriptor: 0x%1 - %2 Descriptor").arg(desc_tag,2,16,QChar('0')).arg(dvbnames.dvb_descriptortag[desc_tag]));
		parent_t = parent;
		tree_create_child_wait(&parent_t, QString("Language: %1").arg(mytune->readstr(3)));
		parent_t = parent;
		tree_create_child_wait(&parent_t, QString("Event Name: %1").arg(mytune->readstr(mytune->read8())));
		parent_t = parent;
		tree_create_child_wait(&parent_t, QString("Text: %1").arg(mytune->readstr(mytune->read8())));
		break;
	case 0xA0: // extended_channel_name_descriptor
	{
		tree_create_child_wait(&parent, QString("Descriptor: 0x%1 - %2 Descriptor").arg(desc_tag,2,16,QChar('0')).arg(dvbnames.dvb_descriptortag[desc_tag]));
		while (mytune->index < desc_end) {
			int number_strings = mytune->read8();
			for (int i = 0; i < number_strings; i++) {
				mytune->index += 3;
				int number_segments = mytune->read8();
				for (int a = 0; a < number_segments; a++) {
					mytune->index += 2;
					parent_t = parent;
					tree_create_child_wait(&parent_t, QString("Long Channel Name: %1").arg(mytune->readstr(mytune->read8())));
				}
			}
		}
		break;
	}
	default:
		tree_create_child_wait(&parent, QString("Descriptor: 0x%1 - %2 Descriptor").arg(desc_tag,2,16,QChar('0')).arg(dvbnames.dvb_descriptortag[desc_tag]));
		break;
	}
	mytune->index = desc_end;
	return 1;
}

int tuning_thread::parse_psip_tvct()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return -1;
	}
	mytune->packet_processed.append(mytune->buffer);

	unsigned int pid = mytune->dvbdata.first().pid;
	if (pid_parent[pid] == -1) {
		tree_create_root_wait(&pid_parent[pid], QString("PSIP pid: 0x%1").arg(pid,4,16,QChar('0')), pid);
		emit setcolor(pid_parent[pid], Qt::green);
	}

	int parent = pid_parent[pid];
	int parent_1, parent_2;

	parent_1 = parent;
	tree_create_child_wait(&parent_1, QString("TVCT - Terrestrial Virtual Channel Table"));

	int section_length = mytune->read16(0x0FFF) + mytune->index - 4;
	Q_UNUSED(section_length);

	mytune->index += 6;

	int num_channels_in_section = mytune->read8();
	for (int i = 0; i < num_channels_in_section; i++) {
		QString short_name = mytune->readstr16(7);
		parent_2 = parent_1;
		tree_create_child_wait(&parent_2, QString("Name: %1").arg(short_name));
		parent_2 = parent_1;
		int channel = mytune->read24();
		tree_create_child_wait(&parent_2, QString("Channel Number: %1-%2").arg(mytune->maskbits(channel, 0xFFC00)).arg(mytune->maskbits(channel, 0x3FF)));

		mytune->index += 13;
		int descriptors_length = mytune->index + mytune->read16(0x3FF);
		while (mytune->index < descriptors_length) {
			parent_2 = parent_1;
			parse_descriptor(parent_2);
		}
	}
	int additional_descriptors_length = mytune->index + mytune->read16(0x3FF);
	while (mytune->index < additional_descriptors_length) {
		parent_2 = parent_1;
		parse_descriptor(parent_2);
	}
	return 1;
}

int tuning_thread::parse_psip_eit()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return -1;
	}
	mytune->packet_processed.append(mytune->buffer);

	unsigned int pid = mytune->dvbdata.first().pid;
	if (pid_parent[pid] == -1) {
		tree_create_root_wait(&pid_parent[pid], QString("PSIP pid: 0x%1").arg(pid,4,16,QChar('0')), pid);
		emit setcolor(pid_parent[pid], Qt::green);
	}

	int parent = pid_parent[pid];
	int parent_1, parent_2, parent_3, parent_4;

	parent_1 = parent;
	tree_create_child_wait(&parent_1, QString("EIT - Event Information Table"));

	int section_length = mytune->read16(0x0FFF) + mytune->index - 4;
	Q_UNUSED(section_length);

	unsigned int sourceID = mytune->read16();
	parent_2 = parent_1;
	tree_create_child_wait(&parent_2, QString("SourceID: %1").arg(sourceID,2,16,QChar('0')));

	mytune->index += 4;

	int num_channels_in_section = mytune->read8();
	for (int i = 0; i < num_channels_in_section; i++) {
		unsigned int event_id = mytune->read16(0x3FFF);
		parent_3 = parent_2;
		tree_create_child_wait(&parent_3, QString("Event ID: 0x%1").arg(event_id,4,16,QChar('0')));

		__u32 stime = mytune->read32();
		__u32 dtime = mytune->read24(0x0FFF);

		mytune->index++;

		parent_4 = parent_3;
		parse_etm(parent_4, "Channel Name");

		QDateTime start_time = QDateTime::fromTime_t(stime + 315964800);
		parent_4 = parent_3;
		tree_create_child_wait(&parent_4, QString("Start Time: %1").arg(start_time.toString()));
		unsigned int h = dtime/60/60;
		unsigned int m = dtime/60 - (h*60);
		unsigned int s = dtime - (h*60*60) - (m*60);
		QTime duration = QTime(h, m, s);
		parent_4 = parent_3;
		tree_create_child_wait(&parent_4, QString("Duration: %1").arg(duration.toString()));

		int descriptors_length = mytune->index + mytune->read16(0xFFF);
		while (mytune->index < descriptors_length) {
			parent_4 = parent_3;
			parse_descriptor(parent_4);
		}
	}
	return 1;
}

int tuning_thread::parse_psip_stt()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return -1;
	}
	mytune->packet_processed.append(mytune->buffer);

	unsigned int pid = mytune->dvbdata.first().pid;
	if (pid_parent[pid] == -1) {
		tree_create_root_wait(&pid_parent[pid], QString("PSIP pid: 0x%1").arg(pid,4,16,QChar('0')), pid);
		emit setcolor(pid_parent[pid], Qt::green);
	}

	int parent = pid_parent[pid];
	int parent_1, parent_2;

	parent_1 = parent;
	// If you change this text, change it in tuning::tree_create_child() as well
	tree_create_child_wait(&parent_1, QString("STT - System Time Table"));

	int section_length = mytune->read16(0x0FFF) + mytune->index - 4;
	Q_UNUSED(section_length);

	mytune->index += 6;

	__u32 stime = mytune->read32();

	parent_2 = parent_1;
	QDateTime start_time = QDateTime::fromTime_t(stime + 315964800);
	tree_create_child_wait(&parent_2, QString("System Time: %1").arg(start_time.toString()));

	mytune->index += 3;

	while (mytune->index < section_length) {
		parent_2 = parent_1;
		parse_descriptor(parent_2);
	}
	return 1;
}

int tuning_thread::parse_psip_mgt()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return -1;
	}
	mytune->packet_processed.append(mytune->buffer);

	unsigned int pid = mytune->dvbdata.first().pid;
	if (pid_parent[pid] == -1) {
		tree_create_root_wait(&pid_parent[pid], QString("PSIP pid: 0x%1").arg(pid,4,16,QChar('0')), pid);
		emit setcolor(pid_parent[pid], Qt::green);
	}

	int parent = pid_parent[pid];
	int parent_1, parent_2;

	parent_1 = parent;
	tree_create_child_wait(&parent_1, QString("MGT - Master Guide Table"));

	int section_length = mytune->read16(0x0FFF) + mytune->index - 4;
	Q_UNUSED(section_length);

	mytune->index += 6;

	unsigned int num_tables = mytune->read16();
	for (unsigned int i = 0; i < num_tables; i++) {
		unsigned int table_type	= mytune->read16();
		unsigned int table_pid	= mytune->read16(0x1FFF);
		mytune->index++;
		__u32 table_size		= mytune->read32();
		parent_2 = parent_1;
		tree_create_child_wait(&parent_2, QString("Table Type/PID/Size: 0x%1 / 0x%2 / 0x%3").arg(table_type,4,16,QChar('0')).arg(table_pid,4,16,QChar('0')).arg(table_size));

		if (table_type >= 0x0100 && table_type <= 0x17F) {
			filter_pids(table_pid, 0xCB);
		} else if ((table_type >= 0x0200 && table_type <= 0x27F) || table_type == 0x0004) {
			filter_pids(table_pid, 0xCC);
		} else if ((table_type >= 0x0301 && table_type <= 0x3FF)) {
			filter_pids(table_pid, 0xCA);
		} else if (table_type == 0x0000) {
			filter_pids(table_pid, 0xC8);
		} else {
			qDebug() << Q_FUNC_INFO << "Unkown Table_type:" << hex << table_pid << table_type;
		}

		unsigned int desc_len	= mytune->read16(0x0FFF);
		for (unsigned int a = 0; a < desc_len; a++) {
			parent_2 = parent_1;
			parse_descriptor(parent_2);
		}
	}
	unsigned int desc_len	= mytune->read16(0x0FFF);
	for (unsigned int a = 0; a < desc_len; a++) {
		parent_2 = parent_1;
		parse_descriptor(parent_2);
	}
	return 1;
}

int tuning_thread::parse_psip_ett()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return -1;
	}
	mytune->packet_processed.append(mytune->buffer);

	unsigned int pid = mytune->dvbdata.first().pid;
	if (pid_parent[pid] == -1) {
		tree_create_root_wait(&pid_parent[pid], QString("PSIP pid: 0x%1").arg(pid,4,16,QChar('0')), pid);
		emit setcolor(pid_parent[pid], Qt::green);
	}

	int parent = pid_parent[pid];
	int parent_1;

	unsigned int section_length		= mytune->read16(0x0FFF) + mytune->index - 4;
	Q_UNUSED(section_length);
	unsigned int ETT_table_id_ext	= mytune->read16();

	mytune->index += 4;

	unsigned int ETM_id = mytune->read32();

	parent_1 = parent;
	tree_create_child_wait(&parent_1, QString("ETT Table Ext/ID : 0x%1 - 0x%2").arg(ETT_table_id_ext,4,16,QChar('0')).arg(ETM_id,8,16,QChar('0')));

	parse_etm(parent_1, "Text");

	return 1;
}

int tuning_thread::parse_psip_rrt()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return -1;
	}
	mytune->packet_processed.append(mytune->buffer);

	unsigned int pid = mytune->dvbdata.first().pid;
	if (pid_parent[pid] == -1) {
		tree_create_root_wait(&pid_parent[pid], QString("PSIP pid: 0x%1").arg(pid,4,16,QChar('0')), pid);
		emit setcolor(pid_parent[pid], Qt::green);
	}

	int parent = pid_parent[pid];
	int parent_1, parent_2;

	parent_1 = parent;
	tree_create_child_wait(&parent_1, QString("Rating Region Table"));

	unsigned int section_length		= mytune->read16(0x0FFF) + mytune->index - 4;
	Q_UNUSED(section_length);

	mytune->index += 7;

	parent_2 = parent_1;
	parse_etm(parent_2, "Rating Region Name");

	unsigned int dimensions_defined = mytune->read8();
	for (unsigned int i = 0; i < dimensions_defined; i++) {
		mytune->index++;
		parent_2 = parent_1;
		parse_etm(parent_2, "Dimension Name");

		unsigned int values_defined = mytune->read8(0x0F);
		for (unsigned int j = 0; j < values_defined; j++) {
			mytune->index++;
			parent_2 = parent_1;
			parse_etm(parent_2, "Abbreviated Rating Name");
			mytune->index++;
			parent_2 = parent_1;
			parse_etm(parent_2, "Rating Name");
		}
	}

	int descriptors_loop_length = mytune->read16(0x03FF) + mytune->index;
	while (mytune->index < descriptors_loop_length) {
		parse_descriptor(parent_2);
	}

	return 1;
}

int tuning_thread::parse_eit()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return -1;
	}
	mytune->packet_processed.append(mytune->buffer);

	if (pid_parent[0x12] == -1) {
		tree_create_root_wait(&pid_parent[0x12], "EIT pid: 0x12", 0x12);
		emit setcolor(pid_parent[0x12], Qt::green);
	}

	int parent = pid_parent[0x12];
	int parent_1, parent_2;

	emit setcolor(parent, Qt::green);
	int section_length = mytune->read16(0x0FFF) + mytune->index - 4;
	parent_1 = parent;
	tree_create_child_wait(&parent_1, QString("Program Number: %1").arg(mytune->read16()));
	mytune->index += 9;
	while (mytune->index < section_length) {
		parent_2 = parent_1;
		tree_create_child_wait(&parent_2, QString("Event ID: %1").arg(mytune->read16()));
		__u16 t1 = mytune->read16();
		__u8  t2 = mytune->read8();
		__u8  t3 = mytune->read8();
		__u8  t4 = mytune->read8();
		__u8  t5 = mytune->read8();
		__u8  t6 = mytune->read8();
		__u8  t7 = mytune->read8();
		parent_2 = parent_1;
		tree_create_child_wait(&parent_2, QString("Start Date/Time: %1 %2:%3:%4").arg(QDate::fromJulianDay(t1 + 2400000.5).toString()).arg(dtag_convert(t2), 2, 10, QChar('0')).arg(dtag_convert(t3), 2, 10, QChar('0')).arg(dtag_convert(t4), 2, 10, QChar('0')));
		parent_2 = parent_1;
		tree_create_child_wait(&parent_2, QString("Duration: %1:%2:%3").arg(dtag_convert(t5), 2, 10, QChar('0')).arg(dtag_convert(t6), 2, 10, QChar('0')).arg(dtag_convert(t7), 2, 10, QChar('0')));

		int descriptors_loop_length = mytune->read16(0x0FFF) + mytune->index;
		while (mytune->index < descriptors_loop_length) {
			parse_descriptor(parent_2);
		}
	}
	return 1;
}

int tuning_thread::parse_sdt()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return -1;
	}
	mytune->packet_processed.append(mytune->buffer);

	if (pid_parent[0x11] == -1) {
		tree_create_root_wait(&pid_parent[0x11], "SDT pid: 0x11", 0x11);
		emit setcolor(pid_parent[0x11], Qt::green);
	}

	mysdt.sid.clear();
	mysdt.sname.clear();
	mysdt.pname.clear();

	int parent = pid_parent[0x11];
	int parent_t;

	emit setcolor(parent, Qt::green);
	int section_length = mytune->read16(0x0FFF) + mytune->index - 4;
	mytune->index += 8;
	while (mytune->index < section_length) {
		mysdt.sid.append(mytune->read16());

		parent_t = parent;
		tree_create_child_wait(&parent_t, QString("Service ID: %1").arg(mysdt.sid.last()));

		mytune->index += 1;
		int descriptors_loop_length = mytune->read16(0x0FFF) + mytune->index;
		while (mytune->index < descriptors_loop_length) {
			parse_descriptor(parent_t);
		}
	}
	return 1;
}

int tuning_thread::parse_pat()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return -1;
	}
	mytune->packet_processed.append(mytune->buffer);

	if (pid_parent[0x00] == -1) {
		tree_create_root_wait(&pid_parent[0x00], "PAT pid: 0x00", 0x00);
		emit setcolor(pid_parent[0x00], Qt::green);
	}

	mypat.number.clear();
	mypat.pid.clear();

	int section_length = mytune->read16(0x0FFF) - 4;
	mytune->index += 5;
	while (mytune->index < section_length) {
		mypat.number.append(mytune->read16());
		mypat.pid.append(mytune->read16(0x1FFF));
	}
	return 1;
}

int tuning_thread::parse_cat()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return -1;
	}
	mytune->packet_processed.append(mytune->buffer);

	if (pid_parent[0x01] == -1) {
		tree_create_root_wait(&pid_parent[0x01], "CAT pid: 0x01", 0x01);
		emit setcolor(pid_parent[0x01], Qt::red);
	}
	int parent = pid_parent[0x01];

	int section_length = mytune->read16(0x0FFF) - 4;
	mytune->index += 5;
	while (mytune->index < section_length) {
		int parent_t = parent;
		parse_descriptor(parent_t);
	}
	return 1;
}

int tuning_thread::parse_nit()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return -1;
	}
	mytune->packet_processed.append(mytune->buffer);

	if (pid_parent[0x10] == -1) {
		tree_create_root_wait(&pid_parent[0x10], "NIT pid: 0x10", 0x10);
		emit setcolor(pid_parent[0x10], Qt::green);
	}

	int section_length = mytune->read16(0x0FFF);

	int parent = pid_parent[0x10];
	int parent_t, parent_2;

	emit setcolor(parent, Qt::green);
	parent_t = parent;
	tree_create_child_wait(&parent_t, QString("Network ID: 0x%1").arg(mytune->read16(),4,16,QChar('0')));

	mytune->index += 3;
	int network_desc_length = mytune->read16(0x0FFF) + mytune->index;
	while (mytune->index < network_desc_length && mytune->index < section_length) {
		parse_descriptor(parent);
	}

	int transport_stream_loop_length = mytune->read16(0x0FFF) + mytune->index;
	while (mytune->index < transport_stream_loop_length) {
		int transport_stream_id = mytune->read16();
		int original_network_id = mytune->read16();
		int transport_desc_length = mytune->read16(0x0FFF) + mytune->index;

		parent_2 = parent;
		tree_create_child_wait(&parent_2, QString("Transport Stream ID: 0x%1, Original Network ID: 0x%2").arg(transport_stream_id,4,16,QChar('0')).arg(original_network_id,4,16,QChar('0')));
		while (mytune->index < transport_desc_length && mytune->index < section_length) {
			parse_descriptor(parent_2);
		}
	}
	return 1;
}

int tuning_thread::parse_pmt()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return -1;
	}
	mytune->packet_processed.append(mytune->buffer);

	int parent_1 = pid_parent[0x00];
	int parent_2;

	int section_length = mytune->read16(0x0FFF);
	section_length += mytune->index - 4;
	unsigned int pmt_num = mytune->read16();
	unsigned int pmt_pid = mytune->dvbdata.first().pid;

	tree_create_child_wait(&parent_1, QString("PMT PID: 0x%1 - Program: %2").arg(pmt_pid,4,16,QChar('0')).arg(pmt_num,4,10,QChar(' ')), pmt_pid);
	emit setcolor(parent_1, Qt::green);

	mytune->index += 3;
	unsigned int pmt_pcr = mytune->read16(0x1FFF);
	parent_2 = parent_1;
	tree_create_child_wait(&parent_2, QString("PCR: 0x%1").arg(pmt_pcr,4,16,QChar('0')), pmt_pcr);

	int program_info_length = mytune->read16(0x0FFF) + mytune->index;
	while (mytune->index < program_info_length) {
		parse_descriptor(parent_1);
	}

	while (mytune->index < section_length) {
		unsigned int desc_type	= mytune->read8();
		unsigned int desc_pid	= mytune->read16(0x1FFF);

		parent_2 = parent_1;
		tree_create_child_wait(&parent_2, QString("Stream PID: 0x%1, 0x%2 - %3").arg(desc_pid,4,16,QChar('0')).arg(desc_type,2,16,QChar('0')).arg(dvbnames.stream_type[desc_type]), desc_pid);

		int ES_info_length = mytune->read16(0x0FFF) + mytune->index;
		while (mytune->index < ES_info_length) {
			parse_descriptor(parent_2);
		}
	}
	return 1;
}

int tuning_thread::parse_tdt()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return -1;
	}
	mytune->packet_processed.append(mytune->buffer);

	if (pid_parent[0x14] == -1) {
		tree_create_root_wait(&pid_parent[0x14], "TDT pid: 0x14", 0x14);
		emit setcolor(pid_parent[0x14], Qt::green);
	}
	int parent = pid_parent[0x14];
	int parent_t;

	mytune->index += 2;
	__u16 t1 = mytune->read16();
	__u8  t2 = mytune->read8();
	__u8  t3 = mytune->read8();
	__u8  t4 = mytune->read8();
	parent_t = parent;
	// If you change this text, change it in tuning::tree_create_child() as well
	tree_create_child_wait(&parent_t, QString("UTC Date/Time: %1 %2:%3:%4").arg(QDate::fromJulianDay(t1 + 2400000.5).toString()).arg(dtag_convert(t2), 2, 10, QChar('0')).arg(dtag_convert(t3), 2, 10, QChar('0')).arg(dtag_convert(t4), 2, 10, QChar('0')), 0x14);

	return 1;
}

int tuning_thread::parse_dcii_sdt()
{
	return -1; // I need to tune a DCII tp to test this before adding it again

//	parent_2 = parent_1;
//	tree_create_child_wait(&parent_2, "SDT");
//	mytune->index += 15;
//	parent_2 = parent_1;
//	tree_create_child_wait(&parent_2, QString("Service Name: %1").arg(mytune->readstr(mytune->read8())));
}

void tuning_thread::filter_pids(unsigned int pid, unsigned int table)
{
	if (mytune->pids_rate.at(pid) > 0) {
		for (int i = 0; i < fpids.pid.size(); i++) {
			if (fpids.pid.at(i) == pid && fpids.tbl.at(i) == table) {
				return;
			}
		}
		fpids.pid.append(pid);
		fpids.tbl.append(table);
	}
}

void tuning_thread::parsetp()
{
	parsetp_running = true;

	mytune->get_bitrate();
	emit list_create();

	parsetp_loop = true;
	while (parsetp_loop) {
		filter_pids(0x01, 0x01);
		filter_pids(0x11, 0x42);
		filter_pids(0x1FFB, 0xC7);
		filter_pids(0x1FFB, 0xC8);
		filter_pids(0x1FFB, 0xCB);
		filter_pids(0x1FFB, 0xCD);
		filter_pids(0x1FFB, 0xCA);
		filter_pids(0x12, 0x4E);
		filter_pids(0x10, 0x40);
		filter_pids(0x00, 0x00);
		filter_pids(0x14, 0x70);
		for (int i = 0; i < mypat.pid.size(); i++) {
			filter_pids(mypat.pid.at(i), 0x02);
		}
		mytune->demux_packets(fpids);

		while (!mytune->dvbdata.isEmpty()) {
			mytune->buffer = mytune->dvbdata.first().buffer;
			mytune->index = 0;
			switch (mytune->read8()) { // Table ID
			case 0x4E:
				parse_eit();
				break;
			case 0x42:
				parse_sdt();
				break;
			case 0xCB:
				parse_psip_eit();
				break;
			case 0xC7:
				parse_psip_mgt();
				break;
			case 0xC8:
				parse_psip_tvct();
				break;
			case 0xCA:
				parse_psip_rrt();
				break;
			case 0xCC:
				parse_psip_ett();
				break;
			case 0xCD:
				parse_psip_stt();
				break;
			case 0x00:
				parse_pat();
				break;
			case 0x01:
				parse_cat();
				break;
			case 0x40:
				parse_nit();
				break;
			case 0x02:
				parse_pmt();
				break;
			case 0x70:
				parse_tdt();
				break;
			case 0xC1:
				parse_dcii_sdt();
				break;
			default:
				qDebug() << Q_FUNC_INFO << "Unkown TableID:" << hex << mytune->dvbdata.first().pid << mytune->dvbdata.first().table;
				break;
			}
			mytune->dvbdata.removeFirst();
		}
		mytune->get_bitrate();
		emit list_create();
	}
	mytune->stop_demux();
	parsetp_running = false;
	emit parsetp_done();
}

void tuning_thread::run()
{
	loop = true;
	do {
		if (thread_function.contains("parsetp")) {
			thread_function.remove(thread_function.indexOf("parsetp"));
			parsetp();
		}
		if (thread_function.isEmpty()) {
			msleep(100);
		}
	} while (loop);
	thread_function.clear();
}

void tuning_thread::closeEvent(QCloseEvent *event)
{
	Q_UNUSED(event);
	loop = false;
	this->deleteLater();
}
