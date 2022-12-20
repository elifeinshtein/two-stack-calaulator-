/******************************************************************************
 * Project: 	CALCULATOR 
 * Author: 	   Eli Feinshtein
 * Date: 	    11/12/22
 * Version:	1.0
 *******************************************************************************/
#ifndef __CALC_H__
#define __CALC_H__

#include <math.h> /*when compiling, use -lm flag (link math)*/

typedef struct calc calc_ty;

/* Description:   Calculate the calculation specified in the string calculation
   Arguments: 	  calculation - A string consisting of numbers and operators,
                                the number are in decimal base, and may have a single decimal point,
                  the string must end with a terminating '=' (or '\0')
                  possible operator: (binary refers to an operator that require two operand)
                    + 	binary addition operator
                  - 	binary subtraction operator
                  (-)	logical unary negative sign	(??? need to find)
                  *		binary multiplication operator
                  /		binary devision operator
                  (		open parenthesis
                  )		close parenthesis

   Return: 			The result of the calculation or NOT_DOUBLE (SNAN) for syntex error
   Complexity: 	o(n) n is the number of chars in calculation
   notes:				all operator need to be valid and have the right number of operands
                            do not divide by 0!!!
                all open paranthesis must be close
                each number can have up to 1 decimal point

                order of president: top to buttom L to R
                                ()
                (-)
                *	/
                + -
                */
double Calculate(char *calculation);

#endif /* __CALC_H__*/
