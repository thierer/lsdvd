#include "ocode.h"

/*
   Simple helper macros for generating Perl structures
*/
#define INDENT  { int i; for(i=0; i<_lvl; i++) printf("%s", syntax->indent); }
#define DEF DEF_
#define ADEF ADEF_
#define SEP SEP_()
#define CONTAINER_SEP(idx, bound)  CONTAINER_SEP_(idx, bound)
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
        "</return_array_inner>%s",
	"\'"
};

struct Syntax json_syntax = {
	"\t",			// indent
	"\"%s\": ",		// def
	",\n",			// def_sep
	"{\n",			// hash_outer
	"\"%s\": {\n",		// hash_inner
	"{\n",			// hash_anon
	"[\n",			// array_outer
	"\"%s\": [\n",		// array_inner
	",\n",			// adef_sep
	"}",			// return_hash_outer
	"]",			// return_array_outer
	"}",			// return_hash_inner
	"]",			// return_array_inner
	"\""			// content_quote
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
        ");",
        ");",
        "}",
        "]",
	"\'"
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
        "}",
        "]",
        "}",
        "]",
	"\'"
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
        ":%s => [\n",
        ",\n",
        "}",
        "<return_array_outer/>",
        "}",
        "]",
	"\'"
};


static int _lvl = 0;
static char *_lvl_return[256];
static struct Syntax *syntax = &perl_syntax;

static void SEP_() {
	printf("%s", syntax->def_sep);
}

static void CONTAINER_SEP_(int idx, int bound) {
	if ((bound > 1) && (idx != (bound - 1)))
		SEP;
}

static void DEF_(char *name, const char *format, ...) {
        va_list argp;
        INDENT;
        printf(syntax->def, name);

        va_start(argp, format);
        vprintf(format, argp);
        va_end(argp);
}

static void HASH_(char *name) {
        INDENT;
        if (name) {
                printf((_lvl ? syntax->hash_inner : syntax->hash_outer), name);
        } else {
                printf("%s", syntax->hash_anon);
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
}

static void RETURN_()  {
        _lvl--;
         printf("\n");
        INDENT;
        printf("%s", _lvl_return[_lvl]);
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
         char *q = syntax_->content_quote;

        START;
        DEF("device", "%s%s%s", q, dvd_info->discinfo.device, q); SEP;
        DEF("title", "%s%s%s", q, dvd_info->discinfo.disc_title, q); SEP;
        DEF("alt_title", "%s%s%s", q, dvd_info->discinfo.disc_alt_title, q); SEP;
        DEF("serial_no", "%s%s%s", q, dvd_info->discinfo.disc_serial_no, q); SEP;
        if (dvd_info->discinfo.disc_id != NULL) {
                DEF("disc_id", "%s%s%s", q, dvd_info->discinfo.disc_id, q); SEP;
        }
        DEF("vmg_id", "%s%.12s%s", q, dvd_info->discinfo.vmg_id, q); SEP;
        DEF("provider_id", "%s%.32s%s", q, dvd_info->discinfo.provider_id, q); SEP;

        /* This should probably be "tracks": */
        ARRAY("track");

        for (j=0; j < dvd_info->title_count; j++)
        {
                if ( opt_t == j+1 || opt_t == 0 ) {
                        // GENERAL
                        if (dvd_info->titles[j].enabled) {

                                HASH(0);
                                DEF("ix", "%d", j+1); SEP;
                                DEF("length", "%.3f", dvd_info->titles[j].general.length); SEP;
                                DEF("vts_id", "%s%.12s%s", q, dvd_info->titles[j].general.vts_id, q);

                                if (dvd_info->titles[j].parameter.format != NULL ) {
                                        SEP;
                                        DEF("vts", "%d", dvd_info->titles[j].parameter.vts); SEP;
                                        DEF("ttn", "%d", dvd_info->titles[j].parameter.ttn); SEP;
                                        DEF("fps", "%.2f", dvd_info->titles[j].parameter.fps); SEP;
                                        DEF("format", "%s%s%s", q, dvd_info->titles[j].parameter.format, q); SEP;
                                        DEF("aspect", "%s%s%s", q, dvd_info->titles[j].parameter.aspect, q); SEP;
                                        DEF("width", "%s", dvd_info->titles[j].parameter.width); SEP;
                                        DEF("height", "%s", dvd_info->titles[j].parameter.height); SEP;
                                        DEF("df", "%s%s%s", q, dvd_info->titles[j].parameter.df, q);
                                }

                                // PALETTE
                                if (dvd_info->titles[j].palette != NULL) {
                                        SEP;
                                        ARRAY("palette");
                                        for (i=0; i < 16; i++) {
                                                ADEF("%s%06x%s", q,  dvd_info->titles[j].palette[i], q);
                                                CONTAINER_SEP(i, 16);
                                        }
                                        RETURN;
                                }

                                // ANGLES
                                if (dvd_info->titles[j].angle_count) { // poor check, but there's no other info anyway.
                                        SEP;
                                        DEF("angles", "%d", dvd_info->titles[j].angle_count);
                                }

                                // AUDIO
                                if (dvd_info->titles[j].audiostreams != NULL ) {
                                        SEP;
                                        ARRAY("audio");
                                        for (i=0; i<dvd_info->titles[j].audiostream_count; i++)
                                        {
                                                HASH(0);
                                                DEF("ix", "%d", i+1); SEP;
                                                DEF("langcode", "%s%s%s", q, dvd_info->titles[j].audiostreams[i].langcode, q); SEP;
                                                DEF("language", "%s%s%s", q, dvd_info->titles[j].audiostreams[i].language, q); SEP;
                                                DEF("format", "%s%s%s", q, dvd_info->titles[j].audiostreams[i].format, q); SEP;
                                                DEF("frequency", "%s", dvd_info->titles[j].audiostreams[i].frequency); SEP;
                                                DEF("quantization", "%s%s%s", q, dvd_info->titles[j].audiostreams[i].quantization, q); SEP;
                                                DEF("channels", "%d", dvd_info->titles[j].audiostreams[i].channels); SEP;
                                                DEF("ap_mode", "%d", dvd_info->titles[j].audiostreams[i].ap_mode); SEP;
                                                DEF("content", "%s%s%s", q, dvd_info->titles[j].audiostreams[i].content, q); SEP;
                                                DEF("streamid", "%s0x%x%s", q, dvd_info->titles[j].audiostreams[i].streamid, q);
                                                RETURN;
                                                CONTAINER_SEP(i, dvd_info->titles[j].audiostream_count);
                                        }
                                        RETURN;
                                }

                                // CHAPTERS
                                if (dvd_info->titles[j].chapters != NULL) {
                                        SEP;
                                        /* This should probably be "chapters": */
                                        ARRAY("chapter");
                                        for (i=0; i<dvd_info->titles[j].chapter_count; i++)
                                        {
                                                HASH(0);
                                                DEF("ix", "%d", i+1); SEP;
                                                DEF("length", "%.3f", dvd_info->titles[j].chapters[i].length); SEP;
                                                DEF("startcell", "%d", dvd_info->titles[j].chapters[i].startcell);
                                                RETURN;
                                                CONTAINER_SEP(i, dvd_info->titles[j].chapter_count);
                                        }
                                        RETURN;
                                }

                                // CELLS
                                if (dvd_info->titles[j].cells != NULL) {
                                        SEP;
                                        ARRAY("cell");
                                        for (i=0; i<dvd_info->titles[j].cell_count; i++)
                                        {
                                                HASH(0);
                                                DEF("ix", "%d", i+1); SEP;
                                                DEF("length", "%.3f", dvd_info->titles[j].cells[i].length); SEP;
                                                DEF("block_mode", "%d", dvd_info->titles[j].cells[i].block_mode); SEP;
                                                DEF("block_type", "%d", dvd_info->titles[j].cells[i].block_type);
                                                RETURN;
                                                CONTAINER_SEP(i, dvd_info->titles[j].cell_count);
                                        }
                                        RETURN;
                                }

                                // SUBTITLES
                                if (dvd_info->titles[j].subtitles != NULL) {
                                        SEP;
                                        ARRAY("subp");
                                        for (i=0; i<dvd_info->titles[j].subtitle_count; i++)
                                        {
                                                HASH(0);
                                                DEF("ix", "%d", i+1); SEP;
                                                DEF("langcode", "%s%s%s", q, dvd_info->titles[j].subtitles[i].langcode, q); SEP;
                                                DEF("language", "%s%s%s", q, dvd_info->titles[j].subtitles[i].language, q); SEP;
                                                DEF("content", "%s%s%s", q, dvd_info->titles[j].subtitles[i].content, q); SEP;
                                                DEF("streamid", "%s0x%x%s", q, dvd_info->titles[j].subtitles[i].streamid, q);
                                                RETURN;
                                                CONTAINER_SEP(i, dvd_info->titles[j].subtitle_count);
                                        }
                                        RETURN;
                                }
                                RETURN;
                                if (j != (dvd_info->title_count-1))
                                        SEP;
                        }
                }
        }
        RETURN;
        SEP;
        if (! opt_t) {
                DEF("longest_track", "%d", dvd_info->longest_track);
        }
        STOP;


}

