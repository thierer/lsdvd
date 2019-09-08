#include <string.h>

#include "oxml.h"

static int _xlvl = 0;
char *_xlvl_type[256];

int XMLDEF_(char *name, const char *format, ...) {
	va_list argp;
	XMLINDENT;
	printf("<%s>", name);
	
	va_start(argp, format);
	vprintf(format, argp);
	va_end(argp);
	printf("</%s>\n", name);
	return 0;
}

// int XMLBOX_(char *name) {
//	XMLINDENT;
//	printf("<%s>\n", name);
//	_xlvl++; 
//	_xlvl_type[_xlvl] = malloc(20 * sizeof(char));
//	sprintf(_xlvl_type[_xlvl], "</%s>", name);
//	return 0;
//}

//void XMLRETURN_()  {
//	_xlvl--; 
//	XMLINDENT;
//	printf("%s\n", _xlvl_type[_xlvl+1] );
//	free(_xlvl_type[_xlvl+1]);
//}

void XMLBOX_(char *name) {
	XMLINDENT;
	printf("<%s>\n", name);
	_xlvl++;
	_xlvl_type[_xlvl] = name;
}

void XMLRETURN_()  {
	_xlvl--;
	XMLINDENT;
	printf("</%s>\n", _xlvl_type[_xlvl+1]);
}


void XMLSTART_() {
	printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	XMLBOX("lsdvd");
}

void XMLSTOP_() {
	while(_xlvl) XMLRETURN;
}

char *xml_entities(const char *text) {
        unsigned int i, j, count = 0;
        char *escaped;
  
        for (i=0; i < strlen(text); i++)
                if (strchr("\"'&<>", text[i])) count++;
  
        // at most 5 additional chars per entity
        escaped = (char *)malloc(strlen(text) + count * 5 + 1);
  
        for (i=0, j=0; i < strlen(text); i++)
                switch (text[i]) {
                        case '"':
                                strncpy(escaped + j, "&quot;", 6);
                                j += 6;
                                break;
                        case '\'':
                                strncpy(escaped + j, "&apos;", 6);
                                j += 6;
                                break;
                        case '&':
                                strncpy(escaped + j, "&amp;", 5);
                                j += 5;
                                break;
                        case '<':
                                strncpy(escaped + j, "&lt;", 4);
                                j += 4;
                                break;
                        case '>':
                                strncpy(escaped + j, "&gt;", 4);
                                j += 4;
                                break;
                        default:
                                escaped[j++] = text[i];
                                break;
                }
                
        escaped[j] = '\0';
          
        return escaped;
}

