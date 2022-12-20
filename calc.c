/******************************************************************************
 * Project: 	2 STACK CALCULATOR
 * Author: 	    Eli Feinshtein
 * Date: 	   11/12/22
 * Version:	1.0
 *******************************************************************************/
#define _GNU_SOURCE /* math.h */
#include <stddef.h> /* size_t*/
#include <assert.h> /* assert */
#include <stdlib.h> /* malloc, free, strtod */
#include <math.h>   /* NAN */

#include "calc.h"
#include "utils.h"
#include "stack.h"

#define OPERLUTSIZE 128
#define SPACE ' '
#define ADD_SUB 0
#define MULT_DIV 1
#define OPENPAR 2
#define CLOSEPAR 3
#define NUM 4
#define PLUS '+'
#define MINUS '-'
#define MOVECHAR 1
#define OPENPARENT '('

/*****                           FWD DECLARATIONS                         *****/

/* loops over the string, removes spaces and counts numbers and operators
    if input is invalid returns FAIL, SUCCESS otherwise */
int Preprocessor(char *input, size_t *stack_sizes);

/*  creates calculator struct and related structs */
static calc_ty *CreateCalc(size_t *stack_sizes);
static void DestroyCalc(calc_ty *calc);         
static void InputLutInit();
static void CalculatePrev(calc_ty *calc);
int IsError(int tester, calc_ty *calc);

/* math functions */
static double Multiply(double num1, double num2);
static double Add(double num1, double num2);
static double Substract(double num1, double num2);
static double Divide(double num1, double num2);

int GetNumber(calc_ty *calc, char *input);
int PushOpen(calc_ty *calc, char *input);
int PushClose(calc_ty *calc, char *input);
int GetOperator(calc_ty *calc, char *input);
int ErrorFunc(calc_ty *calc, char *input);

/*****                          STRUCTS AND ENUMS                         *****/

struct calc
{
    stack_ty *operands;
    stack_ty *operators;
};

struct state
{
    int (*action_func)(calc_ty *calc, char *input);
    int next_state;
};

typedef struct operator
{
    int index;
    double (*oper_func)(double, double);
}
operator_ty;

enum ret_val
{
    SUCCESS = 0,
    FAIL = 1
};

enum cases
{
    START = 0,
    WAITNUMBER = 1,
    OPERATOR = 2,
    ERROR = -1
};

/*****                          GLOBAL VARIABLES                         *****/

/* state transition table */
static struct state cases_arr[5][3] = {
    /* + or - were entered */
    {{GetNumber, OPERATOR}, {ErrorFunc, ERROR}, {GetOperator, START}},
    /* * or / were entered */
    {{ErrorFunc, ERROR}, {ErrorFunc, ERROR}, {GetOperator, START}},
    /* ( was entered */
    {{PushOpen, START}, {PushOpen, START}, {ErrorFunc, ERROR}},
    /* ) was entered */
    {{ErrorFunc, ERROR}, {ErrorFunc, ERROR}, {PushClose, OPERATOR}},
    /* number was entered */
    {{GetNumber, OPERATOR}, {GetNumber, OPERATOR}, {ErrorFunc, ERROR}}};

/* operators loot table */
static operator_ty opers[OPERLUTSIZE];

/*********                         DEFINITIONS                      ***********/

static calc_ty *CreateCalc(size_t *stack_sizes)
{
    calc_ty *calc = NULL;

    /* allocate memory for calc struct */
    calc = (calc_ty *)malloc(sizeof(calc_ty));

    MALLOC_TEST(NULL == calc, NULL, "calc malloc failed", NULL, NULL);

    /* create the operands stack */
    calc->operands = StackCreate(stack_sizes[0], sizeof(double));

    MALLOC_TEST(NULL == calc->operands, NULL, "operands stack malloc failed",
                calc, NULL);

    /* create the operators stack */
    calc->operators = StackCreate(stack_sizes[1], sizeof(char));

    MALLOC_TEST(NULL == calc->operands, NULL, "operators stack malloc failed",
                calc->operands, calc);

    return calc;
}

static void DestroyCalc(calc_ty *calc)
{
    /* destroy operands stack */
    StackDestroy(calc->operands);

    /* destroy operators stack */
    StackDestroy(calc->operators);

    /* free calc struct */
    free(calc);
    calc = NULL;
}

double Calculate(char *input)
{
    calc_ty *calc = NULL;
    size_t stack_sizes[2] = {0, 0}; /* first operands, second operators*/
    int state = START;
    int offset = 0;
    double return_val = 0.0;

    assert(input && "invalid pointer to input");

    InputLutInit();

    /* create calc struct and its stacks */
    VALCHECK(Preprocessor(input, stack_sizes), FAIL, NAN)

    calc = CreateCalc(stack_sizes);

    VALCHECK(calc, NULL, NAN)

    /* loop through the input untill end or untill ERROR state */
    while (*input != '\0' && ERROR != state)
    {
        /* use the function in state transition table according to input */
        offset =
            (cases_arr[opers[(int)(*input)].index][state]).action_func(calc, input);

        /*update the state */
        state = (cases_arr[opers[(int)(*input)].index][state]).next_state;

        /* update the input pointer */
        input += offset;

        /* check if action function returned error */
        VALCHECK(offset, ERROR, NAN)
    }
    /* if we got to ERROR state - return NAN*/
    VALCHECK(IsError(state,calc), 1, NAN)

    /* make all calculations */
    while (!StackIsEmpty(calc->operators))
    {
        CalculatePrev(calc);
    }

    /* retrieve the calculated value - the only value in num stack*/
    return_val = *(double *)StackPeek(calc->operands);

    /* dont forget to destroy - for vlg*/
    DestroyCalc(calc);

    return return_val;
}

