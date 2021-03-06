/*
* File: eval.c
* ------------
* This file implements the eval.h interface.
*/

#include <stdio.h>
#include "genlib.h"
#include "strlib.h"
#include "exp.h"
#include "eval.h"
#include "symtab.h"
#include "env.h"

/* static variables */
static int recursions;

/* Private function prototypes */
static valueADT Eval(expADT exp, environmentADT env);
static valueADT EvalCompound(expADT exp, environmentADT env);
static valueADT EvalIfExp(expADT exp, environmentADT env);
static valueADT evalFunc(expADT exp, environmentADT env);
static valueADT evalCall(expADT exp, environmentADT env);
static valueADT EvalIdentifier(expADT exp, environmentADT env);

/* Exported entries */

valueADT Evaluating(expADT exp, environmentADT env){
	valueADT result;
	recursions = 0;
	return (Eval(exp, env));
}

static valueADT Eval(expADT exp, environmentADT env){	
	if (recursions > 1000){
		Error("Stack overflow.Too deep recursion.");
	}

	recursions++;

	switch (ExpType(exp)) {
		case ConstExp: {
			return (NewIntegerValue(ExpInteger(exp)));
		}
		case IdentifierExp: {
			return EvalIdentifier(exp, env);
		}
		case CompoundExp:
			return (EvalCompound(exp, env));
		case FuncExp:
			return (evalFunc(exp, NewClosure(env)));
		case IfExp:
			return (EvalIfExp(exp, NewClosure(env)));
		case CallExp:
			return (evalCall(exp, env));
		default:
			Error("Unidentified Evaluation");
	}
}

/* Private functions */

static valueADT EvalIdentifier(expADT exp, environmentADT env){
	expADT body;
	environmentADT closure;
	valueADT value;
	string ident;

	ident = ExpIdentifier(exp);
	
	value = GetIdentifierValue(env,ident);
	closure = GetFuncValueClosure(value);
	body = GetFuncValueBody(value);

	return Eval(body,closure);
}
static valueADT EvalCompound(expADT exp, environmentADT env)
{
	char op;
	int value;
	valueADT lhs, rhs;

	op = ExpOperator(exp);
	lhs = Eval(ExpLHS(exp), env);
	rhs = Eval(ExpRHS(exp), env);

		value = GetIntValue(rhs);

	switch (op) {
	case '+': return NewIntegerValue(GetIntValue(lhs) + value);
	case '-': return NewIntegerValue(GetIntValue(lhs) - value);
	case '*': return NewIntegerValue(GetIntValue(lhs) * value);
	case '/': 
		if (value != 0)
			return NewIntegerValue(GetIntValue(lhs) / value);
		else Error("Division by zero!");
	default:  Error("Illegal operator");
	}
}

static valueADT EvalIfExp(expADT exp, environmentADT env) {
	char op;
	int lhs, rhs;

	op = GetIfRelOp(exp);
	lhs = GetIntValue(Eval(GetIfLHSExpression(exp), env));	
	rhs = GetIntValue(Eval(GetIfRHSExpression(exp), env));

	switch (op) {
		case '<':
			if(lhs < rhs)
				return (Eval(GetIfThenPart(exp), env));
			else
				return (Eval(GetIfElsePart(exp), env));
		case '=':
			if(lhs == rhs)
				return (Eval(GetIfThenPart(exp), env));
			else
				return (Eval(GetIfElsePart(exp), env));
		case '>':
			if(lhs > rhs)
				return (Eval(GetIfThenPart(exp), env));
			else
				return (Eval(GetIfElsePart(exp), env));
		default:
			Error("Unknown operator");
	}
}

static valueADT evalCall(expADT exp, environmentADT env){
	string body;
	expADT arg ,func;
	valueADT funcValue;
	environmentADT newEnv;

	func = GetCallExp(exp);
	arg = GetCallActualArg(exp);
	funcValue = Eval(func, env);

	newEnv = GetFuncValueClosure(funcValue);
	body = GetFuncValueFormalArg(funcValue);

	DefineIdentifier(newEnv, body, arg, env);

	return Eval(GetFuncValueBody(funcValue), newEnv);
}

static valueADT evalFunc(expADT exp, environmentADT env){
	string arg;
	expADT body;

	arg = GetFuncFormalArg(exp);
	body = GetFuncBody(exp);

	return NewFuncValue(arg, body, env);
}