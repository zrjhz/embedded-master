#ifndef __CALCULATOR_H__
#define __CALCULATOR_H__

#include "sys.h"

#define STACK_SIZE 100
typedef struct
{
    int top;
    uint16_t stack[STACK_SIZE];
} Stack;
void push(Stack *s, uint16_t value);
uint16_t pop(Stack *s);
int isOperator(char c);
int getPrecedence(char c);
void infixToPostfix(const char *infix, char *postfix);
uint16_t evaluatePostfix(const char *postfix);
void LetterAssign(char *data, uint16_t a, uint16_t b, uint16_t c);

#endif
