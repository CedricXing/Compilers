//
// File Name: create_gramm_tree.c
// Created By: Cedric Xing
// Created On: 2017年03月25日 星期六 20时12分02秒
// Last Modified By: Cedric Xing
// Last Modified On: 2017年03月25日 星期六 20时12分08秒
// Copyright (c) 2017 Cedric Xing All rights Reserved.
//


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "create_gramm_tree.h"

struct Symbol *symbolTable[16384];
struct Symbol *symbolTable1[16384];
struct Function *functionTable[16384];
int is_initialized = 0;
int return_line = 0;
int function_begin = 0;
int struct_begin = 0;

struct Node* create_node(char *name,int num,...){
	va_list valist;
	struct Node *p_node = (struct Node*)malloc(sizeof(struct Node));
	p_node -> left_child = NULL;
	p_node -> right_child = NULL;
	p_node -> attribute = NULL;

	if(p_node == NULL){
		yyerror("create node failed.");
		exit(-1);
	}
	p_node -> name = name;
	p_node -> is_terminator = 0;
	va_start(valist,num);

	if(num == 0){
		int arg = va_arg(valist,int);
		p_node -> line = arg;
		if(p_node -> line != -1)//zhong jie fu
			p_node -> is_terminator = 1;
		if(strcmp(name,"ID") == 0 || strcmp(name,"TYPE") == 0 || strcmp(name,"RELOP") == 0){
			p_node -> attribute = (char*)malloc(sizeof(char) * 30);
			strcpy(p_node -> attribute,yytext);
		}
		else if(strcmp(name,"INT") == 0){
			p_node -> kind = INTEGER;
			p_node -> value_int = atoi(yytext);
		}
		else if(strcmp(name,"FLOAT") == 0){
			p_node -> kind = FL;
			p_node -> value_float = atof(yytext);
		}
	}
	else if(num > 0){
		//struct Node *temp_node = (Node*)malloc(sizeof(struct Node));
		struct Node *temp = va_arg(valist,struct Node*);
		p_node -> left_child = temp;
		p_node -> line = temp -> line;

		if(num == 1){//only one child   shu xing zhi xiang shang chuan di
			p_node -> kind = temp -> kind;
			p_node -> attribute = temp -> attribute;
		}
		for(int i = 0;i < num - 1;++ i){
			temp -> right_child = va_arg(valist,struct Node*);
			temp = temp -> right_child;
		}
	}

	return p_node;

}

void preorder_traversal(struct Node* p_node,int depth){
	if(p_node == NULL)
		return;
	if(p_node -> line != -1){//kong chuan bu xu yao da yin
		for(int i = 0;i < depth;++ i)
			printf("  ");
		printf("%s",p_node -> name);
		if(strcmp(p_node -> name,"ID") == 0 || strcmp(p_node -> name,"TYPE") == 0)
			printf(": %s",p_node -> id_type);
		else if(strcmp(p_node -> name,"INT") == 0)
			printf(": %d",p_node -> value_int);
		else if(strcmp(p_node -> name,"FLOAT") == 0)
			printf(": %f",p_node -> value_float);
		else if (!p_node -> is_terminator) printf(" (%d)",p_node -> line);
		printf("\n");
	}
	preorder_traversal(p_node -> left_child,depth + 1);
	preorder_traversal(p_node -> right_child,depth);

}

void yyerror(const char *s) 
{
    printf("Error type B at Line %d:",yylineno);
    printf(" %s\n",s);
}

int exist(int def_type,char *name){
	if(!is_initialized){
		add_read_function();
		add_write_function();
		for(int i = 0;i < 16384;++ i){
			symbolTable[i] = NULL;
			symbolTable1[i] = NULL;
		}
		is_initialized = 1;
	}


	int hashcode = hash(name);
/*
	if(symbolTable[hashcode] == NULL && symbolTable1[hashcode] == NULL && functionTable[hashcode] == NULL)
			return 0;
	else return 1;
*/

	if(def_type == 1){//basic type consider conditions like int i,j
			if(symbolTable[hashcode] == NULL && symbolTable1[hashcode] == NULL){//not used yet
				return 0;
			}
			else{
				return 1;
			}
	}
	else if(def_type == 2){
		if(functionTable[hashcode] == NULL)
			return 0;
		else return 1;
	}


	return 1;
}

