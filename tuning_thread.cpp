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

void tuning_thread::tree_create_wait(tree_item *item)
{
	if (!loop) {
		return;
	}
	
	ready = false;
	emit tree_create(item);
	while (!ready) {
		msleep(10);
	}
}

void tuning_thread::parse_etm(tree_item *item, QString desc)
{
	tree_item orig;
	orig.parent			= item->parent;
	orig.return_parent	= item->return_parent;
	item->parent		= item->current;

	unsigned int num_str = mytune->read8();
	for (unsigned int i = 0; i < num_str; i++) {
		item->text = QString("Language: %1").arg(mytune->readstr(3));
		tree_create_wait(item);
		unsigned int num_seg = mytune->read8();
		for (unsigned int j = 0; j < num_seg; j++) {
			mytune->index += 2;
			unsigned int num_bytes = mytune->read8();
			item->text = QString("%1: %2").arg(desc).arg(mytune->readstr(num_bytes));
			tree_create_wait(item);
		}
	}
	item->parent		= orig.parent;
	item->return_parent	= orig.return_parent;
}

void tuning_thread::parse_descriptor(tree_item *item)
{
	tree_item orig;
	orig.parent			= item->parent;
	orig.current		= item->current;
	orig.return_parent	= item->return_parent;
	item->parent		= item->current;

	unsigned int desc_tag = mytune->read8();
	unsigned int desc_len = mytune->read8();
	unsigned int desc_end = desc_len + mytune->index;

	item->text = QString("Descriptor: %1 - %2 Descriptor").arg(tohex(desc_tag,2)).arg(dvbnames.dvb_descriptortag[desc_tag]);
	item->return_parent = true;
	tree_create_wait(item);
	item->return_parent	= false;

	switch(desc_tag) {
	case 0x05: // registration_descriptor
		item->text = QString("Format Identifier: %1").arg(mytune->readstr(4));
		tree_create_wait(item);
		break;
	case 0x09: // CA Descriptor
	{
		unsigned int sys = mytune->read16();
		unsigned int pid = mytune->read16(0x1FFF);
		item->pid	= pid;
		item->color	= QColor(Qt::red);
		item->text	= QString("CA PID: %1 - system_id: %2 (%3)").arg(tohex(pid,4)).arg(tohex(sys,4)).arg(dvbnames.ca_name[sys]);
		tree_create_wait(item);
	}
		break;
	case 0x0A: // ISO_639_language_descriptor
		while (mytune->index < desc_end) {
			item->text = QString("Language: %1").arg(mytune->readstr(3));
			tree_create_wait(item);
			mytune->index++;
		}
		break;
	case 0x40: // network_name_descriptor
		item->text = QString("Network Name: %1").arg(mytune->readstr(desc_len));
		tree_create_wait(item);
		break;
	case 0x41: // service_list_descriptor
		while(mytune->index < desc_end) {
			unsigned int sid = mytune->read16();
			unsigned int type = mytune->read8();
			item->text = QString("Service ID: %1 - %2").arg(sid).arg(dvbnames.stream_type[type]);
			tree_create_wait(item);
		}
		break;
	case 0x43: //satellite_delivery_system_descriptor
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

		item->text = QString("Frequency: %1 %2 %3 , fec = %4").arg(frequency/100).arg(dvbnames.dtag_polarization[polarization]).arg(symbol_rate/10).arg(dvbnames.dtag_fec[fec_inner]);
		tree_create_wait(item);

		item->text = QString("Modulation System: %1").arg(modulation_system ? "DVB-S2" : "DVB-S");
		tree_create_wait(item);

		item->text = QString("Modulation Type: %1").arg(dvbnames.dtag_modulation[modulation_type]);
		tree_create_wait(item);

		if (modulation_system) {
			item->text = QString("Roll Off: %1").arg(dvbnames.dtag_rolloff[roll_off]);
			tree_create_wait(item);
		}
		item->text = QString("Orbital Position: %L1%2").arg(QString::number(orbital_pos/10.0, 'f', 1)).arg(east_west_flag ? "e" : "w");
		tree_create_wait(item);
		break;
	case 0x48: // service_descriptor
		mytune->index += 1;
		mysdt.pname.append(mytune->readstr(mytune->read8()));
		mysdt.sname.append(mytune->readstr(mytune->read8()));
		if (mysdt.sname.last() != "") {
			item->text = QString("Service Name: %1").arg(mysdt.sname.last());
			tree_create_wait(item);
		}
		if (mysdt.pname.last() != "") {
			item->text = QString("Provider Name: %1").arg(mysdt.pname.last());
			tree_create_wait(item);
		}
		break;
	case 0x4d: // short_event_descriptor
		item->text = QString("Language: %1").arg(mytune->readstr(3));
		tree_create_wait(item);
		item->text = QString("Event Name: %1").arg(mytune->readstr(mytune->read8()));
		tree_create_wait(item);
		item->text = QString("Text: %1").arg(mytune->readstr(mytune->read8()));
		tree_create_wait(item);
		break;
	case 0x81: // AC-3_audio_stream_descriptor
	{
		ac3_desc ac3;
		unsigned int tmp;
		tmp = mytune->read8();
		item->text = QString("Sample Rate: %1").arg(ac3.sample_rate_code.at(mytune->maskbits(tmp,0xE0)));
		tree_create_wait(item);
		item->text = QString("BSID: %1").arg(mytune->maskbits(tmp,0x1F));
		tree_create_wait(item);
		tmp = mytune->read8();
		item->text = QString("Bit Rate: %1 %2").arg(ac3.bit_rate_code.at(mytune->maskbits(tmp,0x7C))).arg(mytune->maskbits(tmp,0x80) ? "Upper Limit" : "Exact Rate");
		tree_create_wait(item);
		item->text = QString("Surround Mode: %1").arg(ac3.dsurmod.at(mytune->maskbits(tmp,0xC0)));
		tree_create_wait(item);
		tmp = mytune->read8();
		unsigned int bsmod = mytune->maskbits(tmp,0x38);
		item->text = QString("Bit Stream Mode: %1").arg(ac3.bsmode.at(bsmod));
		tree_create_wait(item);
		unsigned int num_channels = mytune->maskbits(tmp,0x1E);
		item->text = QString("Num Channels: %1").arg(ac3.num_channels.at(num_channels));
		tree_create_wait(item);
		item->text = QString("Full Service: %1").arg(mytune->maskbits(tmp,0x01) ? "Yes" : "No");
		tree_create_wait(item);
	}
		break;
	case 0xA0: // extended_channel_name_descriptor
		while (mytune->index < desc_end) {
			int number_strings = mytune->read8();
			for (int i = 0; i < number_strings; i++) {
				mytune->index += 3;
				int number_segments = mytune->read8();
				for (int a = 0; a < number_segments; a++) {
					mytune->index += 2;
					item->text = QString("Long Channel Name: %1").arg(mytune->readstr(mytune->read8()));
					tree_create_wait(item);
				}
			}
		}
		break;
	case 0xA1: // service_location_descriptor
	{
		item->text	= QString("PCR: %1").arg(tohex(mytune->read16(0x1FFF),4));
		tree_create_wait(item);
		unsigned int number_elements = mytune->read8();
		for (unsigned int i = 0; i < number_elements; i++) {
			unsigned int stream_type = mytune->read8();
			item->text	= QString("Stream PID: %1, Type: %2 - %3").arg(tohex(mytune->read16(0x1FFF),4)).arg(tohex(stream_type,2)).arg(dvbnames.stream_type[stream_type]);
			tree_create_wait(item);
			item->text = QString("Language: %1").arg(mytune->readstr(3));
			tree_create_wait(item);
		}
	}
		break;
	case 0xA3: // component_name_descriptor
		parse_etm(item, "Component Name");
		break;
	default:
		qDebug() << Q_FUNC_INFO << "Unknown Descriptor" << tohex(desc_tag,2);
		break;
	}
	item->parent		= orig.parent;
	item->current		= orig.current;
	item->return_parent	= orig.return_parent;
	mytune->index = desc_end;
}

void tuning_thread::parse_psip_tvct()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return;
	}
	mytune->packet_processed.append(mytune->buffer);

	unsigned int pid = mytune->dvbdata.first().pid;
	tree_item *item = new tree_item;
	item->pid		= pid;
	item->text		= QString("PSIP pid: %1").arg(tohex(pid,4));
	tree_create_wait(item);

	item->text = QString("TVCT - Terrestrial Virtual Channel Table");
	tree_create_wait(item);
	item->pid			= 0xFFFF;
	item->return_parent = false;

	mytune->index += 8;

	unsigned int num_channels_in_section = mytune->read8();
	for (unsigned int i = 0; i < num_channels_in_section; i++) {
		item->text = QString("Short Name: %1").arg(mytune->readstr16(7));
		tree_create_wait(item);

		unsigned int channel = mytune->read24();
		item->text = QString("Channel Number: %1-%2").arg(mytune->maskbits(channel, 0xFFC00)).arg(mytune->maskbits(channel, 0x3FF));
		tree_create_wait(item);

		mytune->index += 13;
		unsigned int descriptors_length = mytune->index + mytune->read16(0x3FF);
		while (mytune->index < descriptors_length) {
			parse_descriptor(item);
		}
	}
	unsigned int additional_descriptors_length = mytune->index + mytune->read16(0x3FF);
	while (mytune->index < additional_descriptors_length) {
		parse_descriptor(item);
	}
}

