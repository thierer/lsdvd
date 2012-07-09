
#ifndef _OXML_H_
#define _OXML_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "lsdvd.h"

/*
   Simple helper macros for generating XML structures
*/
#define XMLINDENT  { int i; for(i=0; i<_xlvl; i++) printf("    "); }
#define XMLDEF XMLDEF_
#define XMLBOX XMLBOX_
#define XMLRETURN XMLRETURN_()
#define XMLSTART XMLSTART_()
#define XMLSTOP XMLSTOP_()

void oxml_print(struct dvd_info*);

#endif /* _OXML_H_ */