int exist_struct(char *name){
	int hashcode = hash(name);
	if(symbolTable1[hashcode] == NULL && symbolTable[hashcode] == NULL)
		return 0;
	else return 1;
}

int hash(char *name){
	int val = 0, i;
	for (; *name; ++name){
		val = (val << 2) + *name;
		if (i = val & ~0x3fff) val = (val ^ (i >> 12)) & 0x3fff;
	}
	return val;
}

//type:1 for val,3 for ARRAY

void new_symbol(int num,...){
	va_list valist;
	struct Symbol *sy = (struct Symbol*)malloc(sizeof(struct Symbol));
	sy->var_no = -1;
	sy->is_arg_array = 0;
	va_start(valist,num);
	int def_type = va_arg(valist,int);
	
	if(def_type == 1){//val definiton	
		sy->tp = (Type*)malloc(sizeof(Type));
		struct Node *temp=(struct Node*)malloc(sizeof(struct Node));
		temp = va_arg(valist, struct Node*);
		if(strcmp(temp->attribute,"int") == 0)
			sy->tp->kind = INTEGER;
		else if(strcmp(temp->attribute,"float") == 0)
			sy->tp->kind = FL;
		else {//struct
			int hashcode = hash(temp->attribute);
			sy->tp = symbolTable1[hashcode]->tp;
		}
		temp = va_arg(valist, struct Node*);
		sy->sym_name = temp->attribute;

		int hashcode = hash(temp -> attribute);
		//printf("%s\n",temp->attribute);
		if(symbolTable[hashcode] == NULL)
			symbolTable[hashcode] = sy;
		else printf("open hashing error!\n");
	}
	else if(def_type == 3){//array difinition
		sy->tp = (Type*)malloc(sizeof(Type));
		//sy->tp->kind = ARRAY;
		struct Node *Specifier_node=(struct Node*)malloc(sizeof(struct Node));
		Specifier_node=va_arg(valist, struct Node*);
		struct Node *Dec_node = va_arg(valist, struct Node*);
		sy->sym_name = Dec_node->attribute;
		struct Node *temp;
		if(strcmp(Dec_node->name,"VarDec") == 0)
			temp = Dec_node;
		else temp = Dec_node->left_child;
		//Type *tp_temp = sy->tp->array.element = (Type*)malloc(sizeof(Type));
		Type *tp_temp = sy->tp;

		int dimen = 0;
		int dimens[20];
		while(strcmp(temp->left_child->name,"ID")!=0){
			temp=temp->left_child;
			dimens[dimen++] = temp->right_child->right_child->value_int;
		}

		/*
		while(dimen > 0){//Multidimensional array
			tp_temp -> kind = ARRAY;
			tp_temp -> array.size = dimens[dimen-1];
			-- dimen;
			if(dimen > 0){
				tp_temp -> array.element = (Type*)malloc(sizeof(Type));
				tp_temp = tp_temp -> array.element;
			}
		}
		*/
		while(dimen > 0){//Multidimensional array
			tp_temp -> array.size = dimens[dimen-1];
			tp_temp -> kind = ARRAY;
			-- dimen;
			if(dimen > 0){
				tp_temp -> array.element = (Type*)malloc(sizeof(Type));
				tp_temp = tp_temp -> array.element;
			}
		}

		if(strcmp(Specifier_node->attribute,"int")==0){
			tp_temp -> array.element = (Type*)malloc(sizeof(Type));
			tp_temp->array.element->kind = INTEGER;	
		}
		else if(strcmp(Specifier_node->attribute,"float") == 0){
			tp_temp -> array.element = (Type*)malloc(sizeof(Type));
			tp_temp->array.element->kind = FL;
		}
		else {//struct 
			int hashcode = hash(Specifier_node->attribute);
			tp_temp->array.element = symbolTable1[hashcode]->tp;
		}

		int hashcode = hash(Dec_node->attribute);
		if(symbolTable[hashcode] == NULL){
			symbolTable[hashcode] = sy;
		}
		else printf("open hashing error!\n");

		/*check whether array definition is processed correctly*/
		//check_array(Dec_node->attribute);
	}

	/*
	for(int i = 0;i < 16384;++ i){
		if(symbolTable[i] != NULL){
			printf("%d\n",i);
			//printf("%s\n",symbolTable[i]->sym_name);
			if(symbolTable[i]->tp->kind == INTEGER)
				printf("int %s\n",symbolTable[i]->sym_name);
			else if(symbolTable[i]->tp->kind == FL)
				printf("float %s\n",symbolTable[i]->sym_name);
			else if(symbolTable[i]->tp->kind == ARRAY){
				check_array(symbolTable[i]->sym_name);
			}
			else if(symbolTable[i]->tp->kind == STRUCTURE){
				check_struct(symbolTable[i]->sym_name);
			}
		}
	}
	printf("\n\n\n\n\n");
	*/
}

