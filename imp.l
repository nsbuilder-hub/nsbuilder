%option yylineno
%option noyywrap
%{
#include "exprtree.h"
#include "imp_yacc.h"
#include "imp_idents.h"

#include <string.h>

void imperror (char *s)
{
	qDebug ( "%s w linii %d na pozycji %d znak %s\n", s, yylineno, textlen, yytext);
	syntaxErrorText = yytext;
}
%}

MINUS		"-"
PLUS		"+"
MULT		"*"
DIV		"/"
MOD		"mod"
IDIV		"div"

ROPER           "="|"<="|"<"|">"|">="|"<>"

BOPER           "and"|"or"|"xor"
NOTOPER         "not"

CYFRA           [0-9]
LICZBA          {CYFRA}+(\.{CYFRA}+)?

LITERA          [_a-zA-Z]
LITERACYFRA     [a-zA-Z0-9]
IDENT           {LITERA}{LITERACYFRA}*

PRZYPSYMB       ":="

NAWIASO         "("
NAWIASZ         ")"

INDO		\[
INDZ		\]

COMMA		,

FUNCIDENT	"sin"|"cos"|"tg"|"ctg"|"exp"|"ln"|"log"|"sqrt"|"abs"

NAPIS		\'[^']+\'
%%
{MOD}		{
	implval = new drzewo_skladn(OPER_ARYTM, strdup (yytext));
#ifdef DEBUG4
qDebug ( "new pointer %p\n", implval);
#endif
#ifdef DEBUG1
qDebug ( "lex: MOD\n");
#endif
	textlen += strlen (yytext);
	return MOD;
}

{IDIV}		{
	implval = new drzewo_skladn(OPER_ARYTM, strdup (yytext));
#ifdef DEBUG4
qDebug ( "new pointer %p\n", implval);
#endif
#ifdef DEBUG1
qDebug ( "lex: IDIV\n");
#endif
	textlen += strlen (yytext);
	return IDIV;
}

{NOTOPER}	{
	struct drzewo_skladn *w;
	/* tworzymy nowy li¶æ drzewa sk³adniowego */
	//w = (struct drzewo_skladn*) malloc (sizeof (struct drzewo_skladn));
	//w->typ = OPER_NOT;
	w = new drzewo_skladn(OPER_NOT, 0L);
#ifdef DEBUG4
qDebug ( "new pointer %p\n", w);
#endif
	w->oper = strdup (yytext);

	implval = w;
#ifdef DEBUG1
qDebug ( "lex: notoper\n");
#endif
	textlen += strlen (yytext);
	return NOTOPER;
}

{BOPER}		{
	struct drzewo_skladn *w;
	/* tworzymy nowy li¶æ drzewa sk³adniowego */
	//w = (struct drzewo_skladn*) malloc (sizeof (struct drzewo_skladn));
	//w->typ = OPER_LOG;
	w = new drzewo_skladn(OPER_LOG, 0L);
#ifdef DEBUG4
qDebug ( "new pointer %p\n", w);
#endif
	w->oper = strdup (yytext);

	implval = w;
#ifdef DEBUG1
qDebug ( "lex: boper: %s\n", w->oper);
#endif
	textlen += strlen (yytext);
	return BOPER;
}

"true"	{
	struct drzewo_skladn *w;
	/* tworzymy nowy li¶æ drzewa sk³adniowego */
	//w = (struct drzewo_skladn*) malloc (sizeof (struct drzewo_skladn));
	//w->typ = ATOM_LOG;
	w = new drzewo_skladn(ATOM_LOG, 0L);
#ifdef DEBUG4
qDebug ( "new pointer %p\n", w);
#endif
	w->val.val = true;
	w->val.d = Long;

	implval = w;
#ifdef DEBUG1
qDebug ( "lex: true\n");
#endif
	textlen += strlen (yytext);
	return TRUESYMB;
}

