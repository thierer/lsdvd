/*
 *  lsdvd.c
 *
 *  DVD info lister
 *
 *  Copyright (C) 2003  EFF
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation;
 *
 *  2003	by Chris Phillips
 *  2003-04-19  Cleanups get_title_name, added dvdtime2msec, added helper macros,
 *			  output info structures in form of a Perl module, by Henk Vergonet.
 */
#include <dvdread/ifo_read.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "lsdvd.h"
#include "ocode.h"

static struct { char code[3];	char name[20];}
language[] = {
	{ "  ", "Not Specified" }, { "aa", "Afar" },	{ "ab", "Abkhazian" }, { "af", "Afrikaans" },	{ "am", "Amharic" },
	{ "ar", "Arabic" }, { "as", "Assamese" },	{ "ay", "Aymara" }, { "az", "Azerbaijani" }, { "ba", "Bashkir" },
	{ "be", "Byelorussian" }, { "bg", "Bulgarian" }, { "bh", "Bihari" }, { "bi", "Bislama" }, { "bn", "Bengali; Bangla" },
	{ "bo", "Tibetan" }, { "br", "Breton" }, { "ca", "Catalan" }, { "co", "Corsican" }, { "cs", "Czech" },
	{ "cy", "Welsh" }, { "da", "Dansk" }, { "de", "Deutsch" }, { "dz", "Bhutani" }, { "el", "Greek" }, { "en", "English" },
	{ "eo", "Esperanto" }, { "es", "Espanol" }, { "et", "Estonian" }, { "eu", "Basque" }, { "fa", "Persian" },
	{ "fi", "Suomi" }, { "fj", "Fiji" }, { "fo", "Faroese" }, { "fr", "Francais" }, { "fy", "Frisian" }, { "ga", "Gaelic" },
	{ "gd", "Scots Gaelic" }, { "gl", "Galician" }, { "gn", "Guarani" }, { "gu", "Gujarati" }, { "ha", "Hausa" },
	{ "he", "Hebrew" }, { "hi", "Hindi" }, { "hr", "Hrvatski" }, { "hu", "Magyar" }, { "hy", "Armenian" },
	{ "ia", "Interlingua" }, { "id", "Indonesian" }, { "ie", "Interlingue" }, { "ik", "Inupiak" }, { "in", "Indonesian" },
	{ "is", "Islenska" }, { "it", "Italiano" }, { "iu", "Inuktitut" }, { "iw", "Hebrew" }, { "ja", "Japanese" },
	{ "ji", "Yiddish" }, { "jw", "Javanese" }, { "ka", "Georgian" }, { "kk", "Kazakh" }, { "kl", "Greenlandic" },
	{ "km", "Cambodian" }, { "kn", "Kannada" }, { "ko", "Korean" }, { "ks", "Kashmiri" }, { "ku", "Kurdish" },
	{ "ky", "Kirghiz" }, { "la", "Latin" }, { "ln", "Lingala" }, { "lo", "Laothian" }, { "lt", "Lithuanian" },
	{ "lv", "Latvian, Lettish" }, { "mg", "Malagasy" }, { "mi", "Maori" }, { "mk", "Macedonian" }, { "ml", "Malayalam" },
	{ "mn", "Mongolian" }, { "mo", "Moldavian" }, { "mr", "Marathi" }, { "ms", "Malay" }, { "mt", "Maltese" },
	{ "my", "Burmese" }, { "na", "Nauru" }, { "ne", "Nepali" }, { "nl", "Nederlands" }, { "no", "Norsk" }, { "oc", "Occitan" },
	{ "om", "Oromo" }, { "or", "Oriya" }, { "pa", "Punjabi" }, { "pl", "Polish" }, { "ps", "Pashto, Pushto" },
	{ "pt", "Portugues" }, { "qu", "Quechua" }, { "rm", "Rhaeto-Romance" }, { "rn", "Kirundi" }, { "ro", "Romanian"  },
	{ "ru", "Russian" }, { "rw", "Kinyarwanda" }, { "sa", "Sanskrit" }, { "sd", "Sindhi" }, { "sg", "Sangho" },
	{ "sh", "Serbo-Croatian" }, { "si", "Sinhalese" }, { "sk", "Slovak" }, { "sl", "Slovenian" }, { "sm", "Samoan" },
 	{ "sn", "Shona"  }, { "so", "Somali" }, { "sq", "Albanian" }, { "sr", "Serbian" }, { "ss", "Siswati" },
	{ "st", "Sesotho" }, { "su", "Sundanese" }, { "sv", "Svenska" }, { "sw", "Swahili" }, { "ta", "Tamil" },
	{ "te", "Telugu" }, { "tg", "Tajik" }, { "th", "Thai" }, { "ti", "Tigrinya" }, { "tk", "Turkmen" }, { "tl", "Tagalog" },
	{ "tn", "Setswana" }, { "to", "Tonga" }, { "tr", "Turkish" }, { "ts", "Tsonga" }, { "tt", "Tatar" }, { "tw", "Twi" },
	{ "ug", "Uighur" }, { "uk", "Ukrainian" }, { "ur", "Urdu" }, { "uz", "Uzbek" }, { "vi", "Vietnamese" },
	{ "vo", "Volapuk" }, { "wo", "Wolof" }, { "xh", "Xhosa" }, { "yi", "Yiddish" }, { "yo", "Yoruba" }, { "za", "Zhuang" },
	{ "zh", "Chinese" }, { "zu", "Zulu" }, { "xx", "Unknown" }, { "\0", "Unknown" } };
