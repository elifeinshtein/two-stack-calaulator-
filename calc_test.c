/******************************************************************************
 * Project: 	2 STACK CALCULATOR
 * Author: 	    Eli Feinshtein
 * Date: 	    11/12/22
 * Version:	1.0
 *******************************************************************************/
#define _GNU_SOURCE
#include <stdio.h> /* printf */
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "calc.h" /* Calculate */
#include "utils.h"
#include "test_utils.h" /* TEST */

#define SUCCESS 0
#define FAILURE 1
#define ARRLEN 100
#define ERROR strtod("NAN", NULL)

int main()
{
    char str[ARRLEN] = "8*7*25/10+9.3*(567-89*76-67*56473-4)+45*93/5*1250/9.2*-1";                /* Result = 15  	*/
    char str2[ARRLEN] = "8+8*3+-2";          /* Result =  30 	*/
    char str3[ARRLEN] = "8-8*3-2/";          /* Result =  ERROR */
    char str4[ARRLEN] = "2/0";               /* Result =  ERROR */
    char str5[ARRLEN] = "8++8*((3-2)*5)";    /* Result = 48 status = SUCCESS */
    char str6[ARRLEN] = "3-2)*5";            /* Result =  ERROR */
    char str7[ARRLEN] = "(3-2/(16))*5+5*(4+4+4)"; /* Result =  74.375 s */
    char str8[ARRLEN] = "3+6 -2"; /* Result = ERROR s */

    TEST("str1", Calculate(str), 8*7*25/10+9.3*(567-89*76-67*56473-4)+45*93/5*1250/9.2*-1);
    TEST("str2", Calculate(str2), 30.0);
    TEST("str3", isnan(Calculate(str3)), 1); 
    TEST("str4", isnan(Calculate(str4)), 1); 
    TEST("str5", Calculate(str5), 48.0);
    TEST("str6", isnan(Calculate(str6)), 1);
    TEST("str7", Calculate(str7), 74.375);
    TEST("str8", isnan(Calculate(str8)), 1);

    PASSED;

    return 0;
}