void new_struct_symbol(int num,...){
	va_list valist;
	struct Symbol *sy = (struct Symbol*)malloc(sizeof(struct Symbol));
	va_start(valist,num);

	sy->tp = (Type*)malloc(sizeof(Type));
	sy->tp->kind = STRUCTURE; 
	struct Node *temp=(struct Node*)malloc(sizeof(struct Node));
	temp = va_arg(valist, struct Node*);
	sy->sym_name = temp -> attribute;
	sy->tp->structure = NULL;

	temp = va_arg(valist,struct Node*);
	create_struct_type(&sy->tp->structure,temp);

	//printf("hello\n");
	int hashcode = hash(sy->sym_name);
	if(symbolTable1[hashcode] == NULL)
		symbolTable1[hashcode] = sy;
	else printf("open hashing error!\n");
}

void create_function(struct Node *Specifier_node,struct Node *FunDec_node){
	struct Function* fc = (struct Function*)malloc(sizeof(struct Function));

	if(strcmp(Specifier_node->left_child->name,"TYPE")==0){//basic types
		fc->return_type = (Type*)malloc(sizeof(Type));
		if(strcmp(Specifier_node->left_child->attribute,"int")==0)
			fc->return_type->kind = INTEGER;
		else
			fc->return_type->kind = FL;
	}
	else{
		int hashcode = hash(Specifier_node->left_child->attribute);
		fc->return_type = symbolTable1[hashcode]->tp;
	}

	if(strcmp(FunDec_node->left_child->right_child->right_child->name,"VarList")==0){//function has arguments
		struct Node *VarList_node = FunDec_node->left_child->right_child->right_child;
		struct Node *temp = VarList_node->left_child;//ParaDec node
		FieldList **fd = &(fc->args);
		while(1){
			struct Node *VarDec_node = temp->left_child->right_child;
			//printf("%s\n",VarDec_node->attribute);
			int hashcode = hash(VarDec_node->attribute);
			
			*fd = (FieldList*)malloc(sizeof(FieldList));
			(*fd)->name = symbolTable[hashcode]->sym_name;
			(*fd)->type = symbolTable[hashcode]->tp;
			(*fd)->tail = NULL;
			if(temp->right_child == NULL)
				break;
			else{
				temp = temp->right_child->right_child->left_child;
				fd = &((*fd)->tail);
			}
		}
	}

	int hashcode = hash(FunDec_node->left_child->attribute);
	if(functionTable[hashcode] == NULL){
		functionTable[hashcode] = fc;
		//check_function(FunDec_node->left_child->attribute);
	}
	else{
		printf("open hash error!\n");
	}
}

void check_function(char *name){
	int hashcode = hash(name);
	printf("Function : %s \n",name);
	printf("Return type : ");
	if(functionTable[hashcode]->return_type->kind == INTEGER)
		printf("int\n");
	else if(functionTable[hashcode]->return_type->kind == FL)
		printf("float\n");
	else if(functionTable[hashcode]->return_type->kind == ARRAY);
		//check_array(functionTable[hashcode]->return_type);
	else if(functionTable[hashcode]->return_type->kind == STRUCTURE);
		//check_struct(functionTable[hashcode]->return_type);

	FieldList *fd = functionTable[hashcode]->args;
	printf("Args: ");
	while(fd!= NULL){
		printf("%s ",fd->name);
		if(fd->type->kind == INTEGER)
			printf("int\n");
		else if(fd->type->kind == FL)
			printf("float\n");
		else if(fd->type->kind == ARRAY)
			check_array(fd->name);
		else if(fd->type->kind == STRUCTURE)
			check_struct(fd->name);
		fd = fd->tail;
	}
}

