#include <stdio.h>
#include "lsdvd.h"

void ohuman_print(struct dvd_info *dvd_info) {

	printf("Disc Title: %s\n", dvd_info->discinfo.disc_title);

	int i, j;
	for (j=0; j < dvd_info->title_count; j++) 
	{
	if ( opt_t == j+1 || opt_t == 0 ) {
		
		//GENERAL
		printf("Title: %02d, Length: %02d:%02d:%02d.%03d ", j+1,
				dvd_info->titles[j].general.playback_time.hour,
				dvd_info->titles[j].general.playback_time.minute,
				dvd_info->titles[j].general.playback_time.second,
				dvd_info->titles[j].general.playback_time.usec);
//				(int)dvd_info->titles[j].general.length / 3600,
//				(int)(dvd_info->titles[j].general.length / 60) % 60,
//				(float)((int)(dvd_info->titles[j].general.length * 1000) % 6000) / 1000); //,
//				dvd_info->titles[j].general.length);
		printf("Chapters: %02d, Cells: %02d, ", dvd_info->titles[j].chapter_count_reported, dvd_info->titles[j].cell_count);
		printf("Audio streams: %02d, Subpictures: %02d", dvd_info->titles[j].audiostream_count, dvd_info->titles[j].subtitle_count);
		printf("\n"); 
		
                if (dvd_info->titles[j].parameter.format != NULL ) {
			printf("\tVTS: %02d, TTN: %02d, ", dvd_info->titles[j].parameter.vts, dvd_info->titles[j].parameter.ttn);
			printf("FPS: %.2f, ", dvd_info->titles[j].parameter.fps);
			printf("Format: %s, Aspect ratio: %s, ", dvd_info->titles[j].parameter.format, dvd_info->titles[j].parameter.aspect);
			printf("Width: %s, Height: %s, ", dvd_info->titles[j].parameter.width, dvd_info->titles[j].parameter.height);
			printf("DF: %s\n", dvd_info->titles[j].parameter.df);
		}

                // PALETTE
                if (dvd_info->titles[j].palette != NULL ) {
                        printf("\tPalette: ");
                        for (i=0; i < 16; i++) {
                                printf("%06x ", dvd_info->titles[j].palette[i]);
                        }
			printf("\n");
                }


		// ANGLES
		if (dvd_info->titles[j].angle_count) {
			printf("\tNumber of Angles: %d\n", dvd_info->titles[j].angle_count);
		}

		// AUDIO
		if (dvd_info->titles[j].audiostreams != NULL) {
			for (i=0; i<dvd_info->titles[j].audiostream_count; i++)
			{
				printf("\tAudio: %d, Language: %s - %s, ", i +1 ,
						dvd_info->titles[j].audiostreams[i].langcode, dvd_info->titles[j].audiostreams[i].language);
				printf("Format: %s, ", dvd_info->titles[j].audiostreams[i].format);
				printf("Frequency: %s, ", dvd_info->titles[j].audiostreams[i].frequency);
				printf("Quantization: %s, ", dvd_info->titles[j].audiostreams[i].quantization);
				printf("Channels: %d, AP: %d, ", dvd_info->titles[j].audiostreams[i].channels, dvd_info->titles[j].audiostreams[i].ap_mode);
				printf("Content: %s, ", dvd_info->titles[j].audiostreams[i].content);
				printf("Stream id: 0x%x", dvd_info->titles[j].audiostreams[i].streamid);
				printf("\n");
			}
		}

		// CHAPTERS
		if (dvd_info->titles[j].chapters != NULL) {
			for (i=0; i<dvd_info->titles[j].chapter_count; i++)
			{
//				printf("\tChapter: %02d, Length: %02d:%02d:%02d.%03d, Start Cell: %02d\n", i+1,
//						(int)dvd_info->titles[j].chapters[i].length / 3600,
//						(int)(dvd_info->titles[j].chapters[i].length / 60 ) % 60,
//						(int)(dvd_info->titles[j].chapters[i].length) % 60,
//						(int)(dvd_info->titles[j].chapters[i].length*1000) % 1000,
//						dvd_info->titles[j].chapters[i].startcell);
				printf("\tChapter: %02d, Length: %02d:%02d:%02d.%03d, Start Cell: %02d\n", i+1,
						dvd_info->titles[j].chapters[i].playback_time.hour,
						dvd_info->titles[j].chapters[i].playback_time.minute,
						dvd_info->titles[j].chapters[i].playback_time.second,
						dvd_info->titles[j].chapters[i].playback_time.usec,
						dvd_info->titles[j].chapters[i].startcell);
			}
		}

		// CELLS
		if (dvd_info->titles[j].cells != NULL) {
			for (i=0; i<dvd_info->titles[j].cell_count; i++)   
			{
//				printf("\tCell: %02d, Length: %02d:%02d:%02d.%03d\n", i+1, 
//						(int)dvd_info->titles[j].cells[i].length / 3600,
//						(int)(dvd_info->titles[j].cells[i].length / 60 ) % 60,
//						(int)(dvd_info->titles[j].cells[i].length) % 60,
//						(int)(dvd_info->titles[j].cells[i].length*1000) % 1000);
				printf("\tCell: %02d, Length: %02d:%02d:%02d.%03d\n", i+1, 
						dvd_info->titles[j].cells[i].playback_time.hour,
						dvd_info->titles[j].cells[i].playback_time.minute,
						dvd_info->titles[j].cells[i].playback_time.second,
						dvd_info->titles[j].cells[i].playback_time.usec);
			}
		}

		// SUBTITLES
		if (dvd_info->titles[j].subtitles != NULL) {
			for (i=0; i<dvd_info->titles[j].subtitle_count; i++)
			{
				printf("\tSubtitle: %02d, Language: %s - %s, ", i+1,
						dvd_info->titles[j].subtitles[i].langcode,
						dvd_info->titles[j].subtitles[i].language);
				printf("Content: %s, ", dvd_info->titles[j].subtitles[i].content);
				printf("Stream id: 0x%x, ", dvd_info->titles[j].subtitles[i].streamid);
				printf("\n");
			}
		}
		printf("\n");
	}
	}

	if (! opt_t) {
		printf("Longest track: %02d\n\n", dvd_info->longest_track);
	}
}