void tuning_thread::parse_psip_eit()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return;
	}
	mytune->packet_processed.append(mytune->buffer);

	unsigned int pid = mytune->dvbdata.first().pid;
	tree_item *item = new tree_item;
	item->pid		= pid;
	item->text		= QString("PSIP pid: %1").arg(tohex(pid,4));
	tree_create_wait(item);

	item->text = QString("EIT - Event Information Table");
	tree_create_wait(item);
	item->pid		= 0xFFFF;

	mytune->index += 2;

	item->text = QString("SourceID: %1").arg(tohex(mytune->read16(),2));
	tree_create_wait(item);

	mytune->index += 4;

	unsigned int num_channels_in_section = mytune->read8();
	for (unsigned int i = 0; i < num_channels_in_section; i++) {
		tree_item orig = *item;

		item->return_parent = true;
		item->text = QString("Event ID: %1").arg(tohex(mytune->read16(0x3FFF),4));
		tree_create_wait(item);
		item->return_parent = false;

		item->text = QString("Start Time: %1").arg(QDateTime::fromTime_t(mytune->read32() + 315964800).toString());
		tree_create_wait(item);

		__u32 dtime = mytune->read24(0x0FFF);
		unsigned int h = dtime/60/60;
		unsigned int m = dtime/60 - (h*60);
		unsigned int s = dtime - (h*60*60) - (m*60);
		item->text = QString("Duration: %1").arg(QTime(h, m, s).toString());
		tree_create_wait(item);

		mytune->index++;

		parse_etm(item, "Event Name");

		unsigned int descriptors_length = mytune->index + mytune->read16(0xFFF);
		while (mytune->index < descriptors_length) {
			parse_descriptor(item);
		}

		*item = orig;
	}
}

void tuning_thread::parse_psip_stt()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return;
	}
	mytune->packet_processed.append(mytune->buffer);

	unsigned int pid = mytune->dvbdata.first().pid;
	tree_item *item = new tree_item;
	item->pid		= pid;
	item->text		= QString("PSIP pid: %1").arg(tohex(pid,4));
	tree_create_wait(item);

	item->text			= QString("STT - System Time Table");
	item->expanded		= false;
	tree_create_wait(item);
	item->pid			= 0xFFFF;
	item->return_parent = false;

	unsigned int section_length = mytune->read16(0x0FFF) + mytune->index - 4;
	mytune->index += 6;

	item->text = QString("System Time: %1").arg(QDateTime::fromTime_t(mytune->read32() + 315964800).toString());
	tree_create_wait(item);

	mytune->index += 3;

	while (mytune->index < section_length) {
		parse_descriptor(item);
	}
}