char *video_format[2] = {"NTSC", "PAL"};
/* 28.9.2003: Chicken run's aspect ratio is 16:9 or 1.85:1, at index
   1.  Addionaly using ' in the quoting makes the perl output not
   parse so changed to " */
char *aspect_ratio[4] = {"4/3", "16/9", "\"?:?\"", "16/9"};
char *quantization[4] = {"16bit", "20bit", "24bit", "drc"};
char *mpeg_version[2] = {"mpeg1", "mpeg2"};
/* 28.9.2003: The European chicken run title has height index 3, and
   576 lines seems right, mplayer -vop cropdetect shows from 552 to
   576 lines.  What the correct value is for index 2 is harder to say */
char *video_height[4] = {"480", "576", "???", "576"};
char *video_width[4]  = {"720", "704", "352", "352"};
char *permitted_df[4] = {"P&S + Letter", "Pan&Scan", "Letterbox", "?"};
char *audio_format[7] = {"ac3", "?", "mpeg1", "mpeg2", "lpcm", "sdds", "dts"};
int   audio_id[7]     = {0x80, 0, 0xC0, 0xC0, 0xA0, 0, 0x88};
/* 28.9.2003: Chicken run again, it has frequency index of 1.
   According to dvd::rip the frequency is 48000 */
char *sample_freq[2]  = {"48000", "48000"};
char *audio_type[5]   = {"Undefined", "Normal", "Impaired", "Comments1", "Comments2"};
char *subp_type[16]   = {"Undefined", "Normal", "Large", "Children", "reserved", "Normal_CC", "Large_CC", "Children_CC",
	"reserved", "Forced", "reserved", "reserved", "reserved", "Director", "Large_Director", "Children_Director"};
int   subp_id_shift[4] = {24, 8, 8, 8};
double frames_per_s[4] = {-1.0, 25.00, -1.0, 29.97};

char* program_name;

//extern void operl_print(struct dvd_info *dvd_info);
//extern void oxml_print(struct dvd_info *dvd_info);
//extern void oruby_print(struct dvd_info *dvd_info);
//extern void ohuman_print(struct dvd_info *dvd_info);

