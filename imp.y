%{
#include "exprtree.h"
#include "imp_idents.h"

int imp_scan_string (const char * str);
void imperror (char *s);
int implex ();
/*
*/
%}

%token FUNCIDENT IDENT NAPIS
%token LICZBA
%token TRUESYMB
%token FALSESYMB
%token NAWIASZ
%token NAWIASO
%token INDO
%token INDZ
%token COMMA
%nonassoc PRZYPSYMB
%nonassoc ROPER
%left PLUS MINUS MOD IDIV
%left MULT DIV
%left NEG
%left BOPER
%left NOTOPER
%%

PROGRAM:	INSTR_PRZYP {
			program = $1;
		}
	|	BWYR {
			program = $1;
		}
	|	INSTR_PROC {
			program = $1;
		}
        | 	LISTA_LWYR {
			program = $1; 
		}
        |	LISTA_WARTOSCI {
			program = $1;
		}
;

LWYR:		IDENT { 
#ifdef DEBUG3
qDebug ( "LWYR: IDENT\n");
#endif
	    		$$ = $1; 
		}
	|	WYR_IND { 
#ifdef DEBUG3
qDebug ( "LWYR: WYR_IND\n");
#endif
    			$$ = $1; 
		}
;

LISTA_LWYR: 	IDENT { 
#ifdef DEBUG3
qDebug ( "LISTA_LWYR: LWYR(IDENT)\n");
#endif
			$$ = $1; 
		}
	  |	WYR_IND {
#ifdef DEBUG3
qDebug ( "LISTA_LWYR: LWYR(WYR_IND)\n");
#endif
			$$ = $1;
	  	}
	  | 	IDENT COMMA LISTA_LWYR { 
#ifdef DEBUG3
qDebug ( "LISTA_LWYR: LWYR(IDENT) COMMA LISTA_LWYR\n");
#endif
			struct drzewo_skladn *w = new drzewo_skladn(LISTA_LWYR, 0L); 
#ifdef DEBUG4
qDebug ( "new pointer %p\n", w);
#endif
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	  | 	WYR_IND COMMA LISTA_LWYR { 
#ifdef DEBUG3
qDebug ( "LISTA_LWYR: LWYR(WYR_IND) COMMA LISTA_LWYR\n");
#endif
			struct drzewo_skladn *w = new drzewo_skladn(LISTA_LWYR, 0L); 
#ifdef DEBUG4
qDebug ( "new pointer %p\n", w);
#endif
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
;

LISTA_WARTOSCI:	AWYR/*LICZBA*/ COMMA LISTA_WARTOSCI {
#ifdef DEBUG3
qDebug ( "LISTA_WARTOSCI: AWYR COMMA LISTA_WARTOSCI \n");
#endif
			struct drzewo_skladn *w = new drzewo_skladn(LISTA_WARTOSCI, (ident_val_t*) 0);
#ifdef DEBUG4
qDebug ( "new pointer %p\n", w);
#endif
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
	      }
              | AWYR/*LICZBA*/ COMMA LISTA_LWYR {
#ifdef DEBUG3
qDebug ( "LISTA_WARTOSCI: AWYR COMMA LISTA_LWYR\n");
#endif
			struct drzewo_skladn *w = new drzewo_skladn(LISTA_WARTOSCI, (ident_val_t*) 0);
#ifdef DEBUG4
qDebug ( "new pointer %p\n", w);
#endif
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
	      }
              |	AWYR/*LICZBA*/ {
#ifdef DEBUG3
qDebug ( "LISTA_WARTOSCI: AWYR\n");
#endif
			$$ = $1;
	      }
              | NAPIS {
#ifdef DEBUG3
qDebug ( "LISTA_WARTOSCI: NAPIS\n");
#endif
                        $$ = $1;
              }
              | NAPIS COMMA LISTA_WARTOSCI {
#ifdef DEBUG3
qDebug ( "LISTA_WARTOSCI: NAPIS COMMA LISTA_WARTOSCI \n");
#endif
			drzewo_skladn *w = new drzewo_skladn(LISTA_WARTOSCI, (ident_val_t*) 0);
#ifdef DEBUG4
qDebug ( "new pointer %p\n", w);
#endif
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
	      }
              | NAPIS COMMA LISTA_LWYR {
#ifdef DEBUG3
qDebug ( "LISTA_WARTOSCI: NAPIS COMMA LISTA_LWYR\n");
#endif
			drzewo_skladn *w = new drzewo_skladn(LISTA_WARTOSCI, (ident_val_t*) 0);
#ifdef DEBUG4
qDebug ( "new pointer %p\n", w);
#endif
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
              }
              | IDENT COMMA LISTA_WARTOSCI {
#ifdef DEBUG3
qDebug ( "LISTA_WARTOSCI: IDENT COMMA LISTA_WARTOSCI\n");
#endif
                        drzewo_skladn *w = new drzewo_skladn(LISTA_WARTOSCI, (ident_val_t*) 0);
#ifdef DEBUG4
qDebug ( "new pointer %p\n", w);
#endif
                        w->skladnik[0] = $1;
                        w->skladnik[1] = $3;
                        $$ = w;
              }
;

INSTR_PROC:	IDENT NAWIASO LISTA_LWYR NAWIASZ {
			struct drzewo_skladn *t = $1;
			t->typ = INSTR_PROC;
			t->skladnik[2] = $3;
			$$ = t;
		}
;
/*tutaj to popraw*/
//WYR_IND:	IDENT INDO AWYR INDZ {
WYR_IND:	IDENT INDO LISTA_WARTOSCI INDZ {
#ifdef DEBUG3
qDebug ( "WYR_IND: IDENT INDO LISTA_WARTOSCI INDZ\n");
#endif
			struct drzewo_skladn *w;
			w = $1;
			w->typ = WYR_INDEKS;
			w->skladnik[2] = $3;
#ifdef DEBUG4
qDebug ( "old pointer %p\n", $3);
#endif
			$$ = w;
		}
//	|	IDENT INDO LWYR INDZ {
        |	IDENT INDO LISTA_LWYR INDZ {
#ifdef DEBUG3
qDebug ( "WYR_IND: IDENT INDO LWYR INDZ\n");
#endif
			struct drzewo_skladn *w;
			w = $1;
			w->typ = WYR_INDEKS;
			w->skladnik[2] = $3;
#ifdef DEBUG4
qDebug ( "old pointer %p\n", $3);
#endif
			$$ = w;
		}
;

FUNC_CALL:	FUNCIDENT NAWIASO LISTA_WARTOSCI NAWIASZ {
#ifdef DEBUG3
qDebug ( "FUNC_CALL: FUNCIDENT NAWIASO LISTA_WARTOSCI NAWIASZ\n");
#endif
			drzewo_skladn *w = new drzewo_skladn(FUNC_CALL, 0L);
#ifdef DEBUG4
qDebug ( "new pointer %p\n", w);
#endif
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	FUNCIDENT NAWIASO LISTA_LWYR NAWIASZ {
#ifdef DEBUG3
qDebug ( "FUNC_CALL: FUNCIDENT NAWIASO LISTA_LWYR NAWIASZ\n");
#endif
			drzewo_skladn *w = new drzewo_skladn(FUNC_CALL, 0L);
#ifdef DEBUG4
qDebug ( "new pointer %p\n", w);
#endif
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
;

AWYR:		LICZBA {                                                              
#ifdef DEBUG3
qDebug ( "AWYR: LICZBA\n");
#endif
			$$ = $1;
                }
/*        |       LWYR {
#ifdef DEBUG3
qDebug ( "AWYR: LWYR\n");
#endif
                        $$ = $1;
                } */
	|	FUNC_CALL {
#ifdef DEBUG3
qDebug ( "AWYR: LICZBA\n");
#endif
			$$ = $1;
		}
	|	AWYR MINUS AWYR {
			drzewo_skladn *w;
#ifdef DEBUG3
qDebug ( "AWYR: AWYR MINUS AWYR\n");
#endif
			w = $2;
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;

			$$ = w;
		}
	|	AWYR MINUS LWYR {
			drzewo_skladn *w;
#ifdef DEBUG3
qDebug ( "bison: AWYR MINUS LWYR\n");
#endif
			w = $2;
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	LWYR MINUS AWYR {
			drzewo_skladn *w;
#ifdef DEBUG3
qDebug ( "bison: LWYR MINUS AWYR\n");
#endif
			w = $2;
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	LWYR MINUS LWYR {
			drzewo_skladn *w;
#ifdef DEBUG3
qDebug ( "bison: LWYR MINUS LWYR\n");
#endif
			w = $2;
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	AWYR PLUS AWYR {
			drzewo_skladn *w;
#ifdef DEBUG3
qDebug ( "bison: AWYR PLUS AWYR\n");
#endif
			w = $2;
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	AWYR PLUS LWYR {
			drzewo_skladn *w;
#ifdef DEBUG3
qDebug ( "bison: AWYR PLUS LWYR\n");
#endif
			w = $2;
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	LWYR PLUS AWYR {
			drzewo_skladn *w;
#ifdef DEBUG3
qDebug ( "bison: LWYR PLUS AWYR\n");
#endif
			w = $2;
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	LWYR PLUS LWYR {
			drzewo_skladn *w;
#ifdef DEBUG3
qDebug ( "bison: LWYR PLUS LWYR\n");
#endif
			w = $2;
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	AWYR MULT AWYR {
			drzewo_skladn *w;
#ifdef DEBUG3
qDebug ( "bison: AWYR MULT AWYR\n");
#endif
			w = $2;
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	AWYR MULT LWYR {
			drzewo_skladn *w;
#ifdef DEBUG3
qDebug ( "bison: AWYR MULT LWYR\n");
#endif
			w = $2;
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	LWYR MULT AWYR {
			drzewo_skladn *w;
#ifdef DEBUG3
qDebug ( "bison: LWYR MULT AWYR\n");
#endif
			w = $2;
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	LWYR MULT LWYR {
			drzewo_skladn *w;
#ifdef DEBUG3
qDebug ( "bison: LWYR MULT LWYR\n");
#endif
			w = $2;
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	AWYR DIV AWYR {
			drzewo_skladn *w;
#ifdef DEBUG3
qDebug ( "bison: AWYR DIV AWYR\n");
#endif
			w = $2;
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	AWYR DIV LWYR {
			drzewo_skladn *w;
#ifdef DEBUG3
qDebug ( "bison: AWYR DIV LWYR\n");
#endif
			w = $2;
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	LWYR DIV AWYR {
			drzewo_skladn *w;
#ifdef DEBUG3
qDebug ( "bison: LWYR DIV AWYR\n");
#endif
			w = $2;
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	LWYR DIV LWYR {
			drzewo_skladn *w;
#ifdef DEBUG3
qDebug ( "bison: LWYR DIV LWYR\n");
#endif
			w = $2;
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	AWYR MOD AWYR {
			drzewo_skladn *w;
#ifdef DEBUG3
qDebug ( "bison: AWYR MOD AWYR\n");
#endif
			w = $2;
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	AWYR MOD LWYR {
			drzewo_skladn *w;
#ifdef DEBUG3
qDebug ( "bison: AWYR MOD LWYR\n");
#endif
			w = $2;
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	LWYR MOD AWYR {
			drzewo_skladn *w;
#ifdef DEBUG3
qDebug ( "bison: AWYR MOD AWYR\n");
#endif
			w = $2;
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	LWYR MOD LWYR {
			drzewo_skladn *w;
#ifdef DEBUG3
qDebug ( "bison: AWYR MOD AWYR\n");
#endif
			w = $2;
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	AWYR IDIV AWYR {
			drzewo_skladn *w;
#ifdef DEBUG3
qDebug ( "bison: AWYR IDIV AWYR\n");
#endif
			w = $2;
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	AWYR IDIV LWYR {
			drzewo_skladn *w;
#ifdef DEBUG3
qDebug ( "bison: AWYR IDIV AWYR\n");
#endif
			w = $2;
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	LWYR IDIV AWYR {
			drzewo_skladn *w;
#ifdef DEBUG3
qDebug ( "bison: AWYR IDIV AWYR\n");
#endif
			w = $2;
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	LWYR IDIV LWYR {
			drzewo_skladn *w;
#ifdef DEBUG3
qDebug ( "bison: AWYR IDIV AWYR\n");
#endif
			w = $2;
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	MINUS LWYR %prec NEG {
#ifdef DEBUG3
qDebug ( "bison: NEG LWYR\n");
#endif
			drzewo_skladn *w;
			w = $1;
			w->skladnik[1] = $2;
			$$ = w;
		}
	|	MINUS AWYR %prec NEG {
#ifdef DEBUG3
qDebug ( "bison: NEG AWYR\n");
#endif
			drzewo_skladn *w;
			w = $1;
			w->skladnik[1] = $2;
			$$ = w;
		}
        |       NAWIASO AWYR NAWIASZ {                                                
#ifdef DEBUG3
qDebug ( "bison: NAWIASO AWYR NAWIASZ\n");
#endif
			$$ = $2;
                }                                                                     
;

BWYR:		TRUESYMB {
#ifdef DEBUG2
qDebug ( "true\n");
#endif
			$$ = $1;
		}
	|	FALSESYMB {
#ifdef DEBUG2
qDebug ( "false\n");
#endif
			$$ = $1;
		}
	|	NAWIASO BWYR NAWIASZ {
#ifdef DEBUG2
qDebug ( "(bwyr)\n");
#endif
			$$ = $2;
		}
	|	BWYR BOPER BWYR {
			drzewo_skladn *w;
			w = $2;
#ifdef DEBUG2
qDebug ( "(%d %s %d)\n", $1->typ, w->oper, $3->typ);
#endif
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	NOTOPER BWYR {
			drzewo_skladn *w;
			w = $1;
			w->skladnik[0] = $2;
			$$ = w;
		}
	|	AWYR ROPER AWYR {
			drzewo_skladn *w;
			w = $2;
#ifdef DEBUG2
qDebug ( "(%d %s %d)\n", $1->typ, w->oper, $3->typ);
#endif
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	AWYR ROPER LWYR {
			drzewo_skladn *w;
			w = $2;
#ifdef DEBUG2
qDebug ( "(%d %s %d)\n", $1->typ, w->oper, $3->typ);
#endif
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	LWYR ROPER AWYR {
			drzewo_skladn *w;
			w = $2;
#ifdef DEBUG2
qDebug ( "(%d %s %d)\n", $1->typ, w->oper, $3->typ);
#endif
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
	|	LWYR ROPER LWYR {
			drzewo_skladn *w;
			w = $2;
#ifdef DEBUG2
qDebug ( "(%d %s %d)\n", $1->typ, w->oper, $3->typ);
#endif
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
;

INSTR_PRZYP:	LWYR PRZYPSYMB AWYR	{
#ifdef DEBUG3
qDebug ( "INSTR_PRZYP: LWYR PRZYPSYMB AWYR\n");
#endif
			struct drzewo_skladn *w;
			w = new drzewo_skladn(INSTR_PODSTAWIENIE, 0L);
#ifdef DEBUG4
qDebug ( "new pointer %p\n", w);
#endif
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
#ifdef DEBUG3
//wypisz_ds ($$);
#endif 
		}
	|	LWYR PRZYPSYMB LWYR	{
#ifdef DEBUG3
qDebug ( "INSTR_PRZYP: LWYR PRZYPSYMB LWYR\n");
#endif
			struct drzewo_skladn *w;
			w = new drzewo_skladn(INSTR_PODSTAWIENIE, 0L);
#ifdef DEBUG4
qDebug ( "new pointer %p\n", w);
#endif
			w->skladnik[0] = $1;
			w->skladnik[1] = $3;
			$$ = w;
		}
;

%%