void oxml_print(struct dvd_info *dvd_info) {
	int j, i;
        char *escaped;

	XMLSTART;
        escaped = xml_entities(dvd_info->discinfo.device);
	XMLDEF("device", "%s", escaped);
        free(escaped);
        escaped = xml_entities(dvd_info->discinfo.disc_title);
	XMLDEF("title", "%s", escaped);
        free(escaped);
        escaped = xml_entities(dvd_info->discinfo.disc_alt_title);
	XMLDEF("alt_title", "%s", escaped);
        free(escaped);
	XMLDEF("serial_no", "%s", dvd_info->discinfo.disc_serial_no);
	if (dvd_info->discinfo.disc_id != NULL) {
		XMLDEF("disc_id", "%s", dvd_info->discinfo.disc_id);
	}
	XMLDEF("vmg_id", "%.12s", dvd_info->discinfo.vmg_id);
	XMLDEF("provider_id", "%.32s", dvd_info->discinfo.provider_id);

	for (j=0; j < dvd_info->title_count; j++)
	{
	if ( opt_t == j+1 || opt_t == 0 ) {

	// GENERAL
	if (dvd_info->titles[j].enabled) {

		XMLBOX("track");
		XMLDEF("ix", "%d", j+1);
		XMLDEF("length", "%.3f", dvd_info->titles[j].general.length);
		XMLDEF("vts_id", "%.12s", dvd_info->titles[j].general.vts_id);

		if (dvd_info->titles[j].parameter.format != NULL ) {
			XMLDEF("vts", "%d", dvd_info->titles[j].parameter.vts);
			XMLDEF("ttn", "%d", dvd_info->titles[j].parameter.ttn);
			XMLDEF("fps", "%.2f", dvd_info->titles[j].parameter.fps);
			XMLDEF("format", "%s", dvd_info->titles[j].parameter.format);
			XMLDEF("aspect", "%s", dvd_info->titles[j].parameter.aspect);
			XMLDEF("width", "%s", dvd_info->titles[j].parameter.width);
			XMLDEF("height", "%s", dvd_info->titles[j].parameter.height);
                        escaped = xml_entities(dvd_info->titles[j].parameter.df);
			XMLDEF("df", "%s", escaped);
                        free(escaped);
		}
		
		// PALETTE
		if (dvd_info->titles[j].palette != NULL ) {
			XMLBOX("palette");
			for (i=0; i < 16; i++) {
				XMLDEF("color","%06x", dvd_info->titles[j].palette[i]);
			}
			XMLRETURN;
		}

		// ANGLES
		if (dvd_info->titles[j].angle_count) { // poor check, but there's no other info anyway.
			XMLDEF("angles", "%d", dvd_info->titles[j].angle_count);
		}

		// AUDIO
		if (dvd_info->titles[j].audiostreams != NULL ) {
			for (i=0; i<dvd_info->titles[j].audiostream_count; i++)
			{
				XMLBOX("audio");
				XMLDEF("ix", "%d", i+1);
				XMLDEF("langcode", "%s", dvd_info->titles[j].audiostreams[i].langcode);
				XMLDEF("language", "%s", dvd_info->titles[j].audiostreams[i].language);
				XMLDEF("format", "%s", dvd_info->titles[j].audiostreams[i].format);
				XMLDEF("frequency", "%s", dvd_info->titles[j].audiostreams[i].frequency);
				XMLDEF("quantization", "%s", dvd_info->titles[j].audiostreams[i].quantization);
				XMLDEF("channels", "%d", dvd_info->titles[j].audiostreams[i].channels);
				XMLDEF("ap_mode", "%d", dvd_info->titles[j].audiostreams[i].ap_mode);
				XMLDEF("content", "%s", dvd_info->titles[j].audiostreams[i].content);
				XMLDEF("streamid", "0x%x", dvd_info->titles[j].audiostreams[i].streamid);
				XMLRETURN;
			}
		}

		// CHAPTERS
		if (dvd_info->titles[j].chapters != NULL) {
			for (i=0; i<dvd_info->titles[j].chapter_count; i++)
			{
				XMLBOX("chapter");
				XMLDEF("ix", "%d", i+1);
				XMLDEF("length", "%.3f", dvd_info->titles[j].chapters[i].length);
				XMLDEF("startcell", "%d", dvd_info->titles[j].chapters[i].startcell);
				XMLRETURN;
			}
		}

		// CELLS
		if (dvd_info->titles[j].cells != NULL) {
			for (i=0; i<dvd_info->titles[j].cell_count; i++)
			{
				XMLBOX("cell");
				XMLDEF("ix", "%d", i+1);
				XMLDEF("length", "%.3f", dvd_info->titles[j].cells[i].length);
				XMLDEF("block_mode", "%d", dvd_info->titles[j].cells[i].block_mode);
				XMLDEF("block_type", "%d", dvd_info->titles[j].cells[i].block_type);
				XMLRETURN;
			}
		}

		// SUBTITLES
		if (dvd_info->titles[j].subtitles != NULL) {
			for (i=0; i<dvd_info->titles[j].subtitle_count; i++)
			{
				XMLBOX("subp");
				XMLDEF("ix", "%d", i+1);
				XMLDEF("langcode", "%s", dvd_info->titles[j].subtitles[i].langcode);
				XMLDEF("language", "%s", dvd_info->titles[j].subtitles[i].language);
				XMLDEF("content", "%s", dvd_info->titles[j].subtitles[i].content);
				XMLDEF("streamid", "0x%x", dvd_info->titles[j].subtitles[i].streamid);
				XMLRETURN;
			}
		}
	XMLRETURN;	
	}
	}
	}

	if (! opt_t) {
		XMLDEF("longest_track", "%d", dvd_info->longest_track);
	}
	XMLSTOP;
}