void check_array(char *name){
	int hashcode = hash(name);
	printf("%s",symbolTable[hashcode]->sym_name);
	Type *tp = symbolTable[hashcode]->tp;
	while(1){
		if(tp -> kind == ARRAY){
			printf("[%d]",tp->array.size);
		}
		else if(tp->kind == STRUCTURE){
			//printf("hello\n");
			FieldList *structure = tp->structure;

			while(structure != NULL){
			if(structure -> type->kind == INTEGER)
				printf("int %s\n",structure->name);
			else if(structure->type->kind == FL)
				printf("float %s\n",structure->name);
			else if(structure->type->kind == ARRAY)
				check_array(structure->name);
			else if(structure->type->kind == STRUCTURE)
				check_struct(structure->name);
			structure = structure->tail;
			}
			break;
		}
		else{
			printf("%d",tp->kind);
			break;
		}
		tp = tp->array.element;
		
	}
	printf("\n");
}

void check_struct(char *name){

	int hashcode = hash(name);
	printf("%s \n",name);
	FieldList *structure = symbolTable[hashcode]->tp->structure;

	while(structure != NULL){
		if(structure -> type->kind == INTEGER)
			printf("int %s\n",structure->name);
		else if(structure->type->kind == FL)
			printf("float %s\n",structure->name);
		else if(structure->type->kind == ARRAY)
			check_array(structure->name);
		else if(structure->type->kind == STRUCTURE)
			check_struct(structure->name);
		structure = structure->tail;
	}
}

int count = 0;

void create_struct_type(FieldList **structure,struct Node *node){//the node always be a DefList node
	if(node->left_child == NULL)
		return;
	//printf("hello");
	//preorder_traversal(node,0);
	++ count;
	struct Node *Def_node = node -> left_child;
	struct Node *Specifier_node = Def_node -> left_child;

	//if(strcmp(Specifier_node->left_child->name,"TYPE") == 0){//Basic type
		struct Node *DecList_node = Specifier_node->right_child;
		struct Node *Dec_node = DecList_node->left_child;
		while(1){//definition like int i,j
			*structure = (FieldList*)malloc(sizeof(FieldList));
			(*structure)->tail = NULL;

			(*structure)->type = (Type*)malloc(sizeof(Type));
			
			int hashcode = hash(Dec_node -> attribute);
			if(strcmp(Specifier_node->attribute,"int") == 0 || strcmp(Specifier_node->attribute,"float") == 0){
				//printf("%s\n",Specifier_node->attribute);
				(*structure)->type = symbolTable[hashcode]->tp;
			}
			else{
				int hashcode = hash(Dec_node->attribute);//must be Dec_node,mustn't be Specifier_node,because they may be different
				//printf("hello\n");
				//check_array("b");
				//printf("%s\n",Specifier_node->attribute);
				(*structure)->type = symbolTable[hashcode]->tp;
			}
			struct Node *VarDec_node = Dec_node->left_child;
			(*structure)->name = Dec_node->attribute;

			structure = &((*structure)->tail);
			
			if(Dec_node->right_child != NULL)
				Dec_node = Dec_node->right_child->right_child->left_child;
			else break;
		}

	//process other areas in recursive way
	struct Node *DefList_node = Def_node -> right_child;
	create_struct_type(structure,DefList_node);
}

int check_is_array(char *name){
	int hashcode = hash(name);
	if(symbolTable[hashcode] == NULL)
		return 0;
	else if(symbolTable[hashcode]->tp->kind != ARRAY)
		return 0;
	else return 1;
}

int check_is_struct(char *name){
	int hashcode = hash(name);
	if(symbolTable[hashcode] == NULL)
		return 0;
	else if(symbolTable[hashcode]->tp->kind != STRUCTURE)
		return 0;
	else return 1;
}

