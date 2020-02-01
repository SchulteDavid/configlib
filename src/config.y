
%name-prefix "conf"


%code requires {
    #include "parser.h"
}

%{
#include "parser.h"
#include <stdlib.h>
#include <stdio.h>

extern int conflex();
void conferror(node_list_t ** ls, char * msg);

value_t createValue(VALUE_TYPE_E type, long lData, double dData, char * sData, node_list_t * ls);

%}

%union {

    unsigned int value_type;
    char * str;
    long i;
    double f;
    value_t val;
    node_t * node;
    node_list_t * list;
    value_list_t * valueList;

}

%token<value_type> TYPE
%token<str> NAME
%token<i> V_INTEGER
%token<f> V_FLOAT
%token<str> V_STRING

%type<val> value
%type<node> declaration
%type<list> nodeList
%type<valueList> valueList

%parse-param {node_list_t ** parseList}

%%

document:
    nodeList                        {*parseList = $1;}
    ;

nodeList:
    declaration                     {$$ = nodeListCreate($1);}
    | nodeList declaration          {$$ = nodeListAppend($1, $2);}

declaration:
    TYPE NAME '=' value                    {$$ = buildNode($1, $2, $4);}
    | TYPE NAME '=' '[' valueList ']'       {$$ = buildNodeList($1, $2, $5);}
    ;

valueList:
    value                           {$$ = valueListCreate($1);}
    | valueList ',' value           {$$ = valueListAppend($1, $3);}
    ;

value:
    V_INTEGER                       {$$ = createValue(TYPE_INT64, $1, 0, NULL, NULL);}
    |V_FLOAT                        {$$ = createValue(TYPE_FLOAT64, 0, $1, NULL, NULL);}
    |V_STRING                       {$$ = createValue(TYPE_STRING, 0, 0, $1, NULL);}
    |'{' nodeList '}'               {$$ = createValue(TYPE_COMPOUND, 0, 0, NULL, $2);}
    ;

%%

value_t createValue(VALUE_TYPE_E type, long lData, double dData, char * sData, node_list_t * ls) {

    value_t v;
    v.type = type;

    switch (type) {

        case TYPE_INT64:
            v.v.i = lData;
            break;

        case TYPE_FLOAT64:
            v.v.d = dData;
            break;

        case TYPE_STRING:
            v.v.str = sData;
            break;

        case TYPE_COMPOUND:
            v.v.ls = ls;
            break;

    }

    return v;

}

void conferror(node_list_t ** ls, char * msg) {
    printf("%s\n", msg);
}

int confwrap() {
    return 1;
}