void tuning_thread::parse_psip_mgt()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return;
	}
	mytune->packet_processed.append(mytune->buffer);

	unsigned int pid = mytune->dvbdata.first().pid;
	tree_item *item = new tree_item;
	item->pid		= pid;
	item->text		= QString("PSIP pid: %1").arg(tohex(pid,4));
	tree_create_wait(item);

	item->text = QString("MGT - Master Guide Table");
	tree_create_wait(item);
	item->pid			= 0xFFFF;
	item->return_parent = false;

	mytune->index += 4;

	item->text = QString("Version Number: %1 ").arg(tohex(mytune->read8(0x3E),2));
	tree_create_wait(item);

	mytune->index += 2;

	item->text = QString("Protocol Version: %1 ").arg(tohex(mytune->read8(),2));
	tree_create_wait(item);

	unsigned int num_tables = mytune->read16();
	for (unsigned int i = 0; i < num_tables; i++) {
		unsigned int table_type	= mytune->read16();
		unsigned int table_pid	= mytune->read16(0x1FFF);

		mytune->index += 5;

		item->text = QString("Table Type: %1 - PID: %2").arg(tohex(table_type,4)).arg(tohex(table_pid,4));
		tree_create_wait(item);

		if (table_type >= 0x0100 && table_type <= 0x17F) {
			filter_pids(table_pid, 0xCB);
		} else if ((table_type >= 0x0200 && table_type <= 0x27F) || table_type == 0x0004) {
			filter_pids(table_pid, 0xCC);
		} else if ((table_type >= 0x0301 && table_type <= 0x3FF)) {
			filter_pids(table_pid, 0xCA);
		} else if (table_type == 0x0000) {
			filter_pids(table_pid, 0xC8);
		} else {
			qDebug() << Q_FUNC_INFO << "Unknown PID:" << tohex(table_pid,4) << "& table_type:" << tohex(table_type,4);
		}

		unsigned int desc_len = mytune->read16(0x0FFF);
		for (unsigned int a = 0; a < desc_len; a++) {
			parse_descriptor(item);
		}
	}
	unsigned int desc_len = mytune->read16(0x0FFF);
	for (unsigned int a = 0; a < desc_len; a++) {
		parse_descriptor(item);
	}
}

void tuning_thread::parse_psip_ett()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return;
	}
	mytune->packet_processed.append(mytune->buffer);

	unsigned int pid = mytune->dvbdata.first().pid;
	tree_item *item = new tree_item;
	item->pid		= pid;
	item->text		= QString("PSIP pid: %1").arg(tohex(pid,4));
	tree_create_wait(item);

	item->text = QString("ETT - Extended Text Table");
	tree_create_wait(item);
	item->pid			= 0xFFFF;
	item->return_parent = false;

	mytune->index += 8;

	item->text = QString("ETM ID : %2").arg(tohex(mytune->read32(),8));
	tree_create_wait(item);

	parse_etm(item, "Text");
}

void tuning_thread::parse_psip_rrt()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return;
	}
	mytune->packet_processed.append(mytune->buffer);

	unsigned int pid = mytune->dvbdata.first().pid;
	tree_item *item = new tree_item;
	item->pid		= pid;
	item->text		= QString("PSIP pid: %1").arg(tohex(pid,4));
	tree_create_wait(item);

	item->text = QString("RRT - Rating Region Table");
	tree_create_wait(item);
	item->pid			= 0xFFFF;
	item->return_parent = false;

	mytune->index += 9;

	parse_etm(item, "Rating Region Name");

	unsigned int dimensions_defined = mytune->read8();
	for (unsigned int i = 0; i < dimensions_defined; i++) {
		mytune->index++;
		parse_etm(item, "Dimension Name");

		unsigned int values_defined = mytune->read8(0x0F);
		for (unsigned int j = 0; j < values_defined; j++) {
			mytune->index++;
			parse_etm(item, "Abbreviated Rating Name");
			mytune->index++;
			parse_etm(item, "Rating Name");
		}
	}

	unsigned int descriptors_loop_length = mytune->read16(0x03FF) + mytune->index;
	while (mytune->index < descriptors_loop_length) {
		parse_descriptor(item);
	}
}

