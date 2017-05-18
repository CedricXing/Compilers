#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "create_gramm_tree.h"

struct InterCodes *head = NULL;
int arg_no = 0;
int var_no = 0;

//preorder traverse
void generate_InterCodes(struct Node *node){
	if(node==NULL)
		return;
	if(strcmp(node->name,"Program")==0){
		generate_InterCodes(node->left_child);//ExtDefList
	}
	else if(strcmp(node->name,"ExtDefList")==0 && node->left_child!=NULL){
		generate_InterCodes(node->left_child);
		generate_InterCodes(node->left_child->right_child);
	}
	else if(strcmp(node->name,"ExtDef")==0){
		if(strcmp(node->left_child->right_child->name,"FunDec")==0){
			struct InterCodes* code1 = generate_function(node->left_child->right_child->attribute);
			struct InterCodes* code2 = translate_CompSt(node->left_child->right_child->right_child);
			if(head == NULL){
				head = code1;
				cat_ir(head,code2);
			}
			else{
				cat_ir(head,code1);
				cat_ir(head,code2);
			}
			
		}
	}
}

//generate all kinds of InterCodes
struct InterCodes* generate_function(char *function_name){
	int hashcode = hash(function_name);
	struct InterCodes* ir1 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	ir1->code.kind = FUNCTION;
	ir1->code.u.function_name=function_name;
	ir1->next=NULL;
	if(functionTable[hashcode]->args!=NULL){
		FieldList *fl = functionTable[hashcode]->args;
		struct InterCodes* temp = ir1;
		while(fl != NULL){
			struct InterCodes* temp_ir = (struct InterCodes*)malloc(sizeof(struct InterCodes));
			temp_ir->code.kind = PARAM;
			temp_ir->code.u.arg_no=(var_no++);
			temp_ir->next = NULL;
			temp->next = temp_ir;
			temp=temp_ir;
			fl = fl->tail;
		}
	}
	return ir1;
}

struct InterCodes* generate_Label(int label){
	//struct InterCodes = 
	return NULL;
}

struct InterCodes* generate_assign_id_constant(int value,int place){
	struct InterCodes* ir = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	ir->next = NULL;
	ir->code.kind = ASSIGN;
	ir->code.u.assign.left.kind=VARIABLE;
	ir->code.u.assign.left.u.var_no = place;
	ir->code.u.assign.right.kind = CONSTANT;
	ir->code.u.assign.right.u.value = value;
	return ir;
}

struct InterCodes* generate_assign_double_id(int place1,int place2){
	struct InterCodes* ir = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	ir -> next = NULL;
	ir -> code.kind = ASSIGN;
	ir -> code.u.assign.left.kind = VARIABLE;
	ir -> code.u.assign.left.u.var_no = place1;
	ir -> code.u.assign.right.kind = VARIABLE;
	ir -> code.u.assign.right.u.var_no = place2;
	return ir;
}

//translation
struct InterCodes* translate_CompSt(struct Node *CompSt){
	struct InterCodes* code1 = translate_DefList(CompSt->left_child->right_child);
	struct InterCodes* code2 = translate_StmtList(CompSt->left_child->right_child->right_child);
	if(code1 == NULL){
		code1 = code2;
	}
	else cat_ir(code1,code2);
	return code1;
}

struct InterCodes* translate_DefList(struct Node *DefList){
	return NULL;
}

struct InterCodes* translate_StmtList(struct Node *StmtList){
	struct Node *st = StmtList;
	struct InterCodes* ir = NULL;
	while(st->left_child != NULL){
		if(ir == NULL) ir = translate_Stmt(st->left_child);
		else cat_ir(ir,translate_Stmt(st->left_child));
		st = st -> left_child->right_child;
	}
	return ir;
}

struct InterCodes* translate_Stmt(struct Node *Stmt){
	if(strcmp(Stmt->left_child->name,"Exp")==0){
		return translate_exp(Stmt->left_child,-1);
	}
	else if(strcmp(Stmt->left_child->name,"CompSt")==0){
		return translate_CompSt(Stmt->left_child);
	}
	return NULL;
	//to be continued
}

struct InterCodes* translate_exp(struct Node* exp,int place){
	if(strcmp(exp->left_child->name,"INT")==0){
		return generate_assign_id_constant(exp->left_child->value_int,place);
	}
	else if(strcmp(exp->left_child->name,"ID")==0){
		int hashcode = hash(exp->left_child->attribute);
		if(symbolTable[hashcode]->var_no == -1){
		symbolTable[hashcode]->var_no = var_no++;
		}
		return generate_assign_double_id(place,symbolTable[hashcode]->var_no);
	}
	else if(strcmp(exp->left_child->name,"Exp")==0&&strcmp(exp->left_child->right_child->name,"ASSIGNOP")==0){
		if(strcmp(exp->left_child->left_child->name,"ID")==0){
			int hashcode = hash(exp->left_child->attribute);
			int temp_no = var_no++;
			if(symbolTable[hashcode]->var_no == -1){
				symbolTable[hashcode]->var_no = var_no++;
			}
			struct InterCodes* code1 = translate_exp(exp->left_child->right_child->right_child,temp_no);
			struct InterCodes* code2 = generate_assign_double_id(symbolTable[hashcode]->var_no,temp_no);
			cat_ir(code1,code2);
			if(place != -1){
				code2 = generate_assign_double_id(place,temp_no);
				cat_ir(code1,code2);
			}
			return code1;
		}
	}
	return NULL;
}

void output_InterCodes(){
	FILE *fp = fopen("i.ir","w");
	if(fp == NULL){
		printf("file open error!");
		return ;
	}
	struct InterCodes* ir = head;
	while(ir != NULL){
		if(ir->code.kind == FUNCTION){
			fprintf(fp,"FUNCTION %s :\n",ir->code.u.function_name);
			printf("FUNCTION %s :\n",ir->code.u.function_name);
		}
		else if(ir->code.kind == PARAM){
			fprintf(fp, "PARAM %s%d\n","t",ir->code.u.arg_no);
			printf("PARAM %s%d\n","t",ir->code.u.arg_no);
		}
		else if(ir->code.kind == ASSIGN){
			if(ir->code.u.assign.right.kind == CONSTANT){
				fprintf(fp,"t%d := #%d\n",ir->code.u.assign.left.u.var_no,ir->code.u.assign.right.u.value);
				printf("t%d := #%d\n",ir->code.u.assign.left.u.var_no,ir->code.u.assign.right.u.value);
			}
			else if(ir->code.u.assign.right.kind == VARIABLE){
				fprintf(fp, "t%d := t%d\n",ir->code.u.assign.left.u.var_no,ir->code.u.assign.right.u.var_no);
				printf("t%d := t%d\n",ir->code.u.assign.left.u.var_no,ir->code.u.assign.right.u.var_no);
			}
		}
		ir = ir->next;
	}
	fclose(fp);
}

void cat_ir(struct InterCodes *code1,struct InterCodes *code2){
	struct InterCodes *tp = code1;
	while(tp ->next != NULL){
		tp = tp->next;
	}
	tp->next = code2;
}