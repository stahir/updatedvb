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

#include "dvb_settings.h"

tp_info::tp_info()
{
	frequency	= 950;
	voltage		= SEC_VOLTAGE_18;
	symbolrate	= 1000;
	fec			= FEC_AUTO;
	system		= SYS_DVBS;
	modulation	= QPSK;
	inversion	= INVERSION_AUTO;
	rolloff		= ROLLOFF_AUTO;
	pilot		= PILOT_AUTO;
}

tuning_options::tuning_options()
{
	f_start	= 3700;
	f_stop	= 4200;
	f_lof	= -5150;
	voltage	= SEC_VOLTAGE_18;
	tone	= SEC_TONE_OFF;
	mis		= -1;
	diseqctype = 3;
	diseqcport = -1;
}

dvb_settings::dvb_settings()
{
	fec.append("None");
	fec.append("1/2");
	fec.append("2/3");
	fec.append("3/4");
	fec.append("4/5");
	fec.append("5/6");
	fec.append("6/7");
	fec.append("7/8");
	fec.append("8/9");
	fec.append("Auto");
	fec.append("3/5");
	fec.append("9/10");
	fec.append("2/5");
	fec.append("5/11");

	system.append("Undefined");
	system.append("DVB-C A");
	system.append("DVB-C B");
	system.append("DVB-T");
	system.append("DSS");
	system.append("DVB-S");
	system.append("DVB-S2");
	system.append("DVB-H");
	system.append("ISDB-T");
	system.append("ISDB-S");
	system.append("ISDB-C");
	system.append("ATSC");
	system.append("ATSC-MH");
	system.append("DMB-TH");
	system.append("CMMB");
	system.append("DAB");
	system.append("DVB-T2");
	system.append("TURBO");
	system.append("DVB-C C");
	system.append("DCII");

	modulation.append("QPSK");
	modulation.append("QAM 16");
	modulation.append("QAM 32");
	modulation.append("QAM 64");
	modulation.append("QAM 128");
	modulation.append("QAM 256");
	modulation.append("QAM Auto");
	modulation.append("VSB 8");
	modulation.append("VSB 16");
	modulation.append("8PSK");
	modulation.append("16PSK");
	modulation.append("32PSK");
	modulation.append("DQPSK");
	modulation.append("QAM 4NR");
	modulation.append("C QPSK");
	modulation.append("I QPSK");
	modulation.append("Q QPSK");
	modulation.append("C OQPSK");

	dtag_modulation.fill("Unknown", 0xFF + 1);
	dtag_modulation[0] = "Auto";
	dtag_modulation[1] = "QPSK";
	dtag_modulation[2] = "8PSK";
	dtag_modulation[3] = "16QAM";

	dtag_rolloff.fill("Unknown", 0xFF + 1);
	dtag_rolloff[0] = "0.35";
	dtag_rolloff[1] = "0.25";
	dtag_rolloff[2] = "0.20";
	dtag_rolloff[3] = "Reserved";

	dtag_fec.fill("Unknown", 0xFF + 1);
	dtag_fec[0] = "Not Defined";
	dtag_fec[1] = "1/2";
	dtag_fec[2] = "2/3";
	dtag_fec[3] = "3/4";
	dtag_fec[4] = "5/6";
	dtag_fec[5] = "7/8";
	dtag_fec[6] = "8/9";
	dtag_fec[7] = "3/5";
	dtag_fec[8] = "4/5";
	dtag_fec[9] = "9/10";
	dtag_fec[10] = "No Conventional Coding";

	dtag_polarization.fill("?", 0xFF + 1);
	dtag_polarization[0] = "H";
	dtag_polarization[1] = "V";
	dtag_polarization[2] = "L";
	dtag_polarization[3] = "R";

	voltage.append(" V ");
	voltage.append(" H ");
	voltage.append(" N ");

	tone.append("ON");
	tone.append("OFF");

	rolloff.append("35");
	rolloff.append("20");
	rolloff.append("25");
	rolloff.append("AUTO");

	pilot.append("OFF");
	pilot.append("ON");
	pilot.append("AUTO");

	inversion.append("OFF");
	inversion.append("ON");
	inversion.append("AUTO");

	stream_type.fill("", 0xFF + 1);
	stream_type[0x00] = "ITU-T | ISO-IE Reserved";
	stream_type[0x01] = "ISO/IEC 11172-2 Video";
	stream_type[0x02] = "ITU-T Rec. H.262 | ISO/IEC 13818-2 Video | ISO/IEC 11172-2 constr. parameter video stream";
	stream_type[0x03] = "ISO/IEC 11172 Audio";
	stream_type[0x04] = "ISO/IEC 13818-3 Audio";
	stream_type[0x05] = "ITU-T Rec. H.222.0 | ISO/IEC 13818-1 private sections";
	stream_type[0x06] = "ITU-T Rec. H.222.0 | ISO/IEC 13818-1 PES packets containing private data";
	stream_type[0x0C] = "Data Broadcast Service";
	stream_type[0x0D] = "ISO/IEC 13818-6 Sections (any type, including private data)";
	stream_type[0x0F] = "ISO/IEC 13818-7 Audio with ADTS transport syntax";
	stream_type[0x1B] = "AVC video stream as defined in ITU-T Rec. H.264";
	stream_type[0x56] = "ITU-T Rec. H.222.0 | ISO/IEC 13818-1 reserved";
	stream_type[0x80] = "User private";
	stream_type[0x81] = "User private | AC3 Audio";
	stream_type[0x82] = "User private";
	stream_type[0x83] = "User private";
	stream_type[0x84] = "User private";
	stream_type[0x85] = "User private";
	stream_type[0x86] = "User private";
	stream_type[0x87] = "User private";
	stream_type[0x88] = "User private";
	stream_type[0x89] = "User private";
	stream_type[0xC0] = "User private";
	stream_type[0xC1] = "User private";
	stream_type[0xC2] = "User private";
	stream_type[0xE0] = "User private";

	table_name.fill("", 0xFF + 1);
	table_name[0x00] = "PAT";
	table_name[0x01] = "CA";
	table_name[0x02] = "PMT";
	table_name[0x03] = "TS Desc";
	table_name[0x40] = "NIT";
	table_name[0x41] = "NIT";
	table_name[0x46] = "SDT";
	table_name[0x4A] = "BAT";
	table_name[0x4E] = "EIT";
	table_name[0x4F] = "EIT";
	table_name[0x50] = "EIT";
	table_name[0x51] = "EIT";
	table_name[0x52] = "EIT";
	table_name[0x53] = "EIT";
	table_name[0x54] = "EIT";
	table_name[0x55] = "EIT";
	table_name[0x56] = "EIT";
	table_name[0x57] = "EIT";
	table_name[0x58] = "EIT";
	table_name[0x59] = "EIT";
	table_name[0x5A] = "EIT";
	table_name[0x5B] = "EIT";
	table_name[0x5C] = "EIT";
	table_name[0x5D] = "EIT";
	table_name[0x5E] = "EIT";
	table_name[0x5F] = "EIT";
	table_name[0x60] = "EIT";
	table_name[0x61] = "EIT";
	table_name[0x62] = "EIT";
	table_name[0x63] = "EIT";
	table_name[0x64] = "EIT";
	table_name[0x65] = "EIT";
	table_name[0x66] = "EIT";
	table_name[0x67] = "EIT";
	table_name[0x68] = "EIT";
	table_name[0x69] = "EIT";
	table_name[0x6A] = "EIT";
	table_name[0x6B] = "EIT";
	table_name[0x6C] = "EIT";
	table_name[0x6D] = "EIT";
	table_name[0x6E] = "EIT";
	table_name[0x6F] = "EIT";
	table_name[0x70] = "TDT";

	ca_name.fill("", 0xFFFF + 1);
	ca_name[0x0001] = "Standardized Systems";
	ca_name[0x0500] = "Viaccess";
	ca_name[0x0602] = "Irdeto";
	ca_name[0x0604] = "Irdeto";
	ca_name[0x0606] = "Irdeto";
	ca_name[0x0608] = "Irdeto";
	ca_name[0x0622] = "Irdeto";
	ca_name[0x0626] = "Irdeto";
	ca_name[0x0635] = "Irdeto";
	ca_name[0x0664] = "Irdeto";
	ca_name[0x0B00] = "Conax CAS 5/7";
	ca_name[0x0D00] = "Cryptoworks";
	ca_name[0x0D02] = "Cryptoworks";
	ca_name[0x0D03] = "Cryptoworks";
	ca_name[0x0D05] = "Cryptoworks";
	ca_name[0x0D07] = "Cryptoworks";
	ca_name[0x0D20] = "Cryptoworks";
	ca_name[0x0E00] = "PowerVu";
	ca_name[0x1010] = "Tandberg Television";
	ca_name[0x1702] = "BetaCrypt 1";
	ca_name[0x1722] = "BetaCrypt 1";
	ca_name[0x1762] = "BetaCrypt 1";
	ca_name[0x1710] = "BetaCrypt 2";
	ca_name[0x1800] = "Nagravision";
	ca_name[0x1801] = "Nagravision";
	ca_name[0x1802] = "Nagravision";
	ca_name[0x1810] = "Nagravision";
	ca_name[0x1830] = "Nagravision";
	ca_name[0x2250] = "Scopus Network Technologies";
	ca_name[0x2251] = "Scopus Network Technologies";
	ca_name[0x2252] = "Scopus Network Technologies";
	ca_name[0x22F0] = "Codicrypt";
	ca_name[0x2600] = "BISS";
	ca_name[0x4900] = "China Crypt - Irdeto";
	ca_name[0x41EC] = "Alliance Broadcast Vision";
	ca_name[0x4749] = "General Instrument";
	ca_name[0x4800] = "Accessgate";
	ca_name[0x4A20] = "AlphaCrypt";
	ca_name[0x4AEB] = "Abel Quintic";
	ca_name[0x5604] = "Verimatrix";

	dvb_descriptortag.fill("", 0xFF + 1);
	dvb_descriptortag[0x00]	= "Reserved";
	dvb_descriptortag[0x02]	= "Video";
	dvb_descriptortag[0x03]	= "Audio";
	dvb_descriptortag[0x05]	= "Registration";
	dvb_descriptortag[0x06]	= "Data Stream Alignment";
	dvb_descriptortag[0x09]	= "CA";
	dvb_descriptortag[0x0C]	= "Multiplex Buffer Utilization";
	dvb_descriptortag[0x10]	= "Smoothing Buffer";
	dvb_descriptortag[0x0A]	= "ISO-639 language";
	dvb_descriptortag[0x0B]	= "System Clock";
	dvb_descriptortag[0x0E]	= "Maximum Bitrate";
	dvb_descriptortag[0x0F]	= "Private Data Indicator";
	dvb_descriptortag[0x11]	= "STD";
	dvb_descriptortag[0x1B]	= "H.264 Video";
	dvb_descriptortag[0x1C]	= "H.264 Audio";
	dvb_descriptortag[0x28]	= "AVC Video";
	dvb_descriptortag[0x2A]	= "AVC Timing and HRD";
	dvb_descriptortag[0x2B]	= "AAC Audio";
	dvb_descriptortag[0x40] = "Network Name";
	dvb_descriptortag[0x41] = "Service List";
	dvb_descriptortag[0x42] = "Stuffing";
	dvb_descriptortag[0x43] = "Satellite Delivery System";
	dvb_descriptortag[0x45]	= "VBI data";
	dvb_descriptortag[0x48]	= "Service";
	dvb_descriptortag[0x49]	= "Country Availibility";
	dvb_descriptortag[0x4A]	= "Linkage";
	dvb_descriptortag[0x52]	= "Stream Identifier";
	dvb_descriptortag[0x53]	= "CA Identifier";
	dvb_descriptortag[0x56]	= "Teletext";
	dvb_descriptortag[0x5F]	= "Private Data Specifier";
	dvb_descriptortag[0x66]	= "Data Broadcast ID";
	dvb_descriptortag[0x6A] = "AC3 Audio";
	dvb_descriptortag[0x7A] = "AC3 Audio";
	dvb_descriptortag[0x7B] = "DTS Audio";
	dvb_descriptortag[0x7C] = "AAC Audio";
	dvb_descriptortag[0x80]	= "Stuffing";
	dvb_descriptortag[0x81]	= "AC3 audio";
	dvb_descriptortag[0x82]	= "User Defined";
	dvb_descriptortag[0x83]	= "User Defined";
	dvb_descriptortag[0x84]	= "User Defined";
	dvb_descriptortag[0x86]	= "Caption Service";
	dvb_descriptortag[0x87]	= "Content Advisory";
	dvb_descriptortag[0x88]	= "User Defined";
	dvb_descriptortag[0x90]	= "User Defined";
	dvb_descriptortag[0xA0]	= "Extended Channel Name";
	dvb_descriptortag[0xA1]	= "Service Location";
	dvb_descriptortag[0xA2]	= "Time-Shifted Service";
	dvb_descriptortag[0xA3]	= "Component Name";
	dvb_descriptortag[0xA5]	= "Data Service";
	dvb_descriptortag[0xA7]	= "User Defined";
	dvb_descriptortag[0xA8]	= "DCC Departing Request";
	dvb_descriptortag[0xA9]	= "DCC Arriving Request";
	dvb_descriptortag[0xAA]	= "Redistribution Control";
	dvb_descriptortag[0xAB]	= "Genre";
	dvb_descriptortag[0xAD]	= "ATSC Private Information";
	dvb_descriptortag[0xC3]	= "User Defined";
	dvb_descriptortag[0xF0]	= "User Defined";
	dvb_descriptortag[0xFC]	= "User Defined";
	dvb_descriptortag[0xFD]	= "User Defined";
	dvb_descriptortag[0xFE]	= "User Defined";
}

