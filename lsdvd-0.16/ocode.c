#include "ocode.h"

/*
   Simple helper macros for generating Perl structures
*/
#define INDENT  { int i; for(i=0; i<_lvl; i++) printf(syntax->indent); }
#define DEF DEF_
#define ADEF ADEF_
#define HASH HASH_
#define ARRAY ARRAY_
#define RETURN RETURN_()
#define START START_()
#define STOP STOP_()

/* This syntax table is not used, but is helpful as a debugging aid. */
struct Syntax debug_syntax = {
        "<indent/>",
        "<def>%s</def>",
        "<def_sep/>",
        "<hash_outer>%s</hash_outer>",
        "<hash_inner>%s</hash_inner>",
        "<hash_anon/>",
        "<array_outer>%s</array_outer>",
        "<array_inner>%s</array_inner>",
        "<adef_sep/>",
        "</return_hash_outer>",
        "</return_array_outer>",
        "</return_hash_inner>",
        "</return_array_inner>%s"
};

struct Syntax perl_syntax = {
        "  ",
        "%s => ",
        ",\n",
        "our %%%s = (\n",
        "%s => {\n",
        "{\n",
        "our @%s = (\n",
        "%s => [\n",
        ",\n",
        ");\n",
        ");\n",
        "},\n",
        "],\n"
};

struct Syntax python_syntax = {
        "  ",
        "'%s' : ",
        ",\n",
        "%s = {\n",
        "'%s' : {\n",
        "{\n",
        "%s = [\n",
        "'%s' : [\n",
        ",\n",
        "}\n",
        "]\n",
        "},\n",
        "],\n"
};


/* This syntax table is not used, but is included here as a starting point
   for somebody who understands Ruby syntax.  For the values that I am not
   certain of, I left some xml-like values. */
struct Syntax ruby_syntax = {
        "  ",
        ":%s => ",
        ",\n",
        "{\n",
        "<hash_inner>%s</hash_inner>",
        "{\n",
        "<array_outer>%s</array_outer>",
        ":%s => [",
        ",\n",
        "}\n",
        "<return_array_outer/>",
        "},\n",
        "],\n"
};


static int _lvl = 0;
static char *_lvl_return[256];
static struct Syntax *syntax = &perl_syntax;


static void DEF_(char *name, const char *format, ...) {
        va_list argp;
        INDENT;
        printf(syntax->def, name);

        va_start(argp, format);
        vprintf(format, argp);
        va_end(argp);
        printf(syntax->def_sep);
}

static void HASH_(char *name) {
        INDENT;
        if (name) {
                printf((_lvl ? syntax->hash_inner : syntax->hash_outer), name);
        } else {
                printf(syntax->hash_anon);
        }
        _lvl_return[_lvl] = _lvl ? syntax->return_hash_inner : syntax->return_hash_outer;
        ++_lvl;
}

static void ARRAY_(char *name) {
        INDENT;
        if (_lvl) {
		printf(syntax->array_inner, name);
        } else {
                printf(syntax->array_outer, name);
        }
	_lvl_return[_lvl] = _lvl ? syntax->return_array_inner : syntax->return_array_outer;
        ++_lvl;
}

static void ADEF_(const char *format, ...) {
        va_list argp;
        INDENT;
        va_start(argp, format);
        vprintf(format, argp);
        va_end(argp);
        printf(syntax->adef_sep);
}

static void RETURN_()  {
        _lvl--;
        INDENT;
        printf(_lvl_return[_lvl]);
}

static void START_() {
        HASH("lsdvd");
}

static void STOP_() {
        while(_lvl) RETURN;
}


