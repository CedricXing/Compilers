//
// File Name: create_gramm_tree.h
// Created By: Cedric Xing
// Created On: 2017年03月25日 星期六 20时12分20秒
// Last Modified By: Cedric Xing
// Last Modified On: 2017年03月25日 星期六 20时12分20秒
// Copyright (c) 2017 Cedric Xing All rights Reserved.
//

#include "syntax.tab.h"

extern int yylineno;
extern char* yytext;

typedef enum {
	INTEGER,
	FL,
	ARRAY,
	STRUCTURE,
	BASIC
} Kind;

struct Node
{
	char *name;
	int line;
	int is_terminator;
	Kind kind;
	char *attribute;
	union{
		char *id_type;
		int value_int;
		float value_float;
	};
	struct Node *left_child;
	struct Node *right_child;
};

typedef struct Type_ Type;
typedef struct FieldList_ FieldList;
typedef struct FieldList_* Field;

struct Type_{
	Kind kind;
	union{
		int basic;
		struct{
			Type *element;
			int size;
		}array ;
		FieldList *structure;
	};
};

struct FieldList_{
	char *name;
	Type *type;
	FieldList *tail;
};

struct Symbol{
	char *sym_name;
	Type *tp;
	struct Symbol *next;
};

struct Function{
	Type *return_type;
	FieldList *args;
};

extern struct Symbol *symbolTable[16384];
extern struct Symbol *symbolTable1[16384];
extern struct Function *functionTable[16384];
extern int return_line;
extern int function_begin;
extern int struct_begin;

struct Node* create_node(char *name,int num,...);
void yyerror(const char *s);
int exist(int def_type,char *name);
int exist_struct(char *name);
int hash(char *name);
void new_symbol(int num,...);
void new_struct_symbol(int num,...);
void create_function(struct Node*,struct Node*);
void create_struct_type(FieldList **structure,struct Node *node);

int check_is_array(char *name);
int check_is_struct(char *name);
int has_field(char *name,char *field);
int exist_struct_type(char *name);
int exist_function(char *name);
int exist_var(char *name);
int check_return_type(struct Node*,struct Node*);
int check_same_type(Type *tp1,Type *tp2);
int check_args(char *name,struct Node*);
int check_op_types(struct Node*,struct Node*);
void check_assignop(struct Node*,struct Node*);

//check function
void check_array(char *name);
void check_struct(char *name);
void check_function(char *name);
//error output
void print_semantic_error1(int lineno,char *name);
void print_semantic_error2(int lineno,char *name);
void print_semantic_error3(int lineno,char *name);
void print_semantic_error4(char *name);
void print_semantic_error5(int lineno);
void print_semantic_error6(int lineno);
void print_semantic_error7(int lineno);
void print_semantic_error8();
void print_semantic_error9(int lineno,char *name);
void print_semantic_error10(int lineno,char *name);
void print_semantic_error11(int lineno,char *name);
void print_semantic_error12(int lineno,float value);
void print_semantic_error13(int lineno,char *name);
void print_semantic_error14(int lineno,char *name);
void print_semantic_error15(int lineno,char *name);
void print_semantic_error16(char *name);
void print_semantic_error17(int lineno,char *name);

//experiment 3
void add_read_function();
void add_write_function();

typedef struct Operand_ Operand;

struct Operand_{
	enum {VARIABLE,CONSTANT,ADDRESS} kind;
	union{
		int var_no;
		int value;
	} u;
} ;

struct InterCode{
	enum { ASSIGN,ADD,SUB,MUL} kind;
	union{
		struct {Operand* right,left;} assign;
		struct {Operand* result,op1,op2;} binop;
	} u;
};

