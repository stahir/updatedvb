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

switch_settings::switch_settings()
{
	voltage		= -1;
	tone		= -1;
	committed	= -1;
	uncommitted	= -1;
}

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
	matype		= 0;
	ber			= 0;
	ucb			= 0;
	snr			= 0;
	lvl			= 0;
	spectrumscan_lvl	= 0;
	status		= 0;
}

tuning_options::tuning_options()
{
	f_start		= 3700;
	f_stop		= 4200;
	f_lof		= -5150;
	voltage		= SEC_VOLTAGE_18;
	tone		= SEC_TONE_OFF;
	mis			= -1;
	committed	= 0;
	uncommitted	= 0;
	site_lat	= 0;
	site_long	= 0;
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
	ca_name[0x0E01] = "PowerVu";
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

qam::qam()
{
	ch.clear();
	freq.clear();
	ch.append(2);
	freq.append(57000);
	ch.append(3);
	freq.append(63000);
	ch.append(4);
	freq.append(69000);
	ch.append(1);
	freq.append(75000);
	ch.append(5);
	freq.append(79000);
	ch.append(6);
	freq.append(85000);
	ch.append(95);
	freq.append(93000);
	ch.append(96);
	freq.append(99000);
	ch.append(97);
	freq.append(105000);
	ch.append(98);
	freq.append(111000);
	ch.append(99);
	freq.append(117000);
	ch.append(14);
	freq.append(123000);
	ch.append(15);
	freq.append(129000);
	ch.append(16);
	freq.append(135000);
	ch.append(17);
	freq.append(141000);
	ch.append(18);
	freq.append(147000);
	ch.append(19);
	freq.append(153000);
	ch.append(20);
	freq.append(159000);
	ch.append(21);
	freq.append(165000);
	ch.append(22);
	freq.append(171000);
	ch.append(7);
	freq.append(177000);
	ch.append(8);
	freq.append(183000);
	ch.append(9);
	freq.append(189000);
	ch.append(10);
	freq.append(195000);
	ch.append(11);
	freq.append(201000);
	ch.append(12);
	freq.append(207000);
	ch.append(13);
	freq.append(213000);
	ch.append(23);
	freq.append(219000);
	ch.append(24);
	freq.append(225000);
	ch.append(25);
	freq.append(231000);
	ch.append(26);
	freq.append(237000);
	ch.append(27);
	freq.append(243000);
	ch.append(28);
	freq.append(249000);
	ch.append(29);
	freq.append(255000);
	ch.append(30);
	freq.append(261000);
	ch.append(31);
	freq.append(267000);
	ch.append(32);
	freq.append(273000);
	ch.append(33);
	freq.append(279000);
	ch.append(34);
	freq.append(285000);
	ch.append(35);
	freq.append(291000);
	ch.append(36);
	freq.append(297000);
	ch.append(37);
	freq.append(303000);
	ch.append(38);
	freq.append(309000);
	ch.append(39);
	freq.append(315000);
	ch.append(40);
	freq.append(321000);
	ch.append(41);
	freq.append(327000);
	ch.append(42);
	freq.append(333000);
	ch.append(43);
	freq.append(339000);
	ch.append(44);
	freq.append(345000);
	ch.append(45);
	freq.append(351000);
	ch.append(46);
	freq.append(357000);
	ch.append(47);
	freq.append(363000);
	ch.append(48);
	freq.append(369000);
	ch.append(49);
	freq.append(375000);
	ch.append(50);
	freq.append(381000);
	ch.append(51);
	freq.append(387000);
	ch.append(52);
	freq.append(393000);
	ch.append(53);
	freq.append(399000);
	ch.append(54);
	freq.append(405000);
	ch.append(55);
	freq.append(411000);
	ch.append(56);
	freq.append(417000);
	ch.append(57);
	freq.append(423000);
	ch.append(58);
	freq.append(429000);
	ch.append(59);
	freq.append(435000);
	ch.append(60);
	freq.append(441000);
	ch.append(61);
	freq.append(447000);
	ch.append(62);
	freq.append(453000);
	ch.append(63);
	freq.append(459000);
	ch.append(64);
	freq.append(465000);
	ch.append(65);
	freq.append(471000);
	ch.append(66);
	freq.append(477000);
	ch.append(67);
	freq.append(483000);
	ch.append(68);
	freq.append(489000);
	ch.append(69);
	freq.append(495000);
	ch.append(70);
	freq.append(501000);
	ch.append(71);
	freq.append(507000);
	ch.append(72);
	freq.append(513000);
	ch.append(73);
	freq.append(519000);
	ch.append(74);
	freq.append(525000);
	ch.append(75);
	freq.append(531000);
	ch.append(76);
	freq.append(537000);
	ch.append(77);
	freq.append(543000);
	ch.append(78);
	freq.append(549000);
	ch.append(79);
	freq.append(555000);
	ch.append(80);
	freq.append(561000);
	ch.append(81);
	freq.append(567000);
	ch.append(82);
	freq.append(573000);
	ch.append(83);
	freq.append(579000);
	ch.append(84);
	freq.append(585000);
	ch.append(85);
	freq.append(591000);
	ch.append(86);
	freq.append(597000);
	ch.append(87);
	freq.append(603000);
	ch.append(88);
	freq.append(609000);
	ch.append(89);
	freq.append(615000);
	ch.append(90);
	freq.append(621000);
	ch.append(91);
	freq.append(627000);
	ch.append(92);
	freq.append(633000);
	ch.append(93);
	freq.append(639000);
	ch.append(94);
	freq.append(645000);
	ch.append(100);
	freq.append(651000);
	ch.append(101);
	freq.append(657000);
	ch.append(102);
	freq.append(663000);
	ch.append(103);
	freq.append(669000);
	ch.append(104);
	freq.append(675000);
	ch.append(105);
	freq.append(681000);
	ch.append(106);
	freq.append(687000);
	ch.append(107);
	freq.append(693000);
	ch.append(108);
	freq.append(699000);
	ch.append(109);
	freq.append(705000);
	ch.append(110);
	freq.append(711000);
	ch.append(111);
	freq.append(717000);
	ch.append(112);
	freq.append(723000);
	ch.append(113);
	freq.append(729000);
	ch.append(114);
	freq.append(735000);
	ch.append(115);
	freq.append(741000);
	ch.append(116);
	freq.append(747000);
	ch.append(117);
	freq.append(753000);
	ch.append(118);
	freq.append(759000);
	ch.append(119);
	freq.append(765000);
	ch.append(120);
	freq.append(771000);
	ch.append(121);
	freq.append(777000);
	ch.append(122);
	freq.append(783000);
	ch.append(123);
	freq.append(789000);
	ch.append(124);
	freq.append(795000);
	ch.append(125);
	freq.append(801000);
	ch.append(126);
	freq.append(807000);
	ch.append(127);
	freq.append(813000);
	ch.append(128);
	freq.append(819000);
	ch.append(129);
	freq.append(825000);
	ch.append(130);
	freq.append(831000);
	ch.append(131);
	freq.append(837000);
	ch.append(132);
	freq.append(843000);
	ch.append(133);
	freq.append(849000);
	ch.append(134);
	freq.append(855000);
	ch.append(135);
	freq.append(861000);
	ch.append(136);
	freq.append(867000);
	ch.append(137);
	freq.append(873000);
	ch.append(138);
	freq.append(879000);
	ch.append(139);
	freq.append(885000);
	ch.append(140);
	freq.append(891000);
	ch.append(141);
	freq.append(897000);
	ch.append(142);
	freq.append(903000);
	ch.append(143);
	freq.append(909000);
	ch.append(144);
	freq.append(915000);
	ch.append(145);
	freq.append(921000);
	ch.append(146);
	freq.append(927000);
	ch.append(147);
	freq.append(933000);
	ch.append(148);
	freq.append(939000);
	ch.append(149);
	freq.append(945000);
	ch.append(150);
	freq.append(951000);
	ch.append(151);
	freq.append(957000);
	ch.append(152);
	freq.append(963000);
	ch.append(153);
	freq.append(969000);
	ch.append(154);
	freq.append(975000);
	ch.append(155);
	freq.append(981000);
	ch.append(156);
	freq.append(987000);
	ch.append(157);
	freq.append(993000);
	ch.append(158);
	freq.append(999000);
}

bool isSatellite(int system)
{
	bool ret = false;
	switch(system) {
	case SYS_DCII:
	case SYS_DSS:
	case SYS_DVBS:
	case SYS_DVBS2:
	case SYS_TURBO:
		ret = true;
	}
	return ret;
}

bool isATSC(int system)
{
	bool ret = false;
	switch(system) {
	case SYS_ATSC:
	case SYS_ATSCMH:
		ret = true;
	}
	return ret;
}

bool isQAM(int system)
{
	bool ret = false;
	switch(system) {
	case SYS_DVBC_ANNEX_B:
		ret = true;
	}
	return ret;
}

int azero(int num)
{
	if (num < 0) {
		num = 0;
	}
	return num;
}
