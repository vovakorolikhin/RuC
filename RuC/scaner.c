//
//  scaner.c
//  RuC
//
//  Created by Andrey Terekhov on 04/08/14.
//  Copyright (c) 2014 Andrey Terekhov. All rights reserved.
//
#define _CRT_SECURE_NO_WARNINGS
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "global_vars.h"
extern void error(int);
extern void warning(int);
extern void printf_char(int);

/// Считывает один символ в формате UTF-8. Может быть русским, тогда разбирается 2 символа.
/// \return
int getnext()
{
    // reads UTF-8
    unsigned char firstchar, secondchar;
    if (fscanf(input, "%c", &firstchar) == EOF)
        return EOF;
    else
    {
        if ((firstchar & /*0b11100000*/ 0xE0) == /*0b11000000*/ 0xC0)
        {
            fscanf(input, "%c", &secondchar);

            nextchar = ((int)(firstchar & /*0b11111*/ 0x1F)) << 6 |
                       (secondchar & /*0b111111*/ 0x3F);
        }
        else
            nextchar = firstchar;
        if (nextchar == 13 /* cr */)
            getnext();
        //                if(kw)
        //                    printf("nextchar %c %i\n", nextchar, nextchar);
        printf("%c %c | ", firstchar, nextchar);
    }
    printf("\n");
    return nextchar;
}

void onemore()
{
    curchar = nextchar;
    nextchar = getnext();
    //    if (kw)
    //        printf("curchar =%c %i nextchar=%c %i\n", curchar, curchar,
    //        nextchar, nextchar);
}

void endofline()
{
    if (prep_flag == 1)
    {
        int j;
        printf("line %i) ", line - 1);
        for (j = lines[line - 1]; j < lines[line]; j++)
            if (source[j] != EOF)
                printf_char(source[j]);
        fflush(stdout);
    }
}

void endnl()
{
    lines[++line] = charnum;
    lines[line + 1] = charnum;
    if (kw)
    {
        endofline();
    }
}
// записывает информацию в массив source, подсчитывает количество символов в строке в lines
void nextch()
{
    onemore();
    if (curchar == EOF)
    {
        onemore();
        lines[++line] = charnum;
        lines[line + 1] = charnum;
        if (kw)
        {
            endofline();
            printf("\n");
        }
        return;
    }

    source[charnum++] = curchar;
    if (instring)
        return;

    if (curchar == '/' && nextchar == '/')
    {
        do
        {
            onemore();
            source[charnum++] = curchar;
            if (curchar == EOF)
            {
                endnl();
                printf("\n");
                return;
            }
        } while (curchar != '\n');

        endnl();
        return;
    }

    if (curchar == '/' && nextchar == '*')
    {
        onemore();
        source[charnum++] =
            curchar; // надо сразу выесть /*, чтобы не попасть на /*/
        do
        {
            onemore();
            source[charnum++] = curchar;
            if (curchar == EOF)
            {
                endnl();
                printf("\n");
                error(comm_not_ended);
            }
            if (curchar == '\n')
                endnl();
        } while (curchar != '*' || nextchar != '/');

        onemore();
        source[charnum++] = curchar;
        curchar = ' ';
        return;
    }
    if (curchar == '\n')
        endnl();
    return;
}

void next_string_elem()
{
    num = curchar;
    if (curchar == '\\')
    {
        nextch();
        if (curchar == 'n' || curchar == 1085 /* 'н' */)
            num = 10;
        else if (curchar == 't' || curchar == 1090 /* 'т' */)
            num = 9;
        else if (curchar == '0')
            num = 0;
        else if (curchar != '\'' && curchar != '\\' && curchar != '\"')
            error(bad_escape_sym);
        else
            num = curchar;
    }
    nextch();
}

int letter()
{
    return (curchar >= 'A' && curchar <= 'Z') ||
           (curchar >= 'a' && curchar <= 'z') || curchar == '_' ||
           (curchar >= 0x410 /*А */ && curchar <= 0x44F /*'я'*/);
}

int digit() { return curchar >= '0' && curchar <= '9'; }

int ispower()
{
    return curchar == 'e' ||
           curchar ==
               'E'; // || curchar == 'е' || curchar == 'Е') // это русские е и Е
}