void tuning_thread::parse_eit()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return;
	}
	mytune->packet_processed.append(mytune->buffer);

	unsigned int pid = mytune->dvbdata.first().pid;
	tree_item *item = new tree_item;
	item->pid		= pid;
	item->text		= QString("EIT pid: %1").arg(tohex(pid,4));
	tree_create_wait(item);
	item->pid			= 0xFFFF;
	item->return_parent = false;

	unsigned int section_length = mytune->read16(0x0FFF) + mytune->index - 4;

	item->text = QString("Program Number: %1").arg(mytune->read16());
	tree_create_wait(item);
	item->return_parent = false;

	mytune->index += 9;

	while (mytune->index < section_length) {
		item->text = QString("Event ID: %1").arg(mytune->read16());
		tree_create_wait(item);

		__u16 t1 = mytune->read16();
		__u8  t2 = mytune->read8();
		__u8  t3 = mytune->read8();
		__u8  t4 = mytune->read8();
		__u8  t5 = mytune->read8();
		__u8  t6 = mytune->read8();
		__u8  t7 = mytune->read8();
		item->text = QString("Start Date/Time: %1 %2:%3:%4").arg(QDate::fromJulianDay(t1 + 2400000.5).toString()).arg(dtag_convert(t2), 2, 10, QChar('0')).arg(dtag_convert(t3), 2, 10, QChar('0')).arg(dtag_convert(t4), 2, 10, QChar('0'));
		tree_create_wait(item);
		item->text = QString("Duration: %1:%2:%3").arg(dtag_convert(t5), 2, 10, QChar('0')).arg(dtag_convert(t6), 2, 10, QChar('0')).arg(dtag_convert(t7), 2, 10, QChar('0'));
		tree_create_wait(item);

		unsigned int descriptors_loop_length = mytune->read16(0x0FFF) + mytune->index;
		while (mytune->index < descriptors_loop_length) {
			parse_descriptor(item);
		}
	}
}

void tuning_thread::parse_sdt()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return;
	}
	mytune->packet_processed.append(mytune->buffer);

	unsigned int pid = mytune->dvbdata.first().pid;
	tree_item *item = new tree_item;
	item->pid		= pid;
	item->text		= QString("SDT pid: %1").arg(tohex(pid,4));
	tree_create_wait(item);
	item->pid			= 0xFFFF;
	item->return_parent = false;

	mysdt.sid.clear();
	mysdt.sname.clear();
	mysdt.pname.clear();

	unsigned int section_length = mytune->read16(0x0FFF) + mytune->index - 4;

	mytune->index += 8;

	while (mytune->index < section_length) {
		mysdt.sid.append(mytune->read16());

		item->text = QString("Service ID: %1").arg(mysdt.sid.last());
		tree_create_wait(item);

		mytune->index += 1;

		unsigned int descriptors_loop_length = mytune->read16(0x0FFF) + mytune->index;
		while (mytune->index < descriptors_loop_length) {
			parse_descriptor(item);
		}
	}
}

void tuning_thread::parse_pat()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return;
	}
	mytune->packet_processed.append(mytune->buffer);

	unsigned int pid = mytune->dvbdata.first().pid;
	tree_item *item = new tree_item;
	item->pid		= pid;
	item->text		= QString("PAT pid: %1").arg(tohex(pid,4));
	tree_create_wait(item);
	item->pid			= 0xFFFF;

	mypat.number.clear();
	mypat.pid.clear();

	unsigned int section_length = mytune->read16(0x0FFF) - 4;

	mytune->index += 5;

	while (mytune->index < section_length) {
		tree_item orig = *item;

		mypat.number.append(mytune->read16());
		mypat.pid.append(mytune->read16(0x1FFF));
		item->text = QString("Program Number: %1").arg(mypat.number.last());
		tree_create_wait(item);
		item->text = QString("Program PID: %1").arg(tohex(mypat.pid.last(),4));
		tree_create_wait(item);

		*item = orig;
	}
}

void tuning_thread::parse_cat()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return;
	}
	mytune->packet_processed.append(mytune->buffer);

	unsigned int pid = mytune->dvbdata.first().pid;
	tree_item *item = new tree_item;
	item->pid		= pid;
	item->color		= QColor(Qt::red);
	item->text		= QString("CAT pid: %1").arg(tohex(pid,4));
	tree_create_wait(item);
	item->pid			= 0xFFFF;
	item->return_parent = false;

	unsigned int section_length = mytune->read16(0x0FFF) - 4;

	mytune->index += 5;

	while (mytune->index < section_length) {
		parse_descriptor(item);
	}
}