int dvdtime2msec(dvd_time_t *dt)
{
	double fps = frames_per_s[(dt->frame_u & 0xc0) >> 6];
	long   ms;
	ms  = (((dt->hour &   0xf0) >> 3) * 5 + (dt->hour   & 0x0f)) * 3600000;
	ms += (((dt->minute & 0xf0) >> 3) * 5 + (dt->minute & 0x0f)) * 60000;
	ms += (((dt->second & 0xf0) >> 3) * 5 + (dt->second & 0x0f)) * 1000;

	if(fps > 0)
	ms += (((dt->frame_u & 0x30) >> 3) * 5 + (dt->frame_u & 0x0f)) * 1000.0 / fps;

	return ms;
}

/*
 * This is used to add up sets of times in the struct. it's not elegant at all
 * but a quick way to easily add up 4 times at once. tracking the times in usec's 
 * constantly is easier, but without using math.h, it sucks to actually DO anything with it
 * also it ***has*** to be better to return the playback_time, not just mess with it like this
 */
void converttime(playback_time_t *pt, dvd_time_t *dt)
{
	double fps = frames_per_s[(dt->frame_u & 0xc0) >> 6];

	pt->usec = pt->usec + (((dt->frame_u & 0x30) >> 3) * 5 + (dt->frame_u & 0x0f)) * 1000.0 / fps;
	pt->second = pt->second + ((dt->second & 0xf0) >> 3) * 5 + (dt->second & 0x0f);
	pt->minute = pt->minute + ((dt->minute & 0xf0) >> 3) * 5 + (dt->minute & 0x0f);
	pt->hour = pt->hour + ((dt->hour &   0xf0) >> 3) * 5 + (dt->hour   & 0x0f);

	if ( pt->usec >= 1000 ) { pt->usec -= 1000; pt->second++; }
	if ( pt->second >= 60 ) { pt->second -= 60; pt->minute++; }
	if ( pt->minute > 59 ) { pt->minute -= 60; pt->hour++; }
}

/*
 *  The following method is based on code from vobcopy, by Robos, with thanks.
 *  Modified to also read serial number and alternative title based on
 *  libdvdnav's src/vm/vm.c
 */
int get_title_info(const char* dvd_device, char* title, char* serial_no, char* alt_title)
{
	FILE *filehandle = 0;
        char buffer[2048];
	int  i;

        title[0] = '\0';
        serial_no[0] = '\0';
        alt_title[0] = '\0';

        if (! (filehandle = fopen(dvd_device, "r"))) {
		fprintf(stderr, "Couldn't open %s for title\n", dvd_device);
		return -1;
	}

	if ( fseek(filehandle, 65536, SEEK_SET )) {
		fclose(filehandle);
		fprintf(stderr, "Couldn't seek in %s for title\n", dvd_device);
		return -1;
	}

	if ( 2048 != fread(buffer, 1, 2048, filehandle) ) {
		fclose(filehandle);
		fprintf(stderr, "Couldn't read enough bytes for title.\n");
		return -1;
	}
	fclose (filehandle);

        for (i=25; i < 73;i++)
              title[i - 25] = isprint(buffer[i]) ? buffer[i] : ' ';
        title[48] = '\0';
        for (i=47; i >= 0;i--)
              if (title[i] == ' ') title[i] = '\0'; else break;

        for (i=73; i < 81;i++)
          serial_no[i - 73] = isprint(buffer[i]) ? buffer[i] : ' ';
        serial_no[8] = '\0';
        for (i=7; i >= 0;i--)
              if (serial_no[i] == ' ') serial_no[i] = '\0'; else break;

        for (i=89; i < 128;i++)
          alt_title[i - 89] = isprint(buffer[i]) ? buffer[i] : ' ';
        alt_title[39] = '\0';
        for (i=38; i >= 0;i--)
              if (alt_title[i] == ' ') alt_title[i] = '\0'; else break;

	return 0;
}

char *get_disc_id(dvd_reader_t *dvd) {
	unsigned char buf[16];
	char *hex;
	int i;

	if (DVDDiscID(dvd, buf) == -1) return NULL;

	hex = (char *)malloc(33);

	for (i = 0; i < 16; i++) {
		sprintf(hex + 2 * i, "%02x", buf[i]);
	}

	return hex;
}

char* lang_name(char* code)
{
	int k=0;
	while (memcmp(language[k].code, code, 2) && language[k].name[0] ) { k++; }
	return language[k].name;
}

