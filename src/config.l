
%option prefix="conf"

%{

#include "config.parser.h"
#include "parser.h"

char * duplicateString(char * str);

%}

%%

"int32"                     {conflval.value_type = TYPE_INT32; return TYPE;}
"int64"                     {conflval.value_type = TYPE_INT64; return TYPE;}
"float32"                   {conflval.value_type = TYPE_FLOAT32; return TYPE;}
"float64"                   {conflval.value_type = TYPE_FLOAT64; return TYPE;}
"string"                    {conflval.value_type = TYPE_STRING; return TYPE;}
"comp"                      {conflval.value_type = TYPE_COMPOUND; return TYPE;}

[0-9]+                      {conflval.i = atol(conftext); return V_INTEGER;}
-[0-9]+                     {conflval.i = atol(conftext); return V_INTEGER;}

[0-9]*\.[0-9]*              {conflval.f = atof(conftext); return V_FLOAT;}
-[0-9]*\.[0-9]*             {conflval.f = atof(conftext); return V_FLOAT;}

\".*\"                      {conflval.str = duplicateString(conftext); return V_STRING;}

[a-zA-Z\_][a-zA-Z\_0-9]*    {conflval.str = duplicateString(conftext); return NAME;}

[=\{\}\[\]\,]               {return *conftext;}
[\t\r\f ]	                {}
"\n"                        {}

%%

char * duplicateString(char * str) {

    int length = strlen(str);
    char * dest = malloc(sizeof(char) * (length+1));

    strcpy(dest, str);

    return dest;

}