"false"	{
	struct drzewo_skladn *w;
	/* tworzymy nowy li¶æ drzewa sk³adniowego */
	//w = (struct drzewo_skladn*) malloc (sizeof (struct drzewo_skladn));
	//w->typ = ATOM_LOG;
	w = new drzewo_skladn(ATOM_LOG, 0L);
#ifdef DEBUG4
qDebug ( "new pointer %p\n", w);
#endif
	w->val.val = false;
	w->val.d = Long;

	implval = w;
#ifdef DEBUG1
qDebug ( "lex: false\n");
#endif
	textlen += strlen (yytext);
	return FALSESYMB;
}

{FUNCIDENT}	{
	struct ident_val_t *tmp;
	struct drzewo_skladn *w;

	w = new drzewo_skladn(ATOM_IDENT, (char*)0);
#ifdef DEBUG4
qDebug ( "new pointer %p\n", w);
#endif
	w->zmienna = tmp = new ident_val_t(yytext);

	implval = w;
#ifdef DEBUG1
qDebug ( "lex: ident=%s implval=%p\n", tmp->ident.toLocal8Bit ().constData (), (void*)implval);
#endif
	textlen += strlen (yytext);
	return FUNCIDENT;
}

{IDENT}		{
	struct ident_val_t *tmp;
	struct drzewo_skladn *w;
	
        if (functionMap.isUserDefined (yytext)) {
                w = new drzewo_skladn(ATOM_IDENT, (char*)0);
#ifdef DEBUG4
qDebug ( "new pointer %p\n", w);
#endif
                w->zmienna = tmp = new ident_val_t(yytext);

                implval = w;
#ifdef DEBUG1
qDebug ( "lex: ident=%s implval=%p\n", tmp->ident.toLocal8Bit ().constData (), (void*)implval);
#endif
                textlen += strlen (yytext);
                return FUNCIDENT;
        }

	/* dodajemy identyfikator do hashtablicy */
	if (! identsMap.contains (yytext)) {
		tmp = new ident_val_t(yytext/*, (long)time (NULL)*/);
		identsMap.insert (QString(yytext), tmp);
        } else {
		tmp = identsMap.value (yytext);
	}
	
	/* tworzymy nowy lisc drzewa skladniowego */
	w = new drzewo_skladn(ATOM_IDENT, (char*)0);
#ifdef DEBUG4
qDebug ( "new pointer %p\n", w);
#endif
	w->zmienna = tmp;

	implval = w;
#ifdef DEBUG1
qDebug ( "lex: ident=%s implval=%p implval->zmienna=%p implval->zmienna->ident=%s\n", 
         tmp->ident.toLocal8Bit ().constData (), (void*)implval,
	 implval->zmienna, implval->zmienna->ident.toLocal8Bit ().constData ());
#endif
	textlen += strlen (yytext);
	return IDENT;
}	

{DIV}		{
	implval = new drzewo_skladn(OPER_ARYTM, strdup (yytext));
#ifdef DEBUG4
qDebug ( "new pointer %p\n", implval);
#endif
#ifdef DEBUG3
qDebug ( "lex: DIV\n");
#endif
	textlen += strlen (yytext);
	return DIV;
}

{MINUS}		{
	implval = new drzewo_skladn(OPER_ARYTM, strdup (yytext));
#ifdef DEBUG4
qDebug ( "new pointer %p\n", implval);
#endif
#ifdef DEBUG3
qDebug ( "lex: MINUS\n");
#endif
	textlen += strlen (yytext);
	return MINUS;
}

{PLUS}		{
	implval = new drzewo_skladn(OPER_ARYTM, strdup (yytext));
#ifdef DEBUG4
qDebug ( "new pointer %p\n", implval);
#endif
#ifdef DEBUG3
qDebug ( "lex: PLUS\n");
#endif
	textlen += strlen (yytext);
	return PLUS;
}

{MULT}		{
	implval = new drzewo_skladn(OPER_ARYTM, strdup (yytext));
#ifdef DEBUG4
qDebug ( "new pointer %p\n", implval);
#endif
#ifdef DEBUG3
qDebug ( "lex: MULT\n");
#endif
	textlen += strlen (yytext);
	return MULT;
}

