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
}

tuning_thread::~tuning_thread()
{
	qDebug() << "~tuning_thread()";
	qDebug() << "~tuning_thread() done";
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
	if (!loop) {
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

int tuning_thread::parse_descriptor(int parent)
{
	int desc_tag = mytune->read8();
	int desc_len = mytune->read8();
	int desc_end = desc_len + mytune->index;
	if (dvbnames.dvb_descriptortag.at(desc_tag) == "") {
		tree_create_child_wait(&parent, QString("Unknown descriptor: 0x%1").arg(desc_tag,2,16,QChar('0')));
		qDebug() << "Unkown descriptor:" << desc_tag;
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
		tree_create_child_wait(&parent, QString("Network Name: %1").arg(mytune->readstr(mytune->index, desc_len)));
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
		mysdt.pname.append(mytune->readstr(mytune->index, mytune->read8()));
		mysdt.sname.append(mytune->readstr(mytune->index, mytune->read8()));
		if (mysdt.sname.last() != "") {
			parent_t = parent;
			tree_create_child_wait(&parent_t, QString("Service Name: %1").arg(mysdt.sname.last()));
		}
		if (mysdt.pname.last() != "") {
			parent_t = parent;
			tree_create_child_wait(&parent_t, QString("Provider Name: %1").arg(mysdt.pname.last()));
		}
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
					tree_create_child_wait(&parent_t, QString("Long Channel Name: %1").arg(mytune->readstr(mytune->index, mytune->read8())));
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

int tuning_thread::parse_psip()
{
	qDebug() << "parse_psip()";
	if (mytune->read8() != 0xC8) {
		return 0;
	}

	int parent, parent_t;
	tree_create_root_wait(&parent, "PSIP(TVCT) pid: 0x1FFB", 0x1FFB);

	int section_length = mytune->read16(0x0FFF) + mytune->index - 4;
	Q_UNUSED(section_length);
	
	mytune->index += 6;

	int num_channels_in_section = mytune->read8();
	for (int i = 0; i < num_channels_in_section; i++) {
		QString short_name;
		for (int a = 0; a < 7; a++) {
			mytune->index += 1;
			short_name.append(mytune->read8());
		}
		parent_t = parent;
		tree_create_child_wait(&parent_t, QString("Name: %1").arg(short_name));
		parent_t = parent;
		int channel = mytune->read24();
		tree_create_child_wait(&parent_t, QString("Channel Number: %1-%2").arg(mytune->maskbits(channel, 0xFFC00)).arg(mytune->maskbits(channel, 0x3FF)));

		mytune->index += 13;
		int descriptors_length = mytune->index + mytune->read16(0x3FF);
		while (mytune->index < descriptors_length) {
			parent_t = parent;
			parse_descriptor(parent_t);
		}
	}
	int additional_descriptors_length = mytune->index + mytune->read16(0x3FF);
	while (mytune->index < additional_descriptors_length) {
		parent_t = parent;
		parse_descriptor(parent_t);
	}
	return 1;
}

int tuning_thread::parse_sdt()
{
	mysdt.sid.clear();
	mysdt.sname.clear();
	mysdt.pname.clear();

	if (mytune->read8() != 0x42) {
		return -1;
	}

	int parent, parent_t;

	tree_create_root_wait(&parent, "SDT pid: 0x11", 0x11);
	emit setcolor(parent, QColor(Qt::green).darker(300));
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
	mypat.number.clear();
	mypat.pid.clear();
	mytune->index = 0;
	if (mytune->read8() != 0x00) {
		return -1;
	}

	int section_length = mytune->read16(0x0FFF) - 4;
	mytune->index = 8;
	do {
		mypat.number.append(mytune->read16());
		mypat.pid.append(mytune->read16(0x1FFF));
	} while (mytune->index < section_length);
	return 1;
}

int tuning_thread::parse_cat()
{
	mytune->index = 0;
	if (mytune->read8() != 0x01) {
		return -1;
	}

	int parent;
	tree_create_root_wait(&parent, "CAT pid: 0x01", 0x01);

	int section_length = mytune->read16(0x0FFF) - 4;
	mytune->index += 5;
	while (mytune->index < section_length) {
		int parent_t = parent;
		parse_descriptor(parent_t);
	}
	return 1;
}

void tuning_thread::parsetp()
{
	int parent_1 = 0;
	int parent_2 = 0;
	int parent_t = 0;

	mytune->get_bitrate();

	if (mytune->tp.system == SYS_ATSC || mytune->tp.system == SYS_ATSCMH) {
		for (int i = 0; i < 5; i++) {
			if (!loop) return;
			if (mytune->demux_packet(0x1FFB, 0xC8) > 0) {
				if (!loop) return;
				if (parse_psip()) {
					i = 5;
				}
			}
		}
	}

	if (!loop) return;
	if (mytune->pids_rate[0x11] && mytune->demux_packet(0x11, 0x42) > 0) {
		if (!loop) return;
		parse_sdt();
	}

	if (!loop) return;
	if (mytune->pids_rate[0x00] && mytune->demux_packet(0x00, 0x00) > 0) {
		if (!loop) return;
		parse_pat();
	}

	if (!loop) return;
	if (mytune->pids_rate[0x01] && mytune->demux_packet(0x01, 0x01) > 0) {
		if (!loop) return;
		parse_cat();
	}

	if (!loop) return;
	if (mytune->pids_rate[0x10] && mytune->demux_packet(0x10, 0x40) > 0) {
		if (!loop) return;
		mytune->index = 0;
		if (mytune->read8() == 0x40) {
			int section_length = mytune->read16(0x0FFF);

			tree_create_root_wait(&parent_1, "NIT pid: 0x10", 0x10);
			emit setcolor(parent_1, QColor(Qt::green).darker(300));
			parent_t = parent_1;
			tree_create_child_wait(&parent_t, QString("Network ID: 0x%1").arg(mytune->read16(),4,16,QChar('0')));

			mytune->index += 3;
			int network_desc_length = mytune->read16(0x0FFF) + mytune->index;
			while (mytune->index < network_desc_length && mytune->index < section_length) {
				parse_descriptor(parent_1);
			}

			int transport_stream_loop_length = mytune->read16(0x0FFF) + mytune->index;
			while (mytune->index < transport_stream_loop_length) {
				int transport_stream_id = mytune->read16();
				int original_network_id = mytune->read16();
				int transport_desc_length = mytune->read16(0x0FFF) + mytune->index;

				parent_2 = parent_1;
				tree_create_child_wait(&parent_2, QString("Transport Stream ID: 0x%1, Original Network ID: 0x%2").arg(transport_stream_id,4,16,QChar('0')).arg(original_network_id,4,16,QChar('0')));
				while (mytune->index < transport_desc_length && mytune->index < section_length) {
					parse_descriptor(parent_2);
				}
			}
		}
	}

	for(int i = 0; i < mypat.number.size(); i++) {
		tree_create_root_wait(&parent_1, QString("PMT PID: 0x%1 - Program: %2").arg(mypat.pid[i],4,16,QChar('0')).arg(mypat.number[i]), mypat.pid[i]);
		emit setcolor(parent_1, QColor(Qt::green).darker(300));

		if (mytune->tp.system == SYS_DCII) {
			if (!loop) return;
			if (mytune->pids_rate[mypat.pid[i]] && mytune->demux_packet(mypat.pid[i], 0xC1) > 0) {
				parent_2 = parent_1;
				tree_create_child_wait(&parent_2, "SDT");
				mytune->index += 15;
				parent_t = parent_2;
				tree_create_child_wait(&parent_t, QString("Service Name: %1").arg(mytune->readstr(mytune->index, mytune->read8())));
			}
		}

		if (!loop) return;
		if (mytune->pids_rate[mypat.pid[i]] && mytune->demux_packet(mypat.pid[i], 0x02, 500) <= 0) {
			continue;
		}
		if (!loop) return;

		if (mysdt.sid.indexOf(mypat.number[i]) != -1) {
			parent_2 = parent_1;
			tree_create_child_wait(&parent_2, "SDT");
			if (mysdt.sname[mysdt.sid.indexOf(mypat.number[i])] != "") {
				parent_t = parent_2;
				tree_create_child_wait(&parent_t, QString("Service Name: %1").arg(mysdt.sname[mysdt.sid.indexOf(mypat.number[i])]));
			}
			if (mysdt.pname[mysdt.sid.indexOf(mypat.number[i])] != "") {
				parent_t = parent_2;
				tree_create_child_wait(&parent_t, QString("Provider Name: %1").arg(mysdt.pname[mysdt.sid.indexOf(mypat.number[i])]));
			}
		}

		mytune->index += 1;
		int section_length = mytune->read16(0x0FFF) + mytune->index - 4;
		mytune->index += 5;

		int pcr_pid = mytune->read16(0x1FFF);
		parent_t = parent_1;
		tree_create_child_wait(&parent_t, QString("PCR: 0x%1").arg(pcr_pid,4,16,QChar('0')), pcr_pid);

		int program_info_length = mytune->read16(0x0FFF) + mytune->index;
		while (mytune->index < program_info_length) {
			parse_descriptor(parent_1);
		}

		while (mytune->index < section_length) {
			int pmt_type = mytune->read8();
			int pmt_pid = mytune->read16(0x1FFF);

			parent_2 = parent_1;
			tree_create_child_wait(&parent_2, QString("Stream PID: 0x%1, 0x%2 - %3").arg(pmt_pid,4,16,QChar('0')).arg(pmt_type,2,16,QChar('0')).arg(dvbnames.stream_type[pmt_type]), pmt_pid);

			int ES_info_length = mytune->read16(0x0FFF) + mytune->index;
			while (mytune->index < ES_info_length) {
				parse_descriptor(parent_2);
			}
		}
	}
	for (int i = 0; i <= 0xFFFF; i++) {
		if (mytune->pids_rate[i] > 0) {
			emit list_create(QString("0x%1 - %2 kbit/s").arg(i,4,16,QChar('0')).arg(mytune->pids_rate[i],5,10,QChar(' ')), i);
		}
	}

	mytune->close_dvr();
	qDebug() << "parsetp() done";
}

void tuning_thread::run()
{
	loop = true;
	do {
		if (thread_function.indexOf("parsetp") != -1) {
			parsetp();
			loop = false;
		}
		msleep(10);
	} while (loop);
	thread_function.clear();
}