void tuning_thread::parse_nit()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return;
	}
	mytune->packet_processed.append(mytune->buffer);

	unsigned int pid = mytune->dvbdata.first().pid;
	tree_item *item = new tree_item;
	item->pid		= pid;
	item->text		= QString("NIT pid: %1").arg(tohex(pid,4));
	tree_create_wait(item);
	item->pid		= 0xFFFF;

	unsigned int section_length = mytune->read16(0x0FFF);

	item->text = QString("Network ID: %1").arg(tohex(mytune->read16(),4));
	tree_create_wait(item);
	item->return_parent = false;

	mytune->index += 3;

	unsigned int network_desc_length = mytune->read16(0x0FFF) + mytune->index;
	while (mytune->index < network_desc_length && mytune->index < section_length) {
		parse_descriptor(item);
	}

	unsigned int transport_stream_loop_length = mytune->read16(0x0FFF) + mytune->index;
	while (mytune->index < transport_stream_loop_length) {
		item->text = QString("Transport Stream ID: %1, Original Network ID: %2").arg(tohex(mytune->read16(),4)).arg(tohex(mytune->read16(),4));
		tree_create_wait(item);

		unsigned int transport_desc_length = mytune->read16(0x0FFF) + mytune->index;
		while (mytune->index < transport_desc_length && mytune->index < section_length) {
			parse_descriptor(item);
		}
	}
}

void tuning_thread::parse_pmt()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return;
	}
	mytune->packet_processed.append(mytune->buffer);

	unsigned int pid = mytune->dvbdata.first().pid;
	tree_item *item = new tree_item;
	item->pid		= pid;

	unsigned int section_length = mytune->read16(0x0FFF) - 4;

	item->text = QString("PMT PID: %1 - Program: %2").arg(tohex(pid,4)).arg(mytune->read16());
	tree_create_wait(item);
	item->return_parent = false;

	mytune->index += 3;

	item->pid	= mytune->read16(0x1FFF);
	item->text	= QString("PCR: %1").arg(tohex(item->pid,4));
	tree_create_wait(item);
	item->pid	= 0xFFFF;

	unsigned int program_info_length = mytune->read16(0x0FFF) + mytune->index;
	while (mytune->index < program_info_length) {
		parse_descriptor(item);
	}

	while (mytune->index < section_length) {
		unsigned int desc_type	= mytune->read8();
		unsigned int desc_pid	= mytune->read16(0x1FFF);

		item->pid	= desc_pid;
		item->text	= QString("Stream PID: %1, Type: %2 - %3").arg(tohex(desc_pid,4)).arg(tohex(desc_type,2)).arg(dvbnames.stream_type[desc_type]);
		tree_create_wait(item);
		item->pid	= 0xFFFF;

		unsigned int ES_info_length = mytune->read16(0x0FFF) + mytune->index;
		while (mytune->index < ES_info_length) {
			parse_descriptor(item);
		}
	}
}

void tuning_thread::parse_tdt()
{
	if (mytune->packet_processed.contains(mytune->buffer)) {
		return;
	}
	mytune->packet_processed.append(mytune->buffer);

	unsigned int pid = mytune->dvbdata.first().pid;
	tree_item *item = new tree_item;
	item->pid		= pid;
	item->expanded	= false;
	item->text	= QString("TDT pid: %1").arg(tohex(pid,4));
	tree_create_wait(item);
	item->pid			= 0xFFFF;
	item->return_parent = false;

	mytune->index += 2;

	__u16 t1 = mytune->read16();
	__u8  t2 = mytune->read8();
	__u8  t3 = mytune->read8();
	__u8  t4 = mytune->read8();

	// If you change this text, change it in tuning::tree_create_child() as well
	item->text = QString("UTC Date/Time: %1 %2:%3:%4").arg(QDate::fromJulianDay(t1 + 2400000.5).toString()).arg(tohex(dtag_convert(t2),2)).arg(tohex(dtag_convert(t3),2)).arg(tohex(dtag_convert(t4),2));
	tree_create_wait(item);
}

void tuning_thread::parse_dcii_sdt()
{
	return; // I need to tune a DCII tp to test this before adding it again

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
				qDebug() << Q_FUNC_INFO << "Unknown TableID:" << hex << mytune->dvbdata.first().pid << mytune->dvbdata.first().table;
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