void version()
{
	fprintf(stderr, "lsdvd "VERSION" - GPL Copyright (c) 2002, 2003, 2004, 2005 \"Written\" by Chris Phillips <acid_kewpie@users.sf.net>\n");
}

void usage()
{
	version();
	fprintf(stderr, "Usage: %s [options] [-t track_number] [dvd path] \n", program_name);
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\tExtra information:\n");
	fprintf(stderr, "\t  -a audio streams\n");
	fprintf(stderr, "\t  -d cells\n");
	fprintf(stderr, "\t  -n angles\n");
	fprintf(stderr, "\t  -c chapters\n");
	fprintf(stderr, "\t  -s subpictures\n");
	fprintf(stderr, "\t  -P palette\n");
	fprintf(stderr, "\t  -v video\n");
	fprintf(stderr, "\t  -x all information\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\tFormatting:\n");
	fprintf(stderr, "\t  -Oh output as human readable (default)\n");
	fprintf(stderr, "\t  -Op output as Perl\n");
	fprintf(stderr, "\t  -Oy output as Python\n");
	fprintf(stderr, "\t  -Or output as Ruby\n");
	fprintf(stderr, "\t  -Ox output as XML\n");
	fprintf(stderr, "\t  -Oj output as JSON\n");
	//fprintf(stderr, "\t  -p output as Perl [deprecated]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\tOther options:\n");
	fprintf(stderr, "\t  -q quiet - no summary totals\n");
	fprintf(stderr, "\t  -h this message\n");
	fprintf(stderr, "\t  -V version information\n");
	fprintf(stderr, "\n");
}

int opt_a=0, opt_c=0, opt_n=0, opt_p=0, opt_q=0, opt_s=0, opt_t=0, opt_v=0, opt_x=0, opt_d=0, opt_P=0;
char opt_O='h';

char output_option(char *arg)
{
	if (strlen(arg) == 1) {
		return arg[0];
	} else if (strcmp(arg, "perl") == 0) {
		return 'p';
	} else if (strcmp(arg, "python") == 0) {
		return 'y';
	} else if (strcmp(arg, "ruby") == 0) {
		return 'r';
	} else if (strcmp(arg, "ruby2") == 0) {
		return '2';
	} else if (strcmp(arg, "xml") == 0) {
		return 'x';
	} else if (strcmp(arg, "human") == 0) {
		return 'h';
	} else if (strcmp(arg, "json") == 0) {
		return 'j';
	} else {
		return '\0';
	}
}
										