static void InputLutInit()
{
    int i = 0;
    /* init all lut to -1,-1, NULL */
    for (i = 0; i < OPERLUTSIZE; i++)
    {
        opers[i].index = -1;
        opers[i].oper_func = NULL;
    }

    /* init the operators */
    opers[40].index = OPENPAR;
    opers[40].oper_func = NULL;

    opers[41].index = CLOSEPAR;
    opers[41].oper_func = NULL;

    opers[42].index = MULT_DIV;
    opers[42].oper_func = Multiply;

    opers[43].index = ADD_SUB;
    opers[43].oper_func = Add;

    opers[45].index = ADD_SUB;
    opers[45].oper_func = Substract;

    opers[47].index = MULT_DIV;
    opers[47].oper_func = Divide;

    /* init numbers */
    for (i = 48; i < 58; i++)
    {
        opers[i].index = NUM;
        opers[i].oper_func = NULL;
    }
}

/******                                FSM Funcs                         ******/
int Preprocessor(char *input, size_t *stack_sizes) 
{
    int last_char_index = -1;
    int i = 0;

    stack_sizes[0] = 30;
    stack_sizes[1] = 30;

    while( '\0' != input[i])
    {
        ++i;
        if(SPACE == input[i])
        {
            return FAIL;
        }
    }

    last_char_index = opers[(int)input[i-1]].index;

    if( NUM !=  last_char_index &&  CLOSEPAR !=  last_char_index)
    {
        return FAIL;
    }

    return SUCCESS; 
}

int GetNumber(calc_ty *calc, char *input)
{
    /* save the starting pointer for return */
    char *end = input;

    /* extract the number from input */
    double num_to_insert = 0;

    num_to_insert = strtod(input, &end);

    /* push the extracted number to operands stack */
    StackPush(calc->operands, &num_to_insert);

    /* on invalid input return ERROR */
    return (end - input);
}

int GetOperator(calc_ty *calc, char *operator)
{
    /* evaluate the last operator in stack */
    char *prev_operator = (char *)StackPeek(calc->operators);
    char mult = '*';
    double sign = -1;
    int plus = '+';

    /* while cuurent operator's priority is lower than previous,
        or it didnt reach open parenthis - calculate and push result*/
    while(opers[(int)*operator].index <= opers[(int)*prev_operator].index
            &&  OPENPARENT != *(char *)StackPeek(calc->operators))
    {
        CalculatePrev(calc);
    }




    if('-' == *operator)
    {
        StackPush(calc->operators, &plus);
        StackPush(calc->operands, &sign);
        StackPush(calc->operators, &mult);
        return MOVECHAR;
    } 
    
    /* push the current operator to stack*/
    StackPush(calc->operators, operator);

    /* move pointer of input one step */
    return MOVECHAR;
}

int PushClose(calc_ty *calc, char *input)
{
    UNUSED(input);
    /* go back and calculate untill you meet an open parenthesis */
    while (OPENPARENT != *(char *)StackPeek(calc->operators))
    {
        CalculatePrev(calc);

        /* check if we reached start of input but not foud parenthes*/
        if(StackIsEmpty(calc->operators))
        {
            DestroyCalc(calc);
            return ERROR;
        }
    }

    /* pop the open paranthesis */
    StackPop(calc->operators);

    /* move pointer of input one step */
    return MOVECHAR;
}

int PushOpen(calc_ty *calc, char *parenthis)
{
    /* push the open parenthis in operators stack */
    StackPush(calc->operators, parenthis);

    /* move pointer of input one step */
    return MOVECHAR;
}

int ErrorFunc(calc_ty *calc, char *input)
{
    UNUSED(calc);
    UNUSED(input);
    return ERROR;
}

/******                           Calculation Funcs                      ******/

static double Multiply(double num1, double num2)
{
    return num1 * num2;
}

static double Add(double num1, double num2)
{
    return num1 + num2;
}

static double Substract(double num1, double num2)
{
    return num1 - num2;
}

static double Divide(double num1, double num2)
{
    return (num2 == 0) ? NAN : (num1 / num2);
}

static void CalculatePrev(calc_ty *calc)
{
    char *curr_operator = (char *)StackPeek(calc->operators);
    double prev_val = 0.0;
    double curr_val = 0.0;

    /* pop current value from operands stack */
    curr_val = *(double *)StackPeek(calc->operands);
    StackPop(calc->operands);

    /* pop previous value from operands stack */
    prev_val = *(double *)StackPeek(calc->operands);
    StackPop(calc->operands);

    /* calculate the new curr val*/
    curr_val = opers[(int)*curr_operator].oper_func(prev_val, curr_val);

    /* push the calculated value */
    StackPush(calc->operands, &curr_val);

    /* pop the previous operator */
    StackPop(calc->operators);
}

int IsError(int tester, calc_ty *calc)
{
    if(tester == ERROR)
    {
        DestroyCalc(calc);
        return FAIL;
    }

    return SUCCESS;
}