atsc::atsc()
{
	ch.clear();
	freq.clear();
	ch.append(2);
	freq.append(57028);
	ch.append(3);
	freq.append(63028);
	ch.append(4);
	freq.append(69028);
	ch.append(5);
	freq.append(79028);
	ch.append(6);
	freq.append(85028);
	ch.append(7);
	freq.append(177028);
	ch.append(8);
	freq.append(183028);
	ch.append(9);
	freq.append(189028);
	ch.append(10);
	freq.append(195028);
	ch.append(11);
	freq.append(201028);
	ch.append(12);
	freq.append(207028);
	ch.append(13);
	freq.append(213028);
	ch.append(14);
	freq.append(473028);
	ch.append(15);
	freq.append(479028);
	ch.append(16);
	freq.append(485028);
	ch.append(17);
	freq.append(491028);
	ch.append(18);
	freq.append(497028);
	ch.append(19);
	freq.append(503028);
	ch.append(20);
	freq.append(509028);
	ch.append(21);
	freq.append(515028);
	ch.append(22);
	freq.append(521028);
	ch.append(23);
	freq.append(527028);
	ch.append(24);
	freq.append(533028);
	ch.append(25);
	freq.append(539028);
	ch.append(26);
	freq.append(545028);
	ch.append(27);
	freq.append(551028);
	ch.append(28);
	freq.append(557028);
	ch.append(29);
	freq.append(563028);
	ch.append(30);
	freq.append(569028);
	ch.append(31);
	freq.append(575028);
	ch.append(32);
	freq.append(581028);
	ch.append(33);
	freq.append(587028);
	ch.append(34);
	freq.append(593028);
	ch.append(35);
	freq.append(599028);
	ch.append(36);
	freq.append(605028);
	ch.append(38);
	freq.append(617028);
	ch.append(39);
	freq.append(623028);
	ch.append(40);
	freq.append(629028);
	ch.append(41);
	freq.append(635028);
	ch.append(42);
	freq.append(641028);
	ch.append(43);
	freq.append(647028);
	ch.append(44);
	freq.append(653028);
	ch.append(45);
	freq.append(659028);
	ch.append(46);
	freq.append(665028);
	ch.append(47);
	freq.append(671028);
	ch.append(48);
	freq.append(677028);
	ch.append(49);
	freq.append(683028);
	ch.append(50);
	freq.append(689028);
	ch.append(51);
	freq.append(695028);
}