void ocode_print(struct Syntax *syntax_, struct dvd_info *dvd_info) {
        int j, i;

        syntax = syntax_;

        START;
        DEF("device", "'%s'", dvd_info->discinfo.device);
        DEF("title", "'%s'", dvd_info->discinfo.disc_title);
        DEF("alt_title", "'%s'", dvd_info->discinfo.disc_alt_title);
        DEF("serial_no", "'%s'", dvd_info->discinfo.disc_serial_no);
        if (dvd_info->discinfo.disc_id != NULL) {
                DEF("disc_id", "'%s'", dvd_info->discinfo.disc_id);
        }
        DEF("vmg_id", "'%.12s'", dvd_info->discinfo.vmg_id);
        DEF("provider_id", "'%.32s'", dvd_info->discinfo.provider_id);

        /* This should probably be "tracks": */
        ARRAY("track");

        for (j=0; j < dvd_info->title_count; j++)
        {
        if ( opt_t == j+1 || opt_t == 0 ) {

        // GENERAL
        if (dvd_info->titles[j].enabled) {

                HASH(0);
                DEF("ix", "%d", j+1);
                DEF("length", "%.3f", dvd_info->titles[j].general.length);
                DEF("vts_id", "'%.12s'", dvd_info->titles[j].general.vts_id);

                if (dvd_info->titles[j].parameter.format != NULL ) {
                        DEF("vts", "%d", dvd_info->titles[j].parameter.vts);
                        DEF("ttn", "%d", dvd_info->titles[j].parameter.ttn);
                        DEF("fps", "%.2f", dvd_info->titles[j].parameter.fps);
                        DEF("format", "'%s'", dvd_info->titles[j].parameter.format);
                        DEF("aspect", "'%s'", dvd_info->titles[j].parameter.aspect);
                        DEF("width", "%s", dvd_info->titles[j].parameter.width);
                        DEF("height", "%s", dvd_info->titles[j].parameter.height);
                        DEF("df", "'%s'", dvd_info->titles[j].parameter.df);
                }

                // PALETTE
                if (dvd_info->titles[j].palette != NULL) {
                        ARRAY("palette");
                        for (i=0; i < 16; i++) {
                                ADEF("'%06x'",  dvd_info->titles[j].palette[i]);
                        }
                        RETURN;
                }

                // ANGLES
                if (dvd_info->titles[j].angle_count) { // poor check, but there's no other info anyway.
                        DEF("angles", "%d", dvd_info->titles[j].angle_count);
                }

                // AUDIO
                if (dvd_info->titles[j].audiostreams != NULL ) {
                        ARRAY("audio");
                        for (i=0; i<dvd_info->titles[j].audiostream_count; i++)
                        {
                                HASH(0);
                                DEF("ix", "%d", i+1);
                                DEF("langcode", "'%s'", dvd_info->titles[j].audiostreams[i].langcode);
                                DEF("language", "'%s'", dvd_info->titles[j].audiostreams[i].language);
                                DEF("format", "'%s'", dvd_info->titles[j].audiostreams[i].format);
                                DEF("frequency", "%s", dvd_info->titles[j].audiostreams[i].frequency);
                                DEF("quantization", "'%s'", dvd_info->titles[j].audiostreams[i].quantization);
                                DEF("channels", "%d", dvd_info->titles[j].audiostreams[i].channels);
                                DEF("ap_mode", "%d", dvd_info->titles[j].audiostreams[i].ap_mode);
                                DEF("content", "'%s'", dvd_info->titles[j].audiostreams[i].content);
                                DEF("streamid", "'0x%x'", dvd_info->titles[j].audiostreams[i].streamid);
                                RETURN;
                        }
                        RETURN;
                }

                // CHAPTERS
                if (dvd_info->titles[j].chapters != NULL) {
                        /* This should probably be "chapters": */
                        ARRAY("chapter");
                        for (i=0; i<dvd_info->titles[j].chapter_count; i++)
                        {
                                HASH(0);
                                DEF("ix", "%d", i+1);
                                DEF("length", "%.3f", dvd_info->titles[j].chapters[i].length);
                                DEF("startcell", "%d", dvd_info->titles[j].chapters[i].startcell);
                                RETURN;
                        }
                        RETURN;
                }

                // CELLS
                if (dvd_info->titles[j].cells != NULL) {
                        ARRAY("cell");
                        for (i=0; i<dvd_info->titles[j].cell_count; i++)
                        {
                                HASH(0);
                                DEF("ix", "%d", i+1);
                                DEF("length", "%.3f", dvd_info->titles[j].cells[i].length);
                                DEF("block_mode", "%d", dvd_info->titles[j].cells[i].block_mode);
                                DEF("block_type", "%d", dvd_info->titles[j].cells[i].block_type);
                                RETURN;
                        }
                        RETURN;
                }

                // SUBTITLES
                if (dvd_info->titles[j].subtitles != NULL) {
                        ARRAY("subp");
                        for (i=0; i<dvd_info->titles[j].subtitle_count; i++)
                        {
                                HASH(0);
                                DEF("ix", "%d", i+1);
                                DEF("langcode", "'%s'", dvd_info->titles[j].subtitles[i].langcode);
                                DEF("language", "'%s'", dvd_info->titles[j].subtitles[i].language);
                                DEF("content", "'%s'", dvd_info->titles[j].subtitles[i].content);
                                DEF("streamid", "'0x%x'", dvd_info->titles[j].subtitles[i].streamid);
                                RETURN;
                        }
                        RETURN;
                }
        RETURN;
        }
        }
        }
        RETURN;
        if (! opt_t) {

                DEF("longest_track", "%d", dvd_info->longest_track);
        }
        STOP;


}