int equal(int i, int j)
{
    ++i;
    ++j;
    while (reprtab[++i] == reprtab[++j])
        if (reprtab[i] == 0 && reprtab[j] == 0)
            return 1;
    return 0;
}

int scan()
{
    int cr;
    while (curchar == ' ' || curchar == '\t' || curchar == '\n')
        nextch();
    //    printf("scan curchar=%c %i\n", curchar, curchar);
    switch (curchar)
    {
    case EOF:
        return LEOF;
    case ',':
    {
        nextch();
        return COMMA;
    }

    case '=':
        nextch();
        if (curchar == '=')
        {
            nextch();
            cr = EQEQ;
        }
        else
            cr = ASS;
        return cr;

    case '*':
        nextch();
        if (curchar == '=')
        {
            nextch();
            cr = MULTASS;
        }
        else
            cr = LMULT;
        return cr;

    case '/':
        nextch();
        if (curchar == '=')
        {
            nextch();
            cr = DIVASS;
        }
        else
            cr = LDIV;
        return cr;

    case '%':
        nextch();
        if (curchar == '=')
        {
            nextch();
            cr = REMASS;
        }
        else
            cr = LREM;
        return cr;

    case '+':
        nextch();
        if (curchar == '=')
        {
            nextch();
            cr = PLUSASS;
        }
        else if (curchar == '+')
        {
            nextch();
            cr = INC;
        }
        else
            cr = LPLUS;
        return cr;

    case '-':
        nextch();
        if (curchar == '=')
        {
            nextch();
            cr = MINUSASS;
        }
        else if (curchar == '-')
        {
            nextch();
            cr = DEC;
        }
        else if (curchar == '>')
        {
            nextch();
            cr = ARROW;
        }
        else
            cr = LMINUS;
        return cr;

    case '<':
        nextch();
        if (curchar == '<')
        {
            nextch();
            if (curchar == '=')
            {
                nextch();
                cr = SHLASS;
            }
            else
                cr = LSHL;
        }
        else if (curchar == '=')
        {
            nextch();
            cr = LLE;
        }
        else
            cr = LLT;

        return cr;

    case '>':
        nextch();
        if (curchar == '>')
        {
            nextch();
            if (curchar == '=')
            {
                nextch();
                cr = SHRASS;
            }
            else
                cr = LSHR;
        }
        else if (curchar == '=')
        {
            nextch();
            cr = LGE;
        }
        else
            cr = LGT;
        return cr;

    case '&':
        nextch();
        if (curchar == '=')
        {
            nextch();
            cr = ANDASS;
        }
        else if (curchar == '&')
        {
            nextch();
            cr = LOGAND;
        }
        else
            cr = LAND;
        return cr;

    case '^':
        nextch();
        if (curchar == '=')
        {
            nextch();
            cr = EXORASS;
        }
        else
            cr = LEXOR;
        return cr;

    case '|':
        nextch();
        if (curchar == '=')
        {
            nextch();
            cr = ORASS;
        }
        else if (curchar == '|')
        {
            nextch();
            cr = LOGOR;
        }
        else
            cr = LOR;
        return cr;

    case '!':
        nextch();
        if (curchar == '=')
        {
            nextch();
            cr = NOTEQ;
        }
        else
            cr = LOGNOT;
        return cr;
    case '\'':
    {
        instring = 1;
        nextch();
        next_string_elem();
        if (curchar != '\'')
            error(no_right_apost);
        nextch();
        instring = 0;
        ansttype = LCHAR;
        return NUMBER;
    }
    case '\"':
    {
        int n = 0, flag = 1;
        instring = 1;
        nextch();
        while (flag)
        {
            while (curchar != '\"' && n < MAXSTRINGL)
            {
                next_string_elem();
                lexstr[n++] = num;
                //                    printf("n= %i %c %i\n", n-1, num, num);
            }
            if (n == MAXSTRINGL)
                error(too_long_string);
            nextch();
            while (curchar == ' ' || curchar == '\t' || curchar == '\n')
                nextch();
            if (curchar == '\"')
                nextch();
            else
                flag = 0;
        }

        num = n;
        instring = 0;
        return STRING;
    }
    case '(':
    {
        nextch();
        return LEFTBR;
    }

    case ')':
    {
        nextch();
        return RIGHTBR;
    }

    case '[':
    {
        nextch();
        return LEFTSQBR;
    }

    case ']':
    {
        nextch();
        return RIGHTSQBR;
    }

    case '~':
    {
        nextch();
        return LNOT; // поразрядное отрицание
    }
    case '{':
    {
        nextch();
        return BEGIN;
    }
    case '}':
    {
        nextch();
        return END;
    }
    case ';':
    {
        nextch();
        return SEMICOLON;
    }
    case '?':
    {
        nextch();
        return QUEST;
    }
    case ':':
    {
        nextch();
        return COLON;
    }
    case '.':
        if (nextchar < '0' || nextchar > '9')
        {
            nextch();
            return DOT;
        }
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    {
        int flagint = 1, flagtoolong = 0;
        double k;
        num = 0;
        numdouble = 0.0;
        while (digit())
        {
            numdouble = numdouble * 10 + (curchar - '0');
            if (numdouble > (double)INT_MAX)
            {
                flagtoolong = 1;
                flagint = 0;
            }
            num = num * 10 + (curchar - '0');
            nextch();
        }

        if (curchar == '.')
        {
            flagint = 0;
            nextch();
            k = 0.1;
            while (digit())
            {
                numdouble += (curchar - '0') * k;
                k *= 0.1;
                nextch();
            }
        }

        if (ispower())
        {
            int d = 0, k = 1, i;
            nextch();
            if (curchar == '-')
            {
                flagint = 0;
                nextch();
                k = -1;
            }
            else if (curchar == '+')
                nextch();
            if (!digit())
                error(must_be_digit_after_exp);
            while (digit())
            {
                d = d * 10 + curchar - '0';
                nextch();
            }
            if (flagint)
                for (i = 1; i <= d; i++)
                    num *= 10;
            numdouble *= pow(10.0, k * d);
        }

        if (flagint)
        {
            ansttype = LINT;
            return NUMBER;
        }
        else
        {
            if (flagtoolong)
                warning(too_long_int);
            ansttype = LFLOAT;
        }
        memcpy(&numr, &numdouble, sizeof(double));
        return NUMBER;
    }

    default:
        if (letter() || curchar == '#')
        {
            int oldrepr = rp, r;
            rp += 2;
            hash = 0;
            /*
             * У каждого слова* есть хэш_1. По хэшу_1 в hashtab хранится число r
             * r = индекс, по которому хранится коллизия слова в reprtab*
             * r + 1 = тип включевого слова - ВОЗВРАЩАЕТСЯ ОН
             * r + 2... - сами слова. Символ окончания 0.
             */

            // решетка на 1 месте -- значит, ключевое слово препроцессора
            do
            {
                // считывание слова пока символ или цифра, вычисление хэша, занесение символов в reprtab
                hash += curchar;
                reprtab[rp++] = curchar;
                nextch();
            } while (letter() || digit());

            hash &= 255;
            reprtab[rp++] = 0;
            r = hashtab[hash]; // получение позиции в reprtab по хэшу
            if (r)
            {
                do
                {
                    if (equal(r, oldrepr)) // равны ли ключевые слова?
                    {
                        rp = oldrepr; // если равны, то возвращаем указатель в reprtab обратно, т.к. ничего записывать не надо
                        return (reprtab[r + 1] < 0) ? reprtab[r + 1]
                                                    : (repr = r, IDENT);
                    }
                    else
                        r = reprtab[r];
                } while (r);
            }
            reprtab[oldrepr] = hashtab[hash];
            repr = hashtab[hash] = oldrepr;
            reprtab[repr + 1] = (keywordsnum)
                                    ? -((++keywordsnum - 2) / 4)
                                    : 1; // 0 - только MAIN, < 0 - ключевые
                                         // слова, 1 - обычные иденты
            return IDENT;
        }
        else
        {
            printf("плохой символ %c %i\n", curchar, curchar);
            nextch();
            exit(10);
        }
    }
}

int scaner()
{
    cur = next;
    next = scan();
    //    if(kw)
    //        printf("scaner cur %i next %i repr %i\n", cur, next, repr);
    return cur;
}