int has_field(char *name,char *field){
	int hashcode = hash(name);
	FieldList *structure = symbolTable[hashcode]->tp->structure;

	while(structure != NULL){
		if(strcmp(structure->name,field)==0)
			return 1;
		structure = structure->tail;
	}
	return 0;
}

int exist_struct_type(char *name){
	int hashcode = hash(name);
	if(symbolTable1[hashcode] == NULL)
		return 0;
	else return 1;
}

int exist_function(char *name){
	int hashcode = hash(name);
	if(functionTable[hashcode] == NULL)
		return 0;
	else return 1;
}

int exist_var(char *name){
	int hashcode = hash(name);
	if(symbolTable[hashcode] == NULL && symbolTable1[hashcode] == NULL)
		return 0;
	else return 1;
}

int check_return_type(struct Node *Specifier_node,struct Node *CompSt_node){
	if(strcmp(Specifier_node->left_child->name,"TYPE")==0){//basic types
		//printf("%s\n",CompSt_node->attribute);
		if(CompSt_node->attribute==NULL){
			if(CompSt_node->kind == 0 && strcmp(Specifier_node->left_child->attribute,"int")==0){
				return 1;
			}
			else if(CompSt_node->kind == 1 && strcmp(Specifier_node->left_child->attribute,"float")==0)
				return 1;
			else {
				return 0;
			}
		}
		else{
			int hashcode = hash(CompSt_node->attribute);
			//printf("%s\n",CompSt_node->attribute);
			//printf("%d %d\n",Specifier_node->left_child->kind,symbolTable[hashcode]->tp->kind);
			if(symbolTable[hashcode]!=NULL){
				if(symbolTable[hashcode]->tp->kind == 0 && strcmp(Specifier_node->left_child->attribute,"int")==0){
				return 1;
				}
				else if(symbolTable[hashcode]->tp->kind == 1 && strcmp(Specifier_node->left_child->attribute,"float")==0)
					return 1;
				else {
					return 0;
				}
			}
			else return 0;
			
		}
	}
	else{//struct types
		if(CompSt_node->attribute==NULL)
			return 0;
		else{
			int hashcode = hash(CompSt_node->attribute);
			int hashcode1 = hash(Specifier_node->attribute);
			return check_same_type(symbolTable[hashcode]->tp,symbolTable1[hashcode1]->tp);
		}
		
	}
}

int check_args(char *name,struct Node *Args_node){
	int hashcode = hash(name);
	if(Args_node == NULL){
		if(functionTable[hashcode]->args == NULL)
			return 1;
		else return 0;
	}
	else{
		if(functionTable[hashcode]->args == NULL)
			return 0;
		else{
			FieldList *fd = functionTable[hashcode]->args;
			struct Node *temp = Args_node;
			while(1){
				if(strcmp(temp->left_child->left_child->name,"INT")==0){
					//printf("hello\n");
					if(fd->type->kind != INTEGER)
						return 0;
				}
				else if(strcmp(temp->left_child->left_child->name,"FLOAT")==0){
					if(fd->type->kind != FL)
						return 0;
				}
				else{//ID
					int hashcode1 = hash(temp->left_child->attribute);
					if(symbolTable[hashcode1] == NULL || !check_same_type(symbolTable[hashcode1]->tp,fd->type))
						return 0;
				}
				fd = fd->tail;
				if(fd == NULL){
					if(temp->left_child->right_child != NULL)
						return 0;
					else
						return 1;
				}
				else{
					if(temp -> left_child->right_child == NULL)
						return 0;
					else
						temp = temp->left_child->right_child->right_child;
				}
			}
		}
	}
}