int main(int argc, char *argv[])
{
	char title[49], serial_no[9], alt_title[40];
	dvd_reader_t *dvd;
	ifo_handle_t *ifo_zero, **ifo;
	pgcit_t *vts_pgcit;
	vtsi_mat_t *vtsi_mat;
	vts_ptt_srpt_t *vts_ptt_srpt;
	vmgi_mat_t *vmgi_mat;
	audio_attr_t *audio_attr;
	video_attr_t *video_attr;
	subp_attr_t *subp_attr;
	pgc_t *pgc;
	int i, j, k, c, titles, cell, vts_ttn, title_set_nr;
 	char lang_code[3];
	char *dvd_device = "/dev/dvd";
	int ret = 0;
	int max_length = 0, max_track = 0;
	struct stat dvd_stat;

	program_name = argv[0];

	while ((c = getopt(argc, argv, "acnpPqsdvt:O:xhV?")) != EOF) {
		switch (c) {
		case 'h':
		case '?':	usage();		return 0;
		case 'V':	version();		return 0;
		case 'a':	opt_a = 1;		break;
		case 'd':	opt_d = 1;		break;
		case 's':	opt_s = 1;		break;
		case 'q':	opt_q = 1;		break;
		case 'c':	opt_c = 1;		break;
		case 'n':	opt_n = 1;		break;
		case 'p':	opt_p = 1;		break;
		case 'P':	opt_P = 1;		break;
		case 't':	opt_t = atoi(optarg);	break;
		case 'O':	opt_O = output_option(optarg);	break;
		case 'v':	opt_v = 1;		break;
		case 'x':	opt_x = 1;
				opt_a = 1;
				opt_c = 1;
				opt_s = 1;
				opt_P = 1;
				opt_d = 1;
				opt_n = 1;
				opt_v = 1;		break;
		}
	}

	if (argv[optind]) { dvd_device = argv[optind];	}
	
	ret = stat(dvd_device, &dvd_stat);
	if ( ret < 0 ) {
		fprintf(stderr, "Can't find device %s\n", dvd_device);
		return 1;
	}
	
	dvd = DVDOpen(dvd_device);
	if( !dvd ) {
		fprintf( stderr, "Can't open disc %s!\n", dvd_device);
		return 2;
	}
	ifo_zero = ifoOpen(dvd, 0);
	if ( !ifo_zero ) {
		fprintf( stderr, "Can't open main ifo!\n");
		return 3;
	}

	ifo = (ifo_handle_t **)malloc((ifo_zero->vts_atrt->nr_of_vtss + 1) * sizeof(ifo_handle_t *));

	for (i=1; i <= ifo_zero->vts_atrt->nr_of_vtss; i++) {
		ifo[i] = ifoOpen(dvd, i);
		if ( !ifo[i] && opt_t == i ) {
			fprintf( stderr, "Can't open ifo %d!\n", i);
			return 4;
		}
	}

	titles = ifo_zero->tt_srpt->nr_of_srpts;

	if ( opt_t > titles || opt_t < 0) {
		fprintf (stderr, "Only %d titles on this disc!", titles);
		return 5;
	}

	get_title_info(dvd_device, title, serial_no, alt_title);

	vmgi_mat = ifo_zero->vmgi_mat;

	struct dvd_info dvd_info;
		
	dvd_info.discinfo.device = dvd_device;
	dvd_info.discinfo.disc_title = title[0] ? title : "unknown";
        dvd_info.discinfo.disc_serial_no = serial_no[0] ? serial_no : "unknown";
        dvd_info.discinfo.disc_alt_title = alt_title[0] ? alt_title : "unknown";
	dvd_info.discinfo.disc_id = get_disc_id(dvd);
	dvd_info.discinfo.vmg_id =  vmgi_mat->vmg_identifier;
	dvd_info.discinfo.provider_id = vmgi_mat->provider_identifier;
	
	dvd_info.title_count = titles;
	dvd_info.titles = calloc(titles, sizeof(*dvd_info.titles));

	for (j=0; j < titles; j++)
	{

		if (opt_t != 0 && opt_t != j+1) {
			continue;
		}

		title_set_nr = ifo_zero->tt_srpt->title[j].title_set_nr;
		if (ifo[title_set_nr] == NULL) {
			// Ignore illegal IFOs
			continue;
		}

		// GENERAL
		vtsi_mat     = ifo[title_set_nr]->vtsi_mat;
		vts_pgcit    = ifo[title_set_nr]->vts_pgcit;
		vts_ptt_srpt = ifo[title_set_nr]->vts_ptt_srpt;
		if (vtsi_mat == NULL || vts_pgcit == NULL || vts_ptt_srpt == NULL) {
			continue;
		}

		video_attr = &vtsi_mat->vts_video_attr;
		vts_ttn = ifo_zero->tt_srpt->title[j].vts_ttn;
		pgc = vts_pgcit->pgci_srp[vts_ptt_srpt->title[vts_ttn - 1].ptt[0].pgcn - 1].pgc;

		if (pgc == NULL || pgc->cell_playback == NULL || pgc->program_map == NULL) {
			// Ignore illegal title
			continue;
		}

		dvd_info.titles[j].enabled = 1;

		dvd_info.titles[j].general.length = dvdtime2msec(&pgc->playback_time)/1000.0;
		converttime(&dvd_info.titles[j].general.playback_time, &pgc->playback_time);
		dvd_info.titles[j].general.vts_id = vtsi_mat->vts_identifier;
				
		if (dvdtime2msec(&pgc->playback_time) > max_length) {
			max_length = dvdtime2msec(&pgc->playback_time);
			max_track = j+1;
		}

		dvd_info.titles[j].chapter_count_reported = ifo_zero->tt_srpt->title[j].nr_of_ptts;
		dvd_info.titles[j].cell_count = pgc->nr_of_cells;

		dvd_info.titles[j].audiostream_count = 0;
                for (k=0; k < 8; k++)
                  if (pgc->audio_control[k] & 0x8000) 
                    dvd_info.titles[j].audiostream_count++;

                dvd_info.titles[j].subtitle_count = 0;
                for (k=0; k < 32; k++)
                  if (pgc->subp_control[k] & 0x80000000) 
                    dvd_info.titles[j].subtitle_count++;

		if(opt_v) {
			dvd_info.titles[j].parameter.vts = title_set_nr;
			dvd_info.titles[j].parameter.ttn = ifo_zero->tt_srpt->title[j].vts_ttn;
			dvd_info.titles[j].parameter.fps = frames_per_s[(pgc->playback_time.frame_u & 0xc0) >> 6];
			dvd_info.titles[j].parameter.format = video_format[video_attr->video_format];
			dvd_info.titles[j].parameter.aspect = aspect_ratio[video_attr->display_aspect_ratio];
			dvd_info.titles[j].parameter.width = video_width[video_attr->picture_size];
			dvd_info.titles[j].parameter.height = video_height[video_attr->video_format];
			dvd_info.titles[j].parameter.df = permitted_df[video_attr->permitted_df];

		}

		// PALETTE
		if (opt_P) {
			dvd_info.titles[j].palette = malloc(16 * sizeof(int));
        	        for (i=0; i < 16; i++) { dvd_info.titles[j].palette[i] = pgc->palette[i]; }
		} else {
			dvd_info.titles[j].palette = NULL;
		}

		// ANGLES

		if (opt_n) {
			dvd_info.titles[j].angle_count = ifo_zero->tt_srpt->title[j].nr_of_angles;
		} else {
			dvd_info.titles[j].angle_count = 0;
		}

		// AUDIO

		if (opt_a) {

			dvd_info.titles[j].audiostreams = calloc(dvd_info.titles[j].audiostream_count, sizeof(*dvd_info.titles[j].audiostreams));

			for (i=0, k=0; i<8; i++)
			{
                                if ((pgc->audio_control[i] & 0x8000) == 0) continue;

                                audio_attr = &vtsi_mat->vts_audio_attr[i];
				sprintf(lang_code, "%c%c", audio_attr->lang_code>>8, audio_attr->lang_code & 0xff);
				if (!isalpha(lang_code[0]) || !isalpha(lang_code[1])) { lang_code[0] = 'x'; lang_code[1] = 'x'; }

				dvd_info.titles[j].audiostreams[k].langcode = strdup(lang_code);
				dvd_info.titles[j].audiostreams[k].language = lang_name(lang_code);
				dvd_info.titles[j].audiostreams[k].format = audio_format[audio_attr->audio_format];
				dvd_info.titles[j].audiostreams[k].frequency = sample_freq[audio_attr->sample_frequency];
				dvd_info.titles[j].audiostreams[k].quantization = quantization[audio_attr->quantization];
				dvd_info.titles[j].audiostreams[k].channels = audio_attr->channels+1;
				dvd_info.titles[j].audiostreams[k].ap_mode = audio_attr->application_mode;
				dvd_info.titles[j].audiostreams[k].content = audio_type[audio_attr->lang_extension];
				dvd_info.titles[j].audiostreams[k].streamid = audio_id[audio_attr->audio_format] + (pgc->audio_control[i] >> 8 & 7);
                                k++;
			}
		} else {
			dvd_info.titles[j].audiostreams = NULL;
		}

		// CHAPTERS

		cell = 0;
		if (opt_c) {
			dvd_info.titles[j].chapter_count = pgc->nr_of_programs;
			dvd_info.titles[j].chapters = calloc(dvd_info.titles[j].chapter_count, sizeof(*dvd_info.titles[j].chapters));

						int ms;
			for (i=0; i<pgc->nr_of_programs; i++)
			{	   
				ms=0;
				int next = pgc->program_map[i+1];   
				if (i == pgc->nr_of_programs - 1) next = pgc->nr_of_cells + 1;

				while (cell < next - 1)
				{
                                        // Only use first cell of multi-angle cells
                                        if (pgc->cell_playback[cell].block_mode <= 1)
                                        {
                                                ms = ms + dvdtime2msec(&pgc->cell_playback[cell].playback_time);
                                                converttime(&dvd_info.titles[j].chapters[i].playback_time, &pgc->cell_playback[cell].playback_time);
                                        }
					cell++;
				}
				dvd_info.titles[j].chapters[i].startcell = pgc->program_map[i];
				dvd_info.titles[j].chapters[i].length = ms * 0.001;


			}
		}

		// CELLS


		dvd_info.titles[j].cells = calloc(dvd_info.titles[j].cell_count, sizeof(*dvd_info.titles[j].cells));

		if (opt_d) {
			for (i=0; i<pgc->nr_of_cells; i++)
			{
				dvd_info.titles[j].cells[i].length = dvdtime2msec(&pgc->cell_playback[i].playback_time)/1000.0;
                                dvd_info.titles[j].cells[i].block_mode = pgc->cell_playback[i].block_mode;
                                dvd_info.titles[j].cells[i].block_type = pgc->cell_playback[i].block_type;
                                converttime(&dvd_info.titles[j].cells[i].playback_time, &pgc->cell_playback[i].playback_time);
			}
		} else {
			dvd_info.titles[j].cells = NULL;
		}

		// SUBTITLES

		dvd_info.titles[j].subtitles = calloc(dvd_info.titles[j].subtitle_count, sizeof(*dvd_info.titles[j].subtitles));

		if (opt_s) {
			for (i=0, k=0; i<32; i++)
			{
                                if ((pgc->subp_control[i] & 0x80000000) == 0) continue;
                          
				subp_attr = &vtsi_mat->vts_subp_attr[i];
				sprintf(lang_code, "%c%c", subp_attr->lang_code>>8, subp_attr->lang_code & 0xff);
				if (!isalpha(lang_code[0]) || !isalpha(lang_code[1])) { lang_code[0] = 'x'; lang_code[1] = 'x'; }
				
				dvd_info.titles[j].subtitles[k].langcode = strdup(lang_code);
				dvd_info.titles[j].subtitles[k].language = lang_name(lang_code);
				dvd_info.titles[j].subtitles[k].content = subp_type[subp_attr->lang_extension];
				dvd_info.titles[j].subtitles[k].streamid = 0x20 + ((pgc->subp_control[i] >> subp_id_shift[video_attr->display_aspect_ratio]) & 0x1f);
                                k++;
			}
		} else {
			dvd_info.titles[j].subtitles = NULL;
		}
	} // for each title
	
	if (! opt_t) { dvd_info.longest_track = max_track; }

	if (opt_p) {
		ocode_print(&perl_syntax, &dvd_info);
	} else {
		switch(opt_O) {
			case 'p':	
				ocode_print(&perl_syntax, &dvd_info);		
				break;
                        case 'y':
			       ocode_print(&python_syntax, &dvd_info);           
				break;
			case 'x':
				oxml_print(&dvd_info);		
				break;
			case 'r':
				ocode_print(&ruby_syntax, &dvd_info);		
				break;
                        case 'd':
				ocode_print(&debug_syntax, &dvd_info);           
				break;
			case 'j':
				ocode_print(&json_syntax, &dvd_info);
				break;
			default :
				ohuman_print(&dvd_info);	
				break;
		}
	}

	for (i=1; i <= ifo_zero->vts_atrt->nr_of_vtss; i++) { ifoClose(ifo[i]);	}

	ifoClose(ifo_zero);
	DVDClose(dvd);

	return 0;
}
