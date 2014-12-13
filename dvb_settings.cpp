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

rating_region::rating_region()
{
	name.append("Undefined");
	name.append("US");
	name.append("Canada");
	name.append("Taiwan");
	name.append("South Korea");
	name.append("Alternate US Rating Region");
}

ac3_desc::ac3_desc()
{
	sample_rate_code.append("48 khz");
	sample_rate_code.append("44.1 khz");
	sample_rate_code.append("32 khz");
	sample_rate_code.append("Reserved");
	sample_rate_code.append("48 or 44.1khz");
	sample_rate_code.append("48 or 32khz");
	sample_rate_code.append("44.1 or 32khz");
	sample_rate_code.append("48 or 44.1 or 32khz");

	bit_rate_code.append("32 kbit/sec");
	bit_rate_code.append("40 kbit/sec");
	bit_rate_code.append("48 kbit/sec");
	bit_rate_code.append("56 kbit/sec");
	bit_rate_code.append("64 kbit/sec");
	bit_rate_code.append("80 kbit/sec");
	bit_rate_code.append("96 kbit/sec");
	bit_rate_code.append("112 kbit/sec");
	bit_rate_code.append("128 kbit/sec");
	bit_rate_code.append("160 kbit/sec");
	bit_rate_code.append("192 kbit/sec");
	bit_rate_code.append("224 kbit/sec");
	bit_rate_code.append("256 kbit/sec");
	bit_rate_code.append("320 kbit/sec");
	bit_rate_code.append("384 kbit/sec");
	bit_rate_code.append("448 kbit/sec");
	bit_rate_code.append("512 kbit/sec");
	bit_rate_code.append("576 kbit/sec");
	bit_rate_code.append("640 kbit/sec");

	dsurmod.append("Not Indicated");
	dsurmod.append("Not Dolby Suround Encoded");
	dsurmod.append("Dolby Suround Encoded");
	dsurmod.append("Reserved");

	num_channels.append("1+1");
	num_channels.append("1/0");
	num_channels.append("2/0");
	num_channels.append("3/0");
	num_channels.append("2/1");
	num_channels.append("3/1");
	num_channels.append("2/2");
	num_channels.append("3/2");
	num_channels.append("1");
	num_channels.append("<=2");
	num_channels.append("<=3");
	num_channels.append("<=4");
	num_channels.append("<=5");
	num_channels.append("<=6");
	num_channels.append("Reserved");
	num_channels.append("Reserved");

	priority.append("Reserved");
	priority.append("Primary Audio");
	priority.append("Other Audio");
	priority.append("Not Specified");

	bsmode.append("main audio service - complete main (CM)");
	bsmode.append("main audio service - music & effects (ME)");
	bsmode.append("associated service - visually impaired (VI)");
	bsmode.append("associated service - hearing impaired (HI)");
	bsmode.append("associated service - dialogue (D)");
	bsmode.append("associated service - commentary (C)");
	bsmode.append("associated service - emergency (E)");
	bsmode.append("associated service - voice over (VO)");
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
	ber_scale	= FE_SCALE_COUNTER;
	snr_scale	= FE_SCALE_DECIBEL;
	lvl_scale	= FE_SCALE_DECIBEL;
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

data_stream_type::data_stream_type()
{
	name.fill("Reserved", 0xFF);
	name[0x01] = "Slice or Video Access Unit";
	name[0x02] = "Video Access Unit";
	name[0x03] = "GOP or SEQ";
	name[0x04] = "SEQ";
}

frame_rate::frame_rate()
{
	rate.fill("Reserved", 0xFF);
	rate[0x00] = "Forbidden";
	rate[0x01] = "23.976 fps";
	rate[0x02] = "24 fps";
	rate[0x03] = "25 fps";
	rate[0x04] = "29.97 fps";
	rate[0x05] = "30 fps";
	rate[0x06] = "50 fps";
	rate[0x07] = "59.95 fps";
	rate[0x08] = "60 fps";
}

QString cue_stream::whatis(unsigned int val)
{
	for (int i = 0; i < text.size(); i++) {
		if (val >= min.at(i) && val <= max.at(i)) {
			return text.at(i);
		}
	}
	return "user defined";
}

cue_stream::cue_stream()
{
	min.append(0x00);	max.append(0x01);	text.append("splice_insert, splice_null, splice_schedule");
	min.append(0x01);	max.append(0x02);	text.append("All Commands");
	min.append(0x02);	max.append(0x02);	text.append("Segmentation");
	min.append(0x03);	max.append(0x03);	text.append("Tiered Splicing");
	min.append(0x04);	max.append(0x04);	text.append("Tiered Segmentation");
	min.append(0x05);	max.append(0x7F);	text.append("Reserved");
	min.append(0x80);	max.append(0xFF);	text.append("User Defined");
}

QString mgt_table::whatis(unsigned int val)
{
	for (int i = 0; i < text.size(); i++) {
		if (val >= min.at(i) && val <= max.at(i)) {
			return text.at(i);
		}
	}
	return "user defined";
}

mgt_table::mgt_table()
{
	min.append(0x0000);	max.append(0x0000);	text.append("Terrestrial VCT with current_next_indicator=1");
	min.append(0x0001);	max.append(0x0001);	text.append("Terrestrial VCT with current_next_indicator=0");
	min.append(0x0002);	max.append(0x0002);	text.append("Cable VCT with current_next_indicator=1");
	min.append(0x0003);	max.append(0x0003);	text.append("Cable VCT with current_next_indicator=0");
	min.append(0x0004);	max.append(0x0004);	text.append("Channel ETT");
	min.append(0x0005);	max.append(0x0005);	text.append("DCCSCT");
	min.append(0x0006);	max.append(0x0006);	text.append("LTST");
	min.append(0x0007);	max.append(0x00FF);	text.append("(Reserved for future ATSC use)");
	min.append(0x0010);	max.append(0x0010);	text.append("Short-form VCT – VCM Subtype");
	min.append(0x0011);	max.append(0x0011);	text.append("Short-form VCT – DCM Subtype");
	min.append(0x0012);	max.append(0x0012);	text.append("Short-form VCT – ICM Subtype");
	min.append(0x0020);	max.append(0x0020);	text.append("Network Information Table - CDS Table Subtype");
	min.append(0x0021);	max.append(0x0021);	text.append("Network Information Table - MMS Table Subtype");
	min.append(0x0030);	max.append(0x0030);	text.append("Network Text Table – SNS Subtype");
	min.append(0x0100);	max.append(0x017F);	text.append("EIT-0 to EIT-127");
	min.append(0x0180);	max.append(0x01FF);	text.append("Reserved for future ATSC use");
	min.append(0x0200);	max.append(0x027F);	text.append("Event ETT-0 to Event ETT-127");
	min.append(0x0280);	max.append(0x0300);	text.append("Reserved for future ATSC use");
	min.append(0x0301);	max.append(0x03FF);	text.append("RRT with rating_region 1-255");
	min.append(0x0400);	max.append(0x0FFF);	text.append("User private");
	min.append(0x1000);	max.append(0xFFFF);	text.append("Reserved for future ATSC use");
	min.append(0x1000);	max.append(0x10FF);	text.append("Aggregate Extended Information Table with MGT_tag 0 to 255");
	min.append(0x1100);	max.append(0x11FF);	text.append("SCTE Aggregate Extended Text Table with MGT_tag 0 to 255");
	min.append(0x1180);	max.append(0x1180);	text.append("Long Term Service Table");
	min.append(0x1200);	max.append(0x127F);	text.append("Extended Text Table for DET");
	min.append(0x1300);	max.append(0x137F);	text.append("Data Event Table");
	min.append(0x1380);	max.append(0x13FF);	text.append("Reserved for future ATSC use");
	min.append(0x1400);	max.append(0x14FF);	text.append("DCCT (with dcc_id 0x00 – 0xFF)");
	min.append(0x1500);	max.append(0x157F);	text.append("Aggregate Data Event Table");
	min.append(0x1580);	max.append(0x15FF);	text.append("Reserved");
	min.append(0x1600);	max.append(0x16FF);	text.append("Satellite VCT");
}

QString data_broadcast_id::whatis(unsigned int val)
{
	for (int i = 0; i < text.size(); i++) {
		if (val >= min.at(i) && val <= max.at(i)) {
			return text.at(i);
		}
	}
	return "user defined";
}

data_broadcast_id::data_broadcast_id()
{
	min.append(0x0000);	max.append(0x0000);	text.append("Reserved for future use");
	min.append(0x0001);	max.append(0x0001);	text.append("Data pipe");
	min.append(0x0002);	max.append(0x0002);	text.append("Asynchronous data stream");
	min.append(0x0003);	max.append(0x0003);	text.append("Synchronous data stream");
	min.append(0x0004);	max.append(0x0004);	text.append("Synchronised data stream");
	min.append(0x0005);	max.append(0x0005);	text.append("Multi protocol encapsulation");
	min.append(0x0006);	max.append(0x0006);	text.append("Data Carousel");
	min.append(0x0007);	max.append(0x0007);	text.append("Object Carousel");
	min.append(0x0008);	max.append(0x0008);	text.append("DVB ATM streams");
	min.append(0x0009);	max.append(0x0009);	text.append("Higher Protocols based on asynchronous data streams");
	min.append(0x000A);	max.append(0x000A);	text.append("System Software Update");
	min.append(0x000B);	max.append(0x00ef);	text.append("Reserved for future use by DVB");
	min.append(0x00F0);	max.append(0x00F0);	text.append("MHP Object Carousel");
	min.append(0x00F1);	max.append(0x00F1);	text.append("reserved for MHP Multi Protocol Encapsulation");
	min.append(0x00F2);	max.append(0x00Fe);	text.append("Reserved for MHP use");
	min.append(0x00FF);	max.append(0x00FF);	text.append("Reserved for future use by DVB");
	min.append(0x0100);	max.append(0x0100);	text.append("Eutelsat Data Piping");
	min.append(0x0101);	max.append(0x0101);	text.append("Eutelsat Data Streaming");
	min.append(0x0102);	max.append(0x0102);	text.append("SAGEM IP encapsulation in MPEG-2 PES packets");
	min.append(0x0103);	max.append(0x0103);	text.append("BARCO Data Broadcasting");
	min.append(0x0104);	max.append(0x0104);	text.append("CyberCity Multiprotocol Encapsulation (New Media Communications Ltd.)");
	min.append(0x0105);	max.append(0x0105);	text.append("CyberSat Multiprotocol Encapsulation (New Media Communications Ltd.)");
	min.append(0x0106);	max.append(0x0106);	text.append("The Digital Network");
	min.append(0x0107);	max.append(0x0107);	text.append("OpenTV Data Carousel");
	min.append(0x0108);	max.append(0x0108);	text.append("Panasonic");
	min.append(0x0109);	max.append(0x0109);	text.append("MSG MediaServices GmbH");
	min.append(0x010A);	max.append(0x010A);	text.append("TechnoTrend");
	min.append(0x010B);	max.append(0x010B);	text.append("Canal + Technologies system software download");
	min.append(0x0110);	max.append(0x0110);	text.append("Televizja Polsat");
	min.append(0x0111);	max.append(0x0111);	text.append("UK DTG");
	min.append(0x0112);	max.append(0x0112);	text.append("SkyMedia");
	min.append(0x0113);	max.append(0x0113);	text.append("Intellibyte DataBroadcasting");
	min.append(0x0114);	max.append(0x0114);	text.append("TeleWeb Data Carousel");
	min.append(0x0115);	max.append(0x0115);	text.append("TeleWeb Object Carousel");
	min.append(0x0116);	max.append(0x0116);	text.append("TeleWeb");
	min.append(0x4444);	max.append(0x4444);	text.append("4TV Data Broadcast");
	min.append(0x4E4F);	max.append(0x4E4F);	text.append("Nokia IP based software delivery");
	min.append(0xBBB1);	max.append(0xBBB1);	text.append("BBG Data Caroussel");
	min.append(0xBBB2);	max.append(0xBBB2);	text.append("BBG Object Caroussel");
	min.append(0xBBBB);	max.append(0xBBBB);	text.append("Bertelsmann Broadband Group");
	min.append(0xFFFF);	max.append(0xFFFF);	text.append("Reserved for future use");
}

QString stream_content::whatis(unsigned int stream_type, unsigned int component_type)
{
	for (int i = 0; i < text.size(); i++) {
		if (stream_type == sval.at(i) && component_type >= cmin.at(i) && component_type <= cmax.at(i)) {
			return text.at(i);
		}
	}
	return "user defined";
}

stream_content::stream_content()
{
	sval.append(0x00);	cmin.append(0x00);	cmax.append(0xFF);	text.append("reserved for future use");
	sval.append(0x01);	cmin.append(0x00);	cmax.append(0x00);	text.append("reserved for future use");
	sval.append(0x01);	cmin.append(0x01);	cmax.append(0x01);	text.append("MPEG-2 video, 4:3 aspect ratio, 25 Hz");
	sval.append(0x01);	cmin.append(0x02);	cmax.append(0x02);	text.append("MPEG-2 video, 16:9 aspect ratio with pan vectors, 25 Hz");
	sval.append(0x01);	cmin.append(0x03);	cmax.append(0x03);	text.append("MPEG-2 video, 16:9 aspect ratio without pan vectors, 25 Hz");
	sval.append(0x01);	cmin.append(0x04);	cmax.append(0x04);	text.append("MPEG-2 video, > 16:9 aspect ratio, 25 Hz");
	sval.append(0x01);	cmin.append(0x05);	cmax.append(0x05);	text.append("MPEG-2 video, 4:3 aspect ratio, 30 Hz");
	sval.append(0x01);	cmin.append(0x06);	cmax.append(0x06);	text.append("MPEG-2 video, 16:9 aspect ratio with pan vectors, 30 Hz");
	sval.append(0x01);	cmin.append(0x07);	cmax.append(0x07);	text.append("MPEG-2 video, 16:9 aspect ratio without pan vectors, 30 Hz");
	sval.append(0x01);	cmin.append(0x08);	cmax.append(0x08);	text.append("MPEG-2 video, > 16:9 aspect ratio, 30 Hz");
	sval.append(0x01);	cmin.append(0x09);	cmax.append(0x09);	text.append("MPEG-2 high definition video, 4:3 aspect ratio, 25 Hz");
	sval.append(0x01);	cmin.append(0x0A);	cmax.append(0x0A);	text.append("MPEG-2 high definition video, 16:9 aspect ratio with pan vectors, 25 Hz");
	sval.append(0x01);	cmin.append(0x0B);	cmax.append(0x0B);	text.append("MPEG-2 high definition video, 16:9 aspect ratio without pan vectors, 25 Hz");
	sval.append(0x01);	cmin.append(0x0C);	cmax.append(0x0C);	text.append("MPEG-2 high definition video, > 16:9 aspect ratio, 25 Hz");
	sval.append(0x01);	cmin.append(0x0D);	cmax.append(0x0D);	text.append("MPEG-2 high definition video, 4:3 aspect ratio, 30 Hz");
	sval.append(0x01);	cmin.append(0x0E);	cmax.append(0x0E);	text.append("MPEG-2 high definition video, 16:9 aspect ratio with pan vectors, 30 Hz");
	sval.append(0x01);	cmin.append(0x0F);	cmax.append(0x0F);	text.append("MPEG-2 high definition video, 16:9 aspect ratio without pan vectors, 30 Hz");
	sval.append(0x01);	cmin.append(0x10);	cmax.append(0x10);	text.append("MPEG-2 high definition video, > 16:9 aspect ratio, 30 Hz");
	sval.append(0x01);	cmin.append(0x11);	cmax.append(0xAF);	text.append("reserved for future use");
	sval.append(0x01);	cmin.append(0xB0);	cmax.append(0xFE);	text.append("user defined");
	sval.append(0x01);	cmin.append(0xFF);	cmax.append(0xFF);	text.append("reserved for future use");
	sval.append(0x02);	cmin.append(0x00);	cmax.append(0x00);	text.append("reserved for future use");
	sval.append(0x02);	cmin.append(0x01);	cmax.append(0x01);	text.append("MPEG-1 Layer 2 audio, single mono channel");
	sval.append(0x02);	cmin.append(0x02);	cmax.append(0x02);	text.append("MPEG-1 Layer 2 audio, dual mono channel");
	sval.append(0x02);	cmin.append(0x03);	cmax.append(0x03);	text.append("MPEG-1 Layer 2 audio, stereo (2 channel)");
	sval.append(0x02);	cmin.append(0x04);	cmax.append(0x04);	text.append("MPEG-1 Layer 2 audio, multi-lingual, multi-channel");
	sval.append(0x02);	cmin.append(0x05);	cmax.append(0x05);	text.append("MPEG-1 Layer 2 audio, surround sound");
	sval.append(0x02);	cmin.append(0x06);	cmax.append(0x3F);	text.append("reserved for future use");
	sval.append(0x02);	cmin.append(0x40);	cmax.append(0x40);	text.append("MPEG-1 Layer 2 audio description for the visually impaired");
	sval.append(0x02);	cmin.append(0x41);	cmax.append(0x41);	text.append("MPEG-1 Layer 2 audio for the hard of hearing");
	sval.append(0x02);	cmin.append(0x42);	cmax.append(0x42);	text.append("receiver-mixed supplementary audio as per annex E of TS 101 154 [9]");
	sval.append(0x02);	cmin.append(0x43);	cmax.append(0x46);	text.append("reserved for future use");
	sval.append(0x02);	cmin.append(0x47);	cmax.append(0x47);	text.append("MPEG-1 Layer 2 audio, receiver mix audio description as per annex E of");
	sval.append(0x02);	cmin.append(0x48);	cmax.append(0x48);	text.append("MPEG-1 Layer 2 audio, broadcaster mix audio description");
	sval.append(0x02);	cmin.append(0x49);	cmax.append(0xAF);	text.append("reserved for future use");
	sval.append(0x02);	cmin.append(0xB0);	cmax.append(0xFE);	text.append("user-defined");
	sval.append(0x02);	cmin.append(0xFF);	cmax.append(0xFF);	text.append("reserved for future use");
	sval.append(0x03);	cmin.append(0x00);	cmax.append(0x00);	text.append("reserved for future use");
	sval.append(0x03);	cmin.append(0x01);	cmax.append(0x01);	text.append("EBU Teletext subtitles");
	sval.append(0x03);	cmin.append(0x02);	cmax.append(0x02);	text.append("associated EBU Teletext");
	sval.append(0x03);	cmin.append(0x03);	cmax.append(0x03);	text.append("VBI data");
	sval.append(0x03);	cmin.append(0x04);	cmax.append(0x0F);	text.append("reserved for future use");
	sval.append(0x03);	cmin.append(0x10);	cmax.append(0x10);	text.append("DVB subtitles (normal) with no monitor aspect ratio criticality");
	sval.append(0x03);	cmin.append(0x11);	cmax.append(0x11);	text.append("DVB subtitles (normal) for display on 4:3 aspect ratio monitor");
	sval.append(0x03);	cmin.append(0x12);	cmax.append(0x12);	text.append("DVB subtitles (normal) for display on 16:9 aspect ratio monitor");
	sval.append(0x03);	cmin.append(0x13);	cmax.append(0x13);	text.append("DVB subtitles (normal) for display on 2.21:1 aspect ratio monitor");
	sval.append(0x03);	cmin.append(0x14);	cmax.append(0x14);	text.append("DVB subtitles (normal) for display on a high definition monitor");
	sval.append(0x03);	cmin.append(0x15);	cmax.append(0x1F);	text.append("reserved for future use");
	sval.append(0x03);	cmin.append(0x20);	cmax.append(0x20);	text.append("DVB subtitles (for the hard of hearing) with no monitor aspect ratio criticality");
	sval.append(0x03);	cmin.append(0x21);	cmax.append(0x21);	text.append("DVB subtitles (for the hard of hearing) for display on 4:3 aspect ratio monitor");
	sval.append(0x03);	cmin.append(0x22);	cmax.append(0x22);	text.append("DVB subtitles (for the hard of hearing) for display on 16:9 aspect ratio monitor");
	sval.append(0x03);	cmin.append(0x23);	cmax.append(0x23);	text.append("DVB subtitles (for the hard of hearing) for display on 2.21:1 aspect ratio monitor");
	sval.append(0x03);	cmin.append(0x24);	cmax.append(0x24);	text.append("DVB subtitles (for the hard of hearing) for display on a high definition monitor");
	sval.append(0x03);	cmin.append(0x25);	cmax.append(0x2F);	text.append("reserved for future use");
	sval.append(0x03);	cmin.append(0x30);	cmax.append(0x30);	text.append("Open (in-vision) sign language interpretation for the deaf");
	sval.append(0x03);	cmin.append(0x31);	cmax.append(0x31);	text.append("Closed sign language interpretation for the deaf");
	sval.append(0x03);	cmin.append(0x32);	cmax.append(0x3F);	text.append("reserved for future use");
	sval.append(0x03);	cmin.append(0x40);	cmax.append(0x40);	text.append("video up-sampled from standard definition source material");
	sval.append(0x03);	cmin.append(0x41);	cmax.append(0xAF);	text.append("reserved for future use");
	sval.append(0x03);	cmin.append(0xB0);	cmax.append(0xFE);	text.append("user defined");
	sval.append(0x03);	cmin.append(0xFF);	cmax.append(0xFF);	text.append("reserved for future use");
	sval.append(0x04);	cmin.append(0x00);	cmax.append(0x7F);	text.append("reserved for AC-3 audio modes (refer to table D.1)");
	sval.append(0x04);	cmin.append(0x80);	cmax.append(0xFF);	text.append("reserved for enhanced AC-3 audio modes (refer to table D.1)");
	sval.append(0x05);	cmin.append(0x00);	cmax.append(0x00);	text.append("reserved for future use");
	sval.append(0x05);	cmin.append(0x01);	cmax.append(0x01);	text.append("H.264/AVC standard definition video, 4:3 aspect ratio, 25 Hz");
	sval.append(0x05);	cmin.append(0x02);	cmax.append(0x02);	text.append("reserved for future use");
	sval.append(0x05);	cmin.append(0x03);	cmax.append(0x03);	text.append("H.264/AVC standard definition video, 16:9 aspect ratio, 25 Hz");
	sval.append(0x05);	cmin.append(0x04);	cmax.append(0x04);	text.append("H.264/AVC standard definition video, > 16:9 aspect ratio, 25 Hz");
	sval.append(0x05);	cmin.append(0x05);	cmax.append(0x05);	text.append("H.264/AVC standard definition video, 4:3 aspect ratio, 30 Hz");
	sval.append(0x05);	cmin.append(0x06);	cmax.append(0x06);	text.append("reserved for future use");
	sval.append(0x05);	cmin.append(0x07);	cmax.append(0x07);	text.append("H.264/AVC standard definition video, 16:9 aspect ratio, 30 Hz");
	sval.append(0x05);	cmin.append(0x08);	cmax.append(0x08);	text.append("H.264/AVC standard definition video, > 16:9 aspect ratio, 30 Hz");
	sval.append(0x05);	cmin.append(0x09);	cmax.append(0x0A);	text.append("reserved for future use");
	sval.append(0x05);	cmin.append(0x0B);	cmax.append(0x0B);	text.append("H.264/AVC high definition video, 16:9 aspect ratio, 25 Hz");
	sval.append(0x05);	cmin.append(0x0C);	cmax.append(0x0C);	text.append("H.264/AVC high definition video, > 16:9 aspect ratio, 25 Hz");
	sval.append(0x05);	cmin.append(0x0D);	cmax.append(0x0E);	text.append("reserved for future use");
	sval.append(0x05);	cmin.append(0x0F);	cmax.append(0x0F);	text.append("H.264/AVC high definition video, 16:9 aspect ratio, 30 Hz");
	sval.append(0x05);	cmin.append(0x10);	cmax.append(0x10);	text.append("H.264/AVC high definition video, > 16:9 aspect ratio, 30 Hz");
	sval.append(0x05);	cmin.append(0x11);	cmax.append(0xAF);	text.append("reserved for future use");
	sval.append(0x05);	cmin.append(0xB0);	cmax.append(0xFE);	text.append("user-defined");
	sval.append(0x05);	cmin.append(0xFF);	cmax.append(0xFF);	text.append("reserved for future use");
	sval.append(0x06);	cmin.append(0x00);	cmax.append(0x00);	text.append("reserved for future use");
	sval.append(0x06);	cmin.append(0x01);	cmax.append(0x01);	text.append("HE-AAC audio, single mono channel");
	sval.append(0x06);	cmin.append(0x02);	cmax.append(0x02);	text.append("reserved for future use");
	sval.append(0x06);	cmin.append(0x03);	cmax.append(0x03);	text.append("HE-AAC audio, stereo");
	sval.append(0x06);	cmin.append(0x04);	cmax.append(0x04);	text.append("reserved for future use");
	sval.append(0x06);	cmin.append(0x05);	cmax.append(0x05);	text.append("HE-AAC audio, surround sound");
	sval.append(0x06);	cmin.append(0x06);	cmax.append(0x3F);	text.append("reserved for future use");
	sval.append(0x06);	cmin.append(0x40);	cmax.append(0x40);	text.append("HE-AAC audio description for the visually impaired");
	sval.append(0x06);	cmin.append(0x41);	cmax.append(0x41);	text.append("HE-AAC audio for the hard of hearing");
	sval.append(0x06);	cmin.append(0x42);	cmax.append(0x42);	text.append("HE-AAC receiver-mixed supplementary audio as per annex E of TS 101 154 [9]");
	sval.append(0x06);	cmin.append(0x43);	cmax.append(0x43);	text.append("HE-AAC v2 audio, stereo");
	sval.append(0x06);	cmin.append(0x44);	cmax.append(0x44);	text.append("HE-AAC v2 audio description for the visually impaired");
	sval.append(0x06);	cmin.append(0x45);	cmax.append(0x45);	text.append("HE-AAC v2 audio for the hard of hearing");
	sval.append(0x06);	cmin.append(0x46);	cmax.append(0x46);	text.append("HE-AAC v2 receiver-mixed supplementary audio as per annex E of TS 101 154 [9]");
	sval.append(0x06);	cmin.append(0x47);	cmax.append(0x47);	text.append("HE-AAC receiver mix audio description for the visually impaired");
	sval.append(0x06);	cmin.append(0x48);	cmax.append(0x48);	text.append("HE-AAC broadcaster mix audio description for the visually impaired");
	sval.append(0x06);	cmin.append(0x49);	cmax.append(0x49);	text.append("HE-AAC v2 receiver mix audio description for the visually impaired");
	sval.append(0x06);	cmin.append(0x4A);	cmax.append(0x4A);	text.append("HE-AAC v2 broadcaster mix audio description for the visually impaired");
	sval.append(0x06);	cmin.append(0x4B);	cmax.append(0xAF);	text.append("reserved for future use");
	sval.append(0x06);	cmin.append(0xB0);	cmax.append(0xFE);	text.append("user-defined");
	sval.append(0x06);	cmin.append(0xFF);	cmax.append(0xFF);	text.append("reserved for future use");
	sval.append(0x07);	cmin.append(0x00);	cmax.append(0x7F);	text.append("reserved for DTS audio modes (refer to annex G)");
	sval.append(0x07);	cmin.append(0x80);	cmax.append(0xFF);	text.append("reserved for future use");
	sval.append(0x08);	cmin.append(0x00);	cmax.append(0x00);	text.append("reserved for future use");
	sval.append(0x08);	cmin.append(0x01);	cmax.append(0x01);	text.append("DVB SRM data [48]");
	sval.append(0x08);	cmin.append(0x02);	cmax.append(0xFF);	text.append("reserved for DVB CPCM modes [46] to [i.4]");
}

tree_item::tree_item()
{
	text.clear();
	parent			= -1;
	current			= -1;
	color			= QColor(Qt::green);
	expanded		= true;
	return_parent	= true;
	pid				= 0xFFFF;
	table			= 0xFFFF;
}

void tree_item::save()
{
	saved = parent;
}

void tree_item::restore()
{
	parent = saved;
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
	voltage.append(" B ");

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
	stream_type[0x0B] = "DSM-CC sections containing A/90 [17] asynchronous data";
	stream_type[0x0C] = "Data Broadcast Service";
	stream_type[0x0D] = "ISO/IEC 13818-6 Sections (any type, including private data)";
	stream_type[0x0F] = "ISO/IEC 13818-7 Audio with ADTS transport syntax";
	stream_type[0x14] = "DSM-CC sections containing non-streaming, synchronized data per A/90 [17]";
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
	stream_type[0x95] = "Sections conveying A/90 [17] Data Service Table, Network Resources Table";
	stream_type[0xC0] = "User private";
	stream_type[0xC1] = "User private";
	stream_type[0xC2] = "PES packets containing A/90 [17] streaming, synchronous data";
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
	dvb_descriptortag[0x4D]	= "Short Event";
	dvb_descriptortag[0x50]	= "Component";
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
	dvb_descriptortag[0x83]	= "Logical Channel";
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

QString private_data_specifier::whois(unsigned long val)
{
	for (int i = 0; i < text.size(); i++) {
		if (val >= min.at(i) && val <= max.at(i)) {
			return text.at(i);
		}
	}
	return "unknown";
}

private_data_specifier::private_data_specifier()
{
	min.append(0x00000000);	max.append(0x00000000);	text.append("Reserved");
	min.append(0x00000001);	max.append(0x00000001);	text.append("SES");
	min.append(0x00000002);	max.append(0x00000002);	text.append("BskyB 1");
	min.append(0x00000003);	max.append(0x00000003);	text.append("BskyB 2");
	min.append(0x00000004);	max.append(0x00000004);	text.append("BskyB 3");
	min.append(0x00000005);	max.append(0x00000005);	text.append("ARD, ZDF, ORF");
	min.append(0x00000006);	max.append(0x00000006);	text.append("Nokia Multimedia Network Terminals");
	min.append(0x00000007);	max.append(0x00000007);	text.append("AT Entertainment Ltd.");
	min.append(0x00000008);	max.append(0x00000008);	text.append("TV Cabo Portugal");
	min.append(0x00000009);	max.append(0x0000000D);	text.append("Nagravision SA // Kudelski");
	min.append(0x0000000E);	max.append(0x0000000E);	text.append("Valvision SA");
	min.append(0x0000000F);	max.append(0x0000000F);	text.append("Quiero Television");
	min.append(0x00000010);	max.append(0x00000010);	text.append("La Television Par Satellite (TPS)");
	min.append(0x00000011);	max.append(0x00000011);	text.append("Echostar Communications");
	min.append(0x00000012);	max.append(0x00000012);	text.append("Telia AB");
	min.append(0x00000013);	max.append(0x00000013);	text.append("Viasat");
	min.append(0x00000014);	max.append(0x00000014);	text.append("Senda (Swedish Terrestrial TV)");
	min.append(0x00000015);	max.append(0x00000015);	text.append("MediaKabel");
	min.append(0x00000016);	max.append(0x00000016);	text.append("Casema");
	min.append(0x00000017);	max.append(0x00000017);	text.append("Humax Electronics Co. Ltd.");
	min.append(0x00000018);	max.append(0x00000018);	text.append("@Sky");
	min.append(0x00000019);	max.append(0x00000019);	text.append("Singapore Digital Terrestrial Television");
	min.append(0x0000001A);	max.append(0x0000001A);	text.append("Telediffusion de France (TDF)");
	min.append(0x0000001B);	max.append(0x0000001B);	text.append("Intellibyte Inc.");
	min.append(0x0000001C);	max.append(0x0000001C);	text.append("Digital Theater Systems Ltd");
	min.append(0x0000001D);	max.append(0x0000001D);	text.append("Finlux Ltd.");
	min.append(0x0000001E);	max.append(0x0000001E);	text.append("Sagem SA");
	min.append(0x00000020);	max.append(0x00000023);	text.append("Lyonnaise Cable");
	min.append(0x00000025);	max.append(0x00000025);	text.append("MTV Europe");
	min.append(0x00000026);	max.append(0x00000026);	text.append("Pansonic");
	min.append(0x00000027);	max.append(0x00000027);	text.append("Mentor Data System, Inc.");
	min.append(0x00000028);	max.append(0x00000028);	text.append("EACEM");
	min.append(0x00000029);	max.append(0x00000029);	text.append("NorDig");
	min.append(0x0000002A);	max.append(0x0000002A);	text.append("Intelsis Sistemas Inteligentes S.A.");
	min.append(0x0000002D);	max.append(0x0000002D);	text.append("Alpha Digital Synthesis S.A.");
	min.append(0x0000002F);	max.append(0x0000002F);	text.append("Conax A.S.");
	min.append(0x00000030);	max.append(0x00000030);	text.append("Telenor");
	min.append(0x00000031);	max.append(0x00000031);	text.append("TeleDenmark");
	min.append(0x00000035);	max.append(0x00000035);	text.append("Europe Online Networks S.A.");
	min.append(0x00000038);	max.append(0x00000038);	text.append("OTE");
	min.append(0x00000039);	max.append(0x00000039);	text.append("Telewizja Polsat");
	min.append(0x000000A0);	max.append(0x000000A0);	text.append("Sentech");
	min.append(0x000000A1);	max.append(0x000000A1);	text.append("TechniSat Digital GmbH");
	min.append(0x000000BE);	max.append(0x000000BE);	text.append("BetaTechnik");
	min.append(0x000000C0);	max.append(0x000000C0);	text.append("Canal+");
	min.append(0x000000D0);	max.append(0x000000D0);	text.append("Dolby Laboratories Inc.");
	min.append(0x000000E0);	max.append(0x000000E0);	text.append("ExpressVu Inc.");
	min.append(0x000000F0);	max.append(0x000000F0);	text.append("France Telecom, CNES and DGA (STENTOR)");
	min.append(0x00000100);	max.append(0x00000100);	text.append("OpenTV");
	min.append(0x00000150);	max.append(0x00000150);	text.append("Loewe Opta GmbH");
	min.append(0x00000600);	max.append(0x00000601);	text.append("UPC 1");
	min.append(0x00000ACE);	max.append(0x00000ACE);	text.append("Ortikon Interactive Oy");
	min.append(0x00001000);	max.append(0x00001000);	text.append("La Television Par Satellite (TPS)");
	min.append(0x000022D4);	max.append(0x000022D4);	text.append("Spanish Broadcasting Regulator");
	min.append(0x000022F1);	max.append(0x000022F1);	text.append("Swedish Broadcasting Regulator");
	min.append(0x0000233A);	max.append(0x0000233A);	text.append("Independent Television Commission");
	min.append(0x00003200);	max.append(0x0000320f);	text.append("Australian Terrestrial Television Networks");
	min.append(0x00006000);	max.append(0x00006000);	text.append("News Datacom");
	min.append(0x00006001);	max.append(0x00006006);	text.append("NDC");
	min.append(0x00362275);	max.append(0x00362275);	text.append("Irdeto");
	min.append(0x004E544C);	max.append(0x004E544C);	text.append("NTL");
	min.append(0x00532D41);	max.append(0x00532D41);	text.append("Scientific Atlanta");
	min.append(0x00600000);	max.append(0x00600000);	text.append("Rhene Vision Cable");
	min.append(0x44414E59);	max.append(0x44414E59);	text.append("News Datacom (IL) 1");
	min.append(0x46524549);	max.append(0x46524549);	text.append("News Datacom (IL) 1");
	min.append(0x46545600);	max.append(0x46545620);	text.append("FreeTV");
	min.append(0x4A4F4A4F);	max.append(0x4A4F4A4F);	text.append("MSG MediaServices GmbH");
	min.append(0x4F545600);	max.append(0x4F5456ff);	text.append("OpenTV");
	min.append(0x50484900);	max.append(0x504849ff);	text.append("Philips DVS");
	min.append(0x53415053);	max.append(0x53415053);	text.append("Scientific Atlanta");
	min.append(0x5347444E);	max.append(0x5347444E);	text.append("StarGuide Digital Networks");
	min.append(0x56444700);	max.append(0x56444700);	text.append("Via Digital");
	min.append(0xBBBBBBBB);	max.append(0xBBBBBBBB);	text.append("Bertelsmann Broadband Group");
	min.append(0xECCA0001);	max.append(0xECCA0001);	text.append("ECCA (European Cable Communications Association)");
	min.append(0xFCFCFCFC);	max.append(0xFCFCFCFC);	text.append("France Telecom");
}


freq_list::freq_list() {
	ch.clear();
	freq.clear();
}

void freq_list::atsc() {
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

void freq_list::qam() {
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

void freq_list::dvbt() {
	ch.clear();
	freq.clear();
	ch.append(5);
	freq.append(177500);
	ch.append(6);
	freq.append(184500);
	ch.append(7);
	freq.append(191500);
	ch.append(8);
	freq.append(198500);
	ch.append(9);
	freq.append(205500);
	ch.append(10);
	freq.append(212500);
	ch.append(11);
	freq.append(219500);
	ch.append(12);
	freq.append(226500);
	ch.append(21);
	freq.append(474000);
	ch.append(22);
	freq.append(482000);
	ch.append(23);
	freq.append(490000);
	ch.append(24);
	freq.append(498000);
	ch.append(25);
	freq.append(506000);
	ch.append(26);
	freq.append(514000);
	ch.append(27);
	freq.append(522000);
	ch.append(28);
	freq.append(530000);
	ch.append(29);
	freq.append(538000);
	ch.append(30);
	freq.append(546000);
	ch.append(31);
	freq.append(554000);
	ch.append(32);
	freq.append(562000);
	ch.append(33);
	freq.append(570000);
	ch.append(34);
	freq.append(578000);
	ch.append(35);
	freq.append(586000);
	ch.append(36);
	freq.append(594000);
	ch.append(37);
	freq.append(602000);
	ch.append(38);
	freq.append(610000);
	ch.append(39);
	freq.append(618000);
	ch.append(40);
	freq.append(626000);
	ch.append(41);
	freq.append(634000);
	ch.append(42);
	freq.append(642000);
	ch.append(43);
	freq.append(650000);
	ch.append(44);
	freq.append(658000);
	ch.append(45);
	freq.append(666000);
	ch.append(46);
	freq.append(674000);
	ch.append(47);
	freq.append(682000);
	ch.append(48);
	freq.append(690000);
	ch.append(49);
	freq.append(698000);
	ch.append(50);
	freq.append(706000);
	ch.append(51);
	freq.append(714000);
	ch.append(52);
	freq.append(722000);
	ch.append(53);
	freq.append(730000);
	ch.append(54);
	freq.append(738000);
	ch.append(55);
	freq.append(746000);
	ch.append(56);
	freq.append(754000);
	ch.append(57);
	freq.append(762000);
	ch.append(58);
	freq.append(770000);
	ch.append(59);
	freq.append(778000);
	ch.append(60);
	freq.append(786000);
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

bool isVectorATSC(QVector<int> system)
{
	bool ret = false;
	for (int i = 0; i < system.size(); i++) {
		if (isATSC(system.at(i))) {
			ret = true;
		}
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

bool isVectorQAM(QVector<int> system)
{
	bool ret = false;
	for (int i = 0; i < system.size(); i++) {
		if (isQAM(system.at(i))) {
			ret = true;
		}
	}
	return ret;
}

bool isDVBT(int system)
{
	bool ret = false;
	switch(system) {
	case SYS_DVBT:
	case SYS_DVBT2:
		ret = true;
	}
	return ret;
}

bool isVectorDVBT(QVector<int> system)
{
	bool ret = false;
	for (int i = 0; i < system.size(); i++) {
		if (isDVBT(system.at(i))) {
			ret = true;
		}
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

unsigned int setbit(unsigned int var, unsigned int MASK)
{
	return var | MASK;
}
unsigned int unsetbit(unsigned int var, unsigned int MASK)
{
	return var & ~MASK;
}

QString tohex(unsigned long int val, int length)
{
	return QString("0x%1").arg(val, length, 16, QChar('0'));
}