int check_same_type(Type *tp1,Type *tp2){
	if(tp1->kind != tp2->kind){
		return 0;
	}
	else{
		if(tp1->kind == INTEGER || tp1->kind == FL)
			return 1;
		else if(tp1->kind == ARRAY){
			return (check_same_type(tp1->array.element,tp2->array.element) && tp1->array.size == tp2->array.size);
		}
		else if(tp1->kind == STRUCTURE){
			FieldList *temp1 = tp1->structure;
			FieldList *temp2 = tp2->structure;
			while(temp1 != NULL && temp2 != NULL){
				//printf("%d %d\n",temp1->type->kind,temp2->type->kind);
				if(!check_same_type(temp1->type,temp2->type))
					return 0;
				temp1 = temp1->tail;
				temp2 = temp2->tail;
			}
			if(temp1 == NULL && temp2 == NULL)
				return 1;
			else return 0;
		}
	}
}

int check_op_types(struct Node *exp1,struct Node *exp2){
	if(strcmp(exp1->left_child->name,"INT")==0){
		if(strcmp(exp2->left_child->name,"INT")==0)
			return 1;
		else if(strcmp(exp2->left_child->name,"ID")==0){
			int hashcode = hash(exp2->left_child->attribute);
			if(symbolTable[hashcode]==NULL)
				return 0;
			if(symbolTable[hashcode]->tp->kind == INTEGER)
				return 1;
			else return 0;
		}
		else return 0;//indeed there still are some situations to be considered
	}
	else if(strcmp(exp1->left_child->name,"FLOAT")==0){
		if(strcmp(exp2->left_child->name,"FLOAT")==0)
			return 1;
		else if(strcmp(exp2->left_child->name,"ID")==0){
			int hashcode = hash(exp2->left_child->attribute);
			if(symbolTable[hashcode]==NULL)
				return 0;
			if(symbolTable[hashcode]->tp->kind == FL)
				return 1;
			else return 0;//indeed there still are some situations to be considered
		}
		else return 0;
	}
	/*
	else if(strcmp(exp1->left_child->name,"ID") == 0){
		int hashcode = hash(exp1->left_child->attribute);
		if(symbolTable[hashcode]==NULL)
			return 0;
		if(symbolTable[hashcode]->tp->kind != INTEGER && symbolTable[hashcode]->tp->kind != FL)
			return 0;
		if(strcmp(exp2->left_child->name,"INT")==0 && symbolTable[hashcode]->tp->kind == INTEGER)
			return 1;
		else if(strcmp(exp2->left_child->name,"FLOAT")==0 && symbolTable[hashcode]->tp->kind == FL)
			return 1;
		else if(strcmp(exp2->left_child->name,"ID") == 0){
			int hashcode1 = hash(exp2->left_child->attribute);
			if(symbolTable[hashcode1] == NULL)
				return 0;
			return (symbolTable[hashcode]->tp->kind == symbolTable[hashcode1]->tp->kind);
		}
	}
	else{
		return 1;
	}*/
	else if(exp1->attribute != NULL){
		int hashcode = hash(exp1->attribute);
		if(symbolTable[hashcode]==NULL)
			return 0;
		if(symbolTable[hashcode]->tp->kind != INTEGER && symbolTable[hashcode]->tp->kind != FL)
			return 0;
		if(strcmp(exp2->left_child->name,"INT")==0 && symbolTable[hashcode]->tp->kind == INTEGER)
			return 1;
		else if(strcmp(exp2->left_child->name,"FLOAT")==0 && symbolTable[hashcode]->tp->kind == FL)
			return 1;
		else if(exp2->attribute!=NULL){
			int hashcode1 = hash(exp2->attribute);
			if(symbolTable[hashcode1]==NULL)
				return 0;
			return (symbolTable[hashcode]->tp->kind==symbolTable[hashcode1]->tp->kind);
		}
	}
}

