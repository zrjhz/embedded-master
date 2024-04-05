#include <stdint.h>
#include "stm32f4xx.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "calculator.h"
#include "debug.h"
#include "delay.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

void push(Stack *s, uint16_t value)
{
	if (s->top < STACK_SIZE)
		s->stack[s->top++] = value;
	else
		return;
}
uint16_t pop(Stack *s)
{
	if (s->top > 0)
		return s->stack[--s->top];
	else
		return 0;
}
int isOperator(char c)
{
	return c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^';
}
int getPrecedence(char c)
{
	switch (c)
	{
	case '+':
	case '-':
		return 1;
	case '*':
	case '/':
	case '%':
		return 2;
	case '^':
		return 3;
	default:
		return 0;
	}
}
void infixToPostfix(const char *infix, char *postfix)
{
	Stack stack;
	stack.top = 0;

	int i = 0;
	int j = 0;
	while (infix[i] != '\0')
	{
		char c = infix[i];
		if (isdigit(c))
			postfix[j++] = c;
		else if (isOperator(c))
		{
			while (stack.top > 0 && isOperator(stack.stack[stack.top - 1]) && getPrecedence(c) <= getPrecedence(stack.stack[stack.top - 1]))
				postfix[j++] = pop(&stack);
			push(&stack, c);
		}
		else if (c == '(')
			push(&stack, c);
		else if (c == ')')
		{
			while (stack.top > 0 && stack.stack[stack.top - 1] != '(')
				postfix[j++] = pop(&stack);

			if (stack.top > 0 && stack.stack[stack.top - 1] == '(')
				pop(&stack);
			else
				return;
		}
		else
			return;
		i++;
	}

	while (stack.top > 0)
	{
		if (stack.stack[stack.top - 1] == '(' || stack.stack[stack.top - 1] == ')')
			return;
		postfix[j++] = pop(&stack);
	}

	postfix[j] = '\0';
}
uint16_t evaluatePostfix(const char *postfix)
{
	Stack stack;
	stack.top = 0;

	int i = 0;
	while (postfix[i] != '\0')
	{
		if (isdigit(postfix[i]))
			push(&stack, postfix[i] - '0');
		else if (isOperator(postfix[i]))
		{
			uint16_t operand2 = pop(&stack);
			uint16_t operand1 = pop(&stack);
			uint16_t result;
			switch (postfix[i])
			{
			case '+':
				result = operand1 + operand2;
				break;
			case '-':
				result = operand1 - operand2;
				break;
			case '*':
				result = operand1 * operand2;
				break;
			case '/':
				result = operand1 / operand2;
				break;
			case '%':
				result = operand1 % operand2;
				break;
			case '^':
				result = (uint16_t)pow(operand1, operand2);
				break;
			}
			push(&stack, result);
		}
		else
			return 1;
		i++;
	}
	if (stack.top != 1)
		return 3;
	return pop(&stack);
}
void LetterAssign(char *data, uint16_t a, uint16_t b, uint16_t c)
{
	uint16_t Letter_1, Letter_2, Letter_3;

	Letter_1 = a;
	Letter_2 = b;
	Letter_3 = c;

	char *ptr = data;
	while (*ptr != '\0')
	{
		if (*ptr == 'a')
			*ptr = '0' + Letter_1;
		if (*ptr == 'b')
			*ptr = '0' + Letter_2;
		if (*ptr == 'c')
			*ptr = '0' + Letter_3;
		ptr++;
	}
}