{ROPER}		{
	struct drzewo_skladn *w;
	/* tworzymy nowy lisc drzewa skladniowego */
	w = new drzewo_skladn(OPER_REL, 0L);
#ifdef DEBUG4
qDebug ( "new pointer %p\n", w);
#endif
	w->oper = strdup (yytext);

	implval = w;
#ifdef DEBUG1
qDebug ( "lex: roper: %s\n", w->oper);
#endif
	textlen += strlen (yytext);
	return ROPER;
}

{LICZBA}	{
	struct drzewo_skladn *w;
	/* tworzymy nowy li¶æ drzewa sk³adniowego */
	w = new drzewo_skladn(ATOM_LICZBA, 0.0);
#ifdef DEBUG4
qDebug ( "new pointer %p\n", w);
#endif

	if (strchr (yytext, '.')) {                
                //w->val.fval = atof (yytext);
                w->val.fval = atol (yytext);
                char * p = strchr (yytext, '.');
                double d = 0.1;
                double x = 0;
                while (*(++p)) {
                    x += d * (*p - '0');
                    d /= 10;
                }
                w->val.fval += x;
		w->val.d = Double;
	} else {
		w->val.val = atol (yytext);
                w->val.d = Long;
	}

	implval = w;
#ifdef DEBUG1
qDebug ( "lex: liczba: %s text: %s dot: %p conv: %f\n", w->val.toString ().toLocal8Bit ().constData (), yytext, strchr(yytext,'.'), atof (yytext));
#endif
	textlen += strlen (yytext);
	return LICZBA;	
}

{NAWIASO} {
#ifdef DEBUG1
qDebug ( "lex: (\n");
#endif
	textlen += strlen (yytext);
	return NAWIASO;
}

{NAWIASZ} {
#ifdef DEBUG1
qDebug ( "lex: )\n");
#endif
	textlen += strlen (yytext);
	return NAWIASZ;
}

{PRZYPSYMB} {
#ifdef DEBUG1
qDebug ( "lex: :=\n");
#endif
	textlen += strlen (yytext);
	return PRZYPSYMB;
}

{INDO} {
#ifdef DEBUG1
qDebug ( "lex: [\n");
#endif
	textlen += strlen (yytext);
	return INDO;
}

{INDZ} {
#ifdef DEBUG1
qDebug ( "lex: ]\n");
#endif
	textlen += strlen (yytext);
	return INDZ;
}

{COMMA} {
#ifdef DEBUG1
qDebug ( "lex: ,\n");
#endif
	textlen += strlen (yytext);
	return COMMA;
}

[ \t\n] {
	textlen += strlen (yytext);
}

{NAPIS}	{
#ifdef DEBUG1
qDebug ( "lex: %s\n", yytext);
#endif
	drzewo_skladn *w;
	w = new drzewo_skladn(ATOM_NAPIS, 0L);
#ifdef DEBUG4
qDebug ( "new pointer %p\n", w);
#endif
	w->val.d = String;
	w->val.sval = new QString(QString(yytext).mid (1, strlen (yytext) - 2));
	implval = w;
	
	textlen += strlen (yytext);
	return NAPIS;
}

.	{
        //printf ("lex unknown: %c", yytext[0]);
        imperror ("unknown character");
	textlen += strlen (yytext);        
}
%%
/*
{MOD}		{
	implval = new drzewo_skladn(OPER_ARYTM, strdup (yytext));
#ifdef DEBUG3
qDebug ( "lex: MOD\n");
#endif
	textlen += strlen (yytext);
	return MOD;
}

{IDIV}		{
	implval = new drzewo_skladn(OPER_ARYTM, strdup (yytext));
#ifdef DEBUG3
qDebug ( "lex: IDIV\n");
#endif
	textlen += strlen (yytext);
	return IDIV;
}
*/

