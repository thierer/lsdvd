#ifndef _OCODE_H_
#define _OCODE_H_

#include <stdio.h>
#include <stdarg.h>
#include "lsdvd.h"

struct Syntax {
        char *indent;
        char *def;
        char *def_sep;
        char *hash_outer;
        char *hash_inner;
        char *hash_anon;
        char *array_outer;
        char *array_inner;
        char *adef_sep;
        char *return_hash_outer;
        char *return_array_outer;
        char *return_hash_inner;
        char *return_array_inner;
        char *content_quote;
};

extern struct Syntax perl_syntax;
extern struct Syntax python_syntax;
extern struct Syntax ruby_syntax;
extern struct Syntax debug_syntax;
extern struct Syntax json_syntax;

void ocode_print(struct Syntax *, struct dvd_info*);

#endif /* _OCODE_H_ */