void check_assignop(struct Node *exp1,struct Node *exp2){
	int hashcode;
	if(strcmp(exp1->left_child->name,"ID") != 0){
		if(exp1->attribute!=NULL)
			hashcode=hash(exp1->attribute);
		else {
			print_semantic_error6(yylineno);
			return;
		}
	}
	else hashcode = hash(exp1->left_child->attribute);
	if(symbolTable[hashcode]==NULL)
		return;
	if(strcmp(exp2->left_child->name,"INT") == 0){
		if(symbolTable[hashcode]->tp->kind != INTEGER){
			print_semantic_error5(yylineno);
		}
	}
	else if(strcmp(exp2->left_child->name,"FLOAT") == 0){
		if(symbolTable[hashcode]->tp->kind != FL){
			print_semantic_error5(yylineno);
		}
	}
	else if(strcmp(exp2->left_child->name,"ID") == 0){
		int hashcode1 = hash(exp2->left_child->attribute);
		if(symbolTable[hashcode1] == NULL)
			return;
		if(!check_same_type(symbolTable[hashcode]->tp,symbolTable[hashcode1]->tp)){
			print_semantic_error5(yylineno);
		}
	}
	else if(exp2->attribute!=NULL){
		int hashcode1 = hash(exp2->attribute);
		if(symbolTable[hashcode1]==NULL)
			return;
		if(!check_same_type(symbolTable[hashcode]->tp,symbolTable[hashcode1]->tp)){
			print_semantic_error5(yylineno);
		}
	}
}

int exist_same_field(struct Node *DefList_node){
	
}

//error output
void print_semantic_error1(int lineno,char *name){
	printf("Error type 1 at Line %d: Undefined Variable '%s'.\n",lineno,name);
}

void print_semantic_error2(int lineno,char *name){
	printf("Error type 2 at Line %d: Undefined function '%s'.\n",lineno,name);
}

void print_semantic_error3(int lineno,char *name){
	printf("Error type 3 at Line %d: Redefined Variable '%s'\n",lineno,name);
}

void print_semantic_error4(char *name){
	printf("Error type 4 at Line %d: Redefined function '%s'.\n",function_begin,name);
}

void print_semantic_error5(int lineno){
	printf("Error type 5 at Line %d: Type mismatched for assignment.\n",lineno);
}

void print_semantic_error6(int lineno){
	printf("Error type 6 at Line %d: The left-hand side of an assignment must be a Variable.\n",lineno);
}

void print_semantic_error7(int lineno){
	printf("Error type 7 at line %d: Type mismatched for operands.\n",lineno);
}

void print_semantic_error8(){
	printf("Error type 8 at Line %d: Type mismatched for return.\n",return_line);
}

void print_semantic_error9(int lineno,char *name){
	printf("Error type 9 at Line %d: Function call to %s is not correct.\n",lineno,name );
}

void print_semantic_error10(int lineno,char *name){
	printf("Error type 10 at Line %d: '%s' is not an array.\n",lineno,name);
}

void print_semantic_error11(int lineno,char *name){
	printf("Error type 11 at Line %d: '%s' is not a function.\n",lineno,name);
}

void print_semantic_error12(int lineno,float value){
	printf("Error type 12 at Line %d: '%f' is not an integer.\n",lineno,value);
}

void print_semantic_error13(int lineno,char *name){
	printf("Error type 13 at Line %d: Illegal use of '.'\n",lineno);
}

void print_semantic_error14(int lineno,char *name){
	printf("Error type 14 at Line %d: Non-existent field '%s'.\n",lineno,name);
}

void print_semantic_error15(int lineno,char *name){
	printf("Error type 15 at Line %d: Redefined field '%s'.\n",lineno,name);
}

void print_semantic_error16(char *name){
	printf("Error type 16 at Line %d: Duplicated name '%s'\n",struct_begin,name);
}

void print_semantic_error17(int lineno,char *name){
	printf("Error type 17 at Line %d: Undefined structure '%s'\n",lineno,name);
}

// experiment 3
void add_read_function(){
	int hashcode = hash("read");
	struct Function* fc = (struct Function*)malloc(sizeof(struct Function));
	fc->return_type = (Type*)malloc(sizeof(Type));
	fc->return_type->kind = INTEGER;
	functionTable[hashcode]=fc;
}

void add_write_function(){
	int hashcode = hash("write");
	struct Function* fc = (struct Function*)malloc(sizeof(struct Function));
	fc->return_type = (Type*)malloc(sizeof(Type));
	fc->return_type->kind = INTEGER;
	fc->args=(FieldList*)malloc(sizeof(FieldList));
	fc->args->tail=NULL;
	fc->args->type=(Type*)malloc(sizeof(Type));
	fc->args->type->kind = INTEGER;
	functionTable[hashcode]=fc;
}

