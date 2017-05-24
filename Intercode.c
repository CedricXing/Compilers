#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "create_gramm_tree.h"

struct InterCodes *head = NULL;
int var_no = 0;
int label_no = 0;
char *output_file;

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
			int hashcode1 = hash(fl->name);
			if(symbolTable[hashcode1]->var_no == -1)
				symbolTable[hashcode1]->var_no = var_no++;
			if(symbolTable[hashcode1]->tp->kind == ARRAY)
				symbolTable[hashcode1]->is_arg_array = 1;
			temp_ir->code.u.arg_no=symbolTable[hashcode1]->var_no;
			temp_ir->next = NULL;
			temp->next = temp_ir;
			temp=temp_ir;
			fl = fl->tail;
		}
	}
	return ir1;
}

struct InterCodes* generate_label(int label){
	struct InterCodes *ir = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	ir->next = NULL;
	ir->code.kind = LABEL;
	ir->code.label_no = label;
	return ir;
}

struct InterCodes* generate_goto(int label){
	struct InterCodes *ir = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	ir->next = NULL;
	ir->code.kind = GOTO;
	ir->code.label_no = label;
	return ir;
}

struct InterCodes* generate_read(int place){
	struct InterCodes *ir = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	ir->next = NULL;
	ir->code.kind = READ;
	ir->code.var_no = place;
	return ir;
}

struct InterCodes* generate_write(int place){
	struct InterCodes *ir = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	ir->next = NULL;
	ir->code.kind = WRITE;
	ir->code.var_no = place;
	return ir;
}

struct InterCodes* generate_arg(int place){
	struct InterCodes *ir = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	ir->next = NULL;
	ir->code.kind = ARG;
	for(int i = 0;i < 16384;++ i){
		if(symbolTable[i] != NULL){
			if(symbolTable[i]->var_no == place && symbolTable[i]->tp->kind == ARRAY){
				ir->code.u.op.kind = ADDRESS;
				ir->code.u.op.u.var_no = place;
				return ir;
			}
		}
	}
	ir->code.u.op.kind = VARIABLE;
	ir->code.u.op.u.var_no = place;
	return ir;
}

struct InterCodes* generate_call(int place,char *function_name){
	struct InterCodes *ir = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	ir->next = NULL;
	ir->code.kind = CALL;
	ir->code.var_no = place;
	ir->code.u.function_name = function_name;
	return ir;
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

struct InterCodes* generate_plus_double_id(int place,int place1,int place2){
	struct InterCodes* ir = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	ir -> next = NULL;
	ir -> code.kind = ADD;
	ir -> code.u.binop.result.kind = VARIABLE;
	ir -> code.u.binop.result.u.var_no = place;
	ir -> code.u.binop.op1.kind = VARIABLE;
	ir -> code.u.binop.op1.u.var_no = place1;
	ir -> code.u.binop.op2.kind = VARIABLE;
	ir -> code.u.binop.op2.u.var_no = place2;
	return ir;
}


struct InterCodes* generate_minus_double_id(int place,int place1,int place2){
	struct InterCodes* ir = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	ir -> next = NULL;
	ir -> code.kind = SUB;
	ir -> code.u.binop.result.kind = VARIABLE;
	ir -> code.u.binop.result.u.var_no = place;
	ir -> code.u.binop.op1.kind = VARIABLE;
	ir -> code.u.binop.op1.u.var_no = place1;
	ir -> code.u.binop.op2.kind = VARIABLE;
	ir -> code.u.binop.op2.u.var_no = place2;
	return ir;
}

struct InterCodes* generate_star_double_id(int place,int place1,int place2){
	struct InterCodes* ir = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	ir -> next = NULL;
	ir -> code.kind = STAR;
	ir -> code.u.binop.result.kind = VARIABLE;
	ir -> code.u.binop.result.u.var_no = place;
	ir -> code.u.binop.op1.kind = VARIABLE;
	ir -> code.u.binop.op1.u.var_no = place1;
	ir -> code.u.binop.op2.kind = VARIABLE;
	ir -> code.u.binop.op2.u.var_no = place2;
	return ir;
}

struct InterCodes* generate_star_id_constant(int place,int place1,int constant){
	struct InterCodes *ir = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	ir -> next = NULL;
	ir -> code.kind = STAR;
	ir -> code.u.binop.result.kind = VARIABLE;
	ir -> code.u.binop.result.u.var_no = place;
	ir -> code.u.binop.op1.kind = VARIABLE;
	ir -> code.u.binop.op1.u.var_no = place1;
	ir -> code.u.binop.op2.kind = CONSTANT;
	ir -> code.u.binop.op2.u.value = constant;
	return ir;
}

struct InterCodes* generate_div_double_id(int place,int place1,int place2){
	struct InterCodes* ir = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	ir -> next = NULL;
	ir -> code.kind = DIV;
	ir -> code.u.binop.result.kind = VARIABLE;
	ir -> code.u.binop.result.u.var_no = place;
	ir -> code.u.binop.op1.kind = VARIABLE;
	ir -> code.u.binop.op1.u.var_no = place1;
	ir -> code.u.binop.op2.kind = VARIABLE;
	ir -> code.u.binop.op2.u.var_no = place2;
	return ir;
}

struct InterCodes* generate_minus_constant_id(int place,int constant,int place1){
	struct InterCodes* ir = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	ir -> next = NULL;
	ir -> code.kind = SUB;
	ir -> code.u.binop.result.kind = VARIABLE;
	ir -> code.u.binop.result.u.var_no = place;
	ir -> code.u.binop.op1.kind = CONSTANT;
	ir -> code.u.binop.op1.u.value = constant;
	ir -> code.u.binop.op2.kind = VARIABLE;
	ir -> code.u.binop.op2.u.var_no = place1;
	return ir;
}

struct InterCodes* generate_return(int place){
	struct InterCodes* ir = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	ir -> next = NULL;
	ir -> code.kind = Return;
	ir -> code.u.op.kind = VARIABLE;
	ir -> code.u.op.u.var_no = place;
	return ir;
}

struct InterCodes* generate_if_double_id(int place1,int place2,char *relop,int label){
	struct InterCodes* ir = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	ir -> next = NULL;
	ir -> code.kind = If;
	ir -> code.relop = relop;
	ir -> code.label_no = label;
	ir -> code.u.double_op.op1.kind = VARIABLE;
	ir -> code.u.double_op.op1.u.var_no = place1;
	ir -> code.u.double_op.op2.kind = VARIABLE;
	ir -> code.u.double_op.op2.u.var_no = place2;
	return ir;
}

struct InterCodes* generate_if_id_constant(int place,int constant,char *relop,int label){
	struct InterCodes *ir = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	ir -> next = NULL;
	ir -> code.kind = If;
	ir -> code.relop = relop;
	ir -> code.label_no = label;
	ir -> code.u.double_op.op1.kind = VARIABLE;
	ir -> code.u.double_op.op1.u.var_no = place;
	ir -> code.u.double_op.op2.kind = CONSTANT;
	ir -> code.u.double_op.op2.u.value = constant;
	return ir;
}

struct InterCodes* generate_dec(int place,int space){
	struct InterCodes* ir = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	ir -> next = NULL;
	ir -> code.kind = DEC;
	ir -> code.u.op.kind = VARIABLE;
	ir -> code.u.op.u.var_no = place;
	ir -> code.space = space;
	return ir;
}

struct InterCodes* generate_get_address(int place1,int place2){
	struct InterCodes *ir = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	ir -> next = NULL;
	ir -> code.kind = GET_ADDRESS;
	ir -> code.u.double_op.op1.kind = VARIABLE;
	ir -> code.u.double_op.op1.u.var_no = place1;
	ir -> code.u.double_op.op2.kind = VARIABLE;
	ir -> code.u.double_op.op2.u.var_no = place2;
	return ir;
}

struct InterCodes* generate_get_value(int place1,int place2){
	struct InterCodes *ir = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	ir -> next = NULL;
	ir -> code.kind = GET_VALUE;
	ir -> code.u.double_op.op1.kind = VARIABLE;
	ir -> code.u.double_op.op1.u.var_no = place1;
	ir -> code.u.double_op.op2.kind = ADDRESS;
	ir -> code.u.double_op.op2.u.var_no = place2;
	return ir;
}

struct InterCodes* generate_get_value_left_operand(int place1,int place2){
	struct InterCodes *ir = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	ir -> next = NULL;
	ir -> code.kind = GET_VALUE;
	ir -> code.u.double_op.op1.kind = ADDRESS;
	ir -> code.u.double_op.op1.u.var_no = place1;
	ir -> code.u.double_op.op2.kind = VARIABLE;
	ir -> code.u.double_op.op2.u.var_no = place2;
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
	struct Node *def = DefList;
	struct InterCodes *ir = NULL;
	while(def->left_child != NULL){
		if(ir == NULL) ir = translate_Def(def->left_child);
		else cat_ir(ir,translate_Def(def->left_child));
		def = def->left_child->right_child;
	}
	return ir;
}

struct InterCodes* translate_Def(struct Node *Def){
	return translate_DecList(Def->left_child->right_child);
}

struct InterCodes* translate_DecList(struct Node *DecList){
	struct Node * declist = DecList;
	struct InterCodes *ir = NULL;
	while(1){
		if(ir == NULL) ir = translate_Dec(declist->left_child);
		else cat_ir(ir,translate_Dec(declist->left_child));
		if(declist->left_child->right_child != NULL)
			declist = declist->left_child->right_child->right_child;
		else break;
	}
	return ir;
}

struct InterCodes* translate_Dec(struct Node *Dec){
	if(Dec->left_child->right_child == NULL)
		return translate_VarDec(Dec->left_child);
	else{
		int temp = var_no++;
		struct InterCodes *code1 = translate_exp(Dec->left_child->right_child->right_child,temp);
		assert(strcmp(Dec->left_child->left_child->name,"ID")==0);
		int hashcode = hash(Dec->left_child->left_child->attribute);

		if(symbolTable[hashcode]->var_no == -1)
			symbolTable[hashcode]->var_no = var_no++;
		cat_ir(code1,generate_assign_double_id(symbolTable[hashcode]->var_no,temp));
		return code1;
	}
}

struct InterCodes* translate_VarDec(struct Node *VarDec){
	if(strcmp(VarDec->left_child->name,"ID")==0)
		return NULL;
	//array definition
	struct Node *vardec = VarDec;
	int num = 1;
	while(strcmp(vardec->left_child->name,"ID") != 0){
		num *= vardec->left_child->right_child->right_child->value_int;
		vardec = vardec->left_child;
	}
	int hashcode = hash(vardec->left_child->attribute);
	if(symbolTable[hashcode]->var_no == -1)
		symbolTable[hashcode]->var_no = var_no++;
	return generate_dec(symbolTable[hashcode]->var_no,num*4);
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
	else if(strcmp(Stmt->left_child->name,"RETURN")==0){
		int temp = var_no++;
		struct InterCodes* code1 = translate_exp(Stmt->left_child->right_child,temp);
		struct InterCodes* code2 = generate_return(temp);
		cat_ir(code1,code2);
		return code1;
	}
	else if(strcmp(Stmt->left_child->name,"IF")==0){
		if(check_has_else(Stmt)){
			int label1 = label_no++;
			int label2 = label_no++;
			int label3 = label_no++;
			struct InterCodes *code1 = translate_Cond(Stmt->left_child->right_child->right_child,label1,label2);
			struct InterCodes *code2 = translate_Stmt(Stmt->left_child->right_child->right_child->right_child->right_child);
			struct InterCodes *code3 = translate_Stmt(Stmt->left_child->right_child->right_child->right_child->right_child->right_child->right_child);
			struct InterCodes *ir = generate_label(label1);
			cat_ir(code1,ir);
			ir = generate_goto(label3);
			cat_ir(code2,ir);
			ir = generate_label(label2);
			cat_ir(code2,ir);
			ir = generate_label(label3);
			cat_ir(code3,ir);
			cat_ir(code1,code2);
			cat_ir(code1,code3);
			return code1;
		}
		else{
			int label1 = label_no++;
			int label2 = label_no++;
			struct InterCodes *code1 = translate_Cond(Stmt->left_child->right_child->right_child,label1,label2);
			struct Node *temp = Stmt->left_child->right_child->right_child->right_child->right_child;
			struct InterCodes *code2 = translate_Stmt(Stmt->left_child->right_child->right_child->right_child->right_child);		
			struct InterCodes *ir = generate_label(label1);
			cat_ir(code1,ir);
			ir = generate_label(label2);
			cat_ir(code2,ir);
			cat_ir(code1,code2);
			return code1;
		}
	}
	else if(strcmp(Stmt->left_child->name,"WHILE")==0){
		int label1 = label_no++;
		int label2 = label_no++;
		int label3 = label_no++;
		struct InterCodes *code1 = translate_Cond(Stmt->left_child->right_child->right_child,label2,label3);
		struct InterCodes *code2 = translate_Stmt(Stmt->left_child->right_child->right_child->right_child->right_child);
		struct InterCodes *ir = generate_label(label1);
		cat_ir(ir,code1);
		cat_ir(ir,generate_label(label2));
		cat_ir(ir,code2);
		cat_ir(ir,generate_goto(label1));
		cat_ir(ir,generate_label(label3));
		return ir;
	}
	return NULL;
	//to be continued
}

struct InterCodes* translate_Cond(struct Node *exp,int label_true,int label_false){
	if(exp->left_child->right_child != NULL && strcmp(exp->left_child->right_child->name,"RELOP")==0){
		int temp1 = var_no++;
		int temp2 = var_no++;
		struct InterCodes* code1 = translate_exp(exp->left_child,temp1);
		struct InterCodes* code2 = translate_exp(exp->left_child->right_child->right_child,temp2);
		struct InterCodes* ir = generate_if_double_id(temp1,temp2,exp->left_child->right_child->attribute,label_true);
		cat_ir(code1,code2);
		cat_ir(code1,ir);
		cat_ir(code1,generate_goto(label_false));
		return code1;
	}
	else if(strcmp(exp->left_child->name,"NOT")==0 && strcmp(exp->left_child->right_child->name,"Exp")==0){
		return translate_Cond(exp->left_child->right_child,label_false,label_true);
	}
	else if(exp->left_child->right_child != NULL && strcmp(exp->left_child->right_child->name,"AND")==0){
		int label1 = label_no++;
		struct InterCodes *code1 = translate_Cond(exp->left_child,label1,label_false);
		struct InterCodes *code2 = translate_Cond(exp->left_child->right_child->right_child,label_true,label_false);
		cat_ir(code1,generate_label(label1));
		cat_ir(code1,code2);
		return code1;
	}
	else if(exp->left_child->right_child != NULL && strcmp(exp->left_child->right_child->name,"OR")==0){
		int label1 = label_no++;
		struct InterCodes *code1 = translate_Cond(exp->left_child,label_true,label1);
		struct InterCodes *code2 = translate_Cond(exp->left_child->right_child->right_child,label_true,label_false);
		cat_ir(code1,generate_label(label1));
		cat_ir(code1,code2);
		return code1;
	}
	else{
		int temp = var_no++;
		struct InterCodes *code1 = translate_exp(exp,temp);
		char *relop = (char*)malloc(sizeof(char)*10);
		strcpy(relop,"!=");
		struct InterCodes *code2 = generate_if_id_constant(temp,0,relop,label_true);
		cat_ir(code1,code2);
		cat_ir(code1,generate_goto(label_false));
		return code1;
	}
	return NULL;
}

struct InterCodes* translate_exp(struct Node* exp,int place){
	if(strcmp(exp->left_child->name,"INT")==0){
		return generate_assign_id_constant(exp->left_child->value_int,place);
	}
	else if(strcmp(exp->left_child->name,"ID")==0 && exp->left_child->right_child==NULL){
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
			//printf("%s\n",exp->left_child->right_child->right_child->left_child->right_child->name);
			struct InterCodes* code1 = translate_exp(exp->left_child->right_child->right_child,temp_no);
			struct InterCodes* code2 = generate_assign_double_id(symbolTable[hashcode]->var_no,temp_no);
			cat_ir(code1,code2);
			if(place != -1){
				code2 = generate_assign_double_id(place,temp_no);
				cat_ir(code1,code2);
			}
			return code1;
		}
		else if(strcmp(exp->left_child->left_child->right_child->name,"LB")==0){
			int temp1 = var_no++;
			int temp2 = var_no++;
			struct InterCodes *code1 = translate_exp(exp->left_child->right_child->right_child,temp1);
			struct InterCodes *code2 = translate_arr_left_operand(exp->left_child,temp2);
			struct InterCodes *ir = generate_get_value_left_operand(temp2,temp1);
			cat_ir(code1,code2);
			cat_ir(code1,ir);
			return code1;
		}
	}
	else if(strcmp(exp->left_child->name,"Exp")==0&&strcmp(exp->left_child->right_child->name,"PLUS")==0){
		//printf("1\n");
		int temp1 = var_no++;
		int temp2 = var_no++;
		struct InterCodes* code1 = translate_exp(exp->left_child,temp1);
		struct InterCodes* code2 = translate_exp(exp->left_child->right_child->right_child,temp2);
		cat_ir(code1,code2);
		struct InterCodes* code = generate_plus_double_id(place,temp1,temp2);
		cat_ir(code1,code);
		return code1;
	}
	else if(strcmp(exp->left_child->name,"Exp")==0&&strcmp(exp->left_child->right_child->name,"MINUS")==0){
		int temp1 = var_no++;
		int temp2 = var_no++;
		struct InterCodes* code1 = translate_exp(exp->left_child,temp1);
		struct InterCodes* code2 = translate_exp(exp->left_child->right_child->right_child,temp2);
		cat_ir(code1,code2);
		struct InterCodes* code = generate_minus_double_id(place,temp1,temp2);
		cat_ir(code1,code);
		return code1;
	}
	else if(strcmp(exp->left_child->name,"Exp")==0&&strcmp(exp->left_child->right_child->name,"STAR")==0){
		//printf("2\n");
		int temp1 = var_no++;
		int temp2 = var_no++;
		struct InterCodes* code1 = translate_exp(exp->left_child,temp1);
		struct InterCodes* code2 = translate_exp(exp->left_child->right_child->right_child,temp2);
		cat_ir(code1,code2);
		struct InterCodes* code = generate_star_double_id(place,temp1,temp2);
		cat_ir(code1,code);
		return code1;
	}
	else if(strcmp(exp->left_child->name,"Exp")==0&&strcmp(exp->left_child->right_child->name,"DIV")==0){
		int temp1 = var_no++;
		int temp2 = var_no++;
		struct InterCodes* code1 = translate_exp(exp->left_child,temp1);
		struct InterCodes* code2 = translate_exp(exp->left_child->right_child->right_child,temp2);
		cat_ir(code1,code2);
		struct InterCodes* code = generate_div_double_id(place,temp1,temp2);
		cat_ir(code1,code);
		return code1;
	}
	else if(strcmp(exp->left_child->name,"MINUS")==0){
		int temp = var_no++;
		struct InterCodes* code1 = translate_exp(exp->left_child->right_child,temp);
		struct InterCodes* code2 = generate_minus_constant_id(place,0,temp);
		cat_ir(code1,code2);
		return code1;
	}
	else if(strcmp(exp->left_child->name,"NOT")==0){
		int label1 = label_no++;
		int label2 = label_no++;
		struct InterCodes* code0 = generate_assign_id_constant(0,place);
		struct InterCodes* code1 = translate_Cond(exp,label1,label2);
		struct InterCodes* code2 = generate_label(label1);
		cat_ir(code2,generate_assign_id_constant(1,place));
		cat_ir(code0,code1);
		cat_ir(code0,code2);
		cat_ir(code0,generate_label(label2));
		return code0;
	}
	else if(strcmp(exp->left_child->name,"Exp")==0 && (strcmp(exp->left_child->right_child->name,"RELOP")==0 || strcmp(exp->left_child->right_child->name,"AND")==0 || strcmp(exp->left_child->right_child->name,"OR")==0)){
		//printf("hello\n");
		int label1 = label_no++;
		int label2 = label_no++;
		struct InterCodes* code0 = generate_assign_id_constant(0,place);
		struct InterCodes* code1 = translate_Cond(exp,label1,label2);
		struct InterCodes* code2 = generate_label(label1);
		cat_ir(code2,generate_assign_id_constant(1,place));
		cat_ir(code0,code1);
		cat_ir(code0,code2);
		cat_ir(code0,generate_label(label2));
		return code0;
	}
	else if(strcmp(exp->left_child->name,"LP")==0){
		return translate_exp(exp->left_child->right_child,place);
	}
	else if(strcmp(exp->left_child->name,"ID")==0 && strcmp(exp->left_child->right_child->right_child->name,"RP")==0){
		if(strcmp(exp->left_child->attribute,"read")==0){
			return generate_read(place);
		}
		else return generate_call(place,exp->left_child->right_child->attribute);
	}
	else if(strcmp(exp->left_child->name,"ID")==0 && strcmp(exp->left_child->right_child->right_child->name,"Args")==0){
		int *arg_list = (int*)malloc(sizeof(int)*20);
		for(int i = 0;i < 20;++ i)
			arg_list[i] = -1;
		struct InterCodes* code1 = translate_Args(exp->left_child->right_child->right_child,arg_list);
		if(strcmp(exp->left_child->attribute,"write") == 0){
			if(code1 == NULL) code1 = generate_write(arg_list[0]);
			else cat_ir(code1,generate_write(arg_list[0]));
			return code1;
		}
		else{
			int i = 0;
			struct InterCodes *ir = NULL;
			while(arg_list[i] != -1){
				if(ir == NULL)	ir = generate_arg(arg_list[i]);
				else cat_ir(ir,generate_arg(arg_list[i]));
				++ i;
			}
			if(code1 == NULL) code1 = ir;
			else cat_ir(code1,ir);
			cat_ir(code1,generate_call(place,exp->left_child->attribute));
			return code1;
		}
	}
	else if(strcmp(exp->left_child->name,"Exp")==0 && strcmp(exp->left_child->right_child->name,"LB")==0){
		int arr_temp[10];
		for(int i = 0;i < 10;++ i)
			arr_temp[i] = -1;
		int index = 9;
		struct Node *temp_exp = exp;
		struct InterCodes *ir = NULL;
		while(strcmp(temp_exp->left_child->name,"ID") != 0){
			int temp = var_no++;
			if(ir == NULL) ir = translate_exp(temp_exp->left_child->right_child->right_child,temp);
			else cat_ir(ir,translate_exp(temp_exp->left_child->right_child->right_child,temp));
			arr_temp[index] = temp;
			-- index;
			temp_exp = temp_exp->left_child;
		} 
		int hashcode = hash(temp_exp->left_child->attribute);
		//printf("%s\n",temp_exp->left_child->attribute);
		assert(symbolTable[hashcode]!=NULL && symbolTable[hashcode]->tp->kind == ARRAY);
		int arr_var;
		if(!symbolTable[hashcode]->is_arg_array){//not arg array
			arr_var = var_no++;
			cat_ir(ir,generate_get_address(arr_var,symbolTable[hashcode]->var_no));
		}
		else {
			arr_var = var_no++;
			cat_ir(ir,generate_assign_double_id(arr_var,symbolTable[hashcode]->var_no));
		}
		Type *tp = symbolTable[hashcode]->tp;
		int arr_info[10];
		int index1 = 0;
		while(1){
			if(tp->kind == ARRAY){
				arr_info[index1] = tp->array.size;
			}
			else break;
			tp = tp->array.element;
			index1++;
		}
		for(int i = 0;i < index1;++ i){
			++ index;
			cat_ir(ir,generate_star_id_constant(arr_temp[index],arr_temp[index],get_space(arr_info,i,index1)));
			cat_ir(ir,generate_plus_double_id(arr_var,arr_var,arr_temp[index]));
		}
		cat_ir(ir,generate_get_value(place,arr_var));
		return ir;
	}
	else if(strcmp(exp->left_child->name,"Exp")==0 && strcmp(exp->left_child->right_child->name,"DOT")==0){
		printf("Cannot translate: Code contains variables or parameters of structure type.\n");
		exit(0);
	}
	return NULL;
}

struct InterCodes* translate_Args(struct Node *Args,int *arg_list){
	if(Args->left_child->right_child == NULL){
		int temp;
		struct InterCodes* code1 = NULL;
		if(strcmp(Args->left_child->left_child->name,"ID")==0 && Args->left_child->left_child->right_child==NULL){
			int hashcode = hash(Args->left_child->left_child->attribute);
			temp = symbolTable[hashcode]->var_no;
		}
		else{
			temp = var_no++;
			code1 = translate_exp(Args->left_child,temp);
		}
		int *arg_temp = (int*)malloc(sizeof(int)*20);
		int i = 0;
		while(arg_list[i] != -1){
			arg_temp[i] = arg_list[i];
			++ i;
		}
		arg_list[0] = temp;
		for(int j = 0;j < i;++ j)
			arg_list[j+1]=arg_temp[j];
		free(arg_temp);
		return code1;
	}
	else{
		int temp;
		struct InterCodes *code1 = NULL;
		if(strcmp(Args->left_child->left_child->name,"ID")==0){
			int hashcode = hash(Args->left_child->left_child->attribute);
			temp = symbolTable[hashcode]->var_no;
		}
		else{
			temp = var_no++;
			code1 = translate_exp(Args->left_child,temp);
		}
		int *arg_temp = (int*)malloc(sizeof(int)*20);
		int i = 0;
		while(arg_list[i] != -1){
			arg_temp[i] = arg_list[i];
			++ i;
		}
		arg_list[0] = temp;
		for(int j = 0;j < i;++ j)
			arg_list[j+1]=arg_temp[j];
		free(arg_temp);
		struct InterCodes *code2 = translate_Args(Args->left_child->right_child->right_child,arg_list);
		if(code1 == NULL) code1 = code2;
		else cat_ir(code1,code2);
		return code1;
	}
}

struct InterCodes* translate_arr_left_operand(struct Node *exp,int place){
	int arr_temp[10];
		for(int i = 0;i < 10;++ i)
			arr_temp[i] = -1;
		int index = 9;
		struct Node *temp_exp = exp;
		struct InterCodes *ir = NULL;
		while(strcmp(temp_exp->left_child->name,"ID") != 0){
			int temp = var_no++;
			if(ir == NULL) ir = translate_exp(temp_exp->left_child->right_child->right_child,temp);
			else cat_ir(ir,translate_exp(temp_exp->left_child->right_child->right_child,temp));
			arr_temp[index] = temp;
			-- index;
			temp_exp = temp_exp->left_child;
		} 
		int hashcode = hash(temp_exp->left_child->attribute);
		assert(symbolTable[hashcode]!=NULL && symbolTable[hashcode]->tp->kind == ARRAY);
		int arr_var;
		if(!symbolTable[hashcode]->is_arg_array){//not arg array
			arr_var = var_no++;
			cat_ir(ir,generate_get_address(arr_var,symbolTable[hashcode]->var_no));
		}
		else arr_var = symbolTable[hashcode]->var_no;
		Type *tp = symbolTable[hashcode]->tp;
		int arr_info[10];
		int index1 = 0;
		while(1){
			if(tp->kind == ARRAY){
				arr_info[index1] = tp->array.size;
			}
			else break;
			tp = tp->array.element;
			index1++;
		}
		for(int i = 0;i < index1;++ i){
			++ index;
			cat_ir(ir,generate_star_id_constant(arr_temp[index],arr_temp[index],get_space(arr_info,i,index1)));
			cat_ir(ir,generate_plus_double_id(arr_var,arr_var,arr_temp[index]));
		}
		cat_ir(ir,generate_assign_double_id(place,arr_var));
		return ir;
}

void output_InterCodes(){
	FILE *fp = fopen(output_file,"w");
	if(fp == NULL){
		printf("file open error!");
		return ;
	}
	struct InterCodes* ir = head;
	while(ir != NULL){
		if(ir->code.kind == FUNCTION){
			fprintf(fp,"FUNCTION %s :\n",ir->code.u.function_name);
			//printf("FUNCTION %s :\n",ir->code.u.function_name);
		}
		else if(ir->code.kind == PARAM){
			fprintf(fp, "PARAM %s%d\n","t",ir->code.u.arg_no);
			//printf("PARAM %s%d\n","t",ir->code.u.arg_no);
		}
		else if(ir->code.kind == ASSIGN){
			if(ir->code.u.assign.right.kind == CONSTANT){
				fprintf(fp,"t%d := #%d\n",ir->code.u.assign.left.u.var_no,ir->code.u.assign.right.u.value);
				//printf("t%d := #%d\n",ir->code.u.assign.left.u.var_no,ir->code.u.assign.right.u.value);
			}
			else if(ir->code.u.assign.right.kind == VARIABLE){
				fprintf(fp, "t%d := t%d\n",ir->code.u.assign.left.u.var_no,ir->code.u.assign.right.u.var_no);
				//printf("t%d := t%d\n",ir->code.u.assign.left.u.var_no,ir->code.u.assign.right.u.var_no);
			}
		}
		else if(ir->code.kind == ADD){
			if(ir->code.u.binop.op1.kind==VARIABLE && ir->code.u.binop.op2.kind==VARIABLE){//double variable
				fprintf(fp, "t%d := t%d + t%d\n",ir->code.u.binop.result.u.var_no,ir->code.u.binop.op1.u.var_no,ir->code.u.binop.op2.u.var_no);
				//printf("t%d := t%d + t%d\n",ir->code.u.binop.result.u.var_no,ir->code.u.binop.op1.u.var_no,ir->code.u.binop.op2.u.var_no);
			}
		}
		else if(ir -> code.kind == SUB){
			if(ir->code.u.binop.op1.kind==CONSTANT && ir->code.u.binop.op2.kind==VARIABLE){//1 constant 2 variable
				fprintf(fp, "t%d := t%d - t%d\n",ir->code.u.binop.result.u.var_no,ir->code.u.binop.op1.u.value,ir->code.u.binop.op2.u.var_no);
				//printf("t%d := t%d - t%d\n",ir->code.u.binop.result.u.var_no,ir->code.u.binop.op1.u.value,ir->code.u.binop.op2.u.var_no);
			}
			else if(ir->code.u.binop.op1.kind==VARIABLE && ir->code.u.binop.op2.kind==VARIABLE){
				fprintf(fp, "t%d := t%d - t%d\n",ir->code.u.binop.result.u.var_no,ir->code.u.binop.op1.u.var_no,ir->code.u.binop.op2.u.var_no);
				//printf("t%d := t%d - t%d\n",ir->code.u.binop.result.u.var_no,ir->code.u.binop.op1.u.var_no,ir->code.u.binop.op2.u.var_no);
			}
		}
		else if(ir->code.kind == STAR){
			if(ir->code.u.binop.op1.kind==VARIABLE && ir->code.u.binop.op2.kind==VARIABLE){//double variable
				fprintf(fp, "t%d := t%d * t%d\n",ir->code.u.binop.result.u.var_no,ir->code.u.binop.op1.u.var_no,ir->code.u.binop.op2.u.var_no);
				//printf("t%d := t%d * t%d\n",ir->code.u.binop.result.u.var_no,ir->code.u.binop.op1.u.var_no,ir->code.u.binop.op2.u.var_no);
			}
			else if(ir->code.u.binop.op1.kind == VARIABLE && ir->code.u.binop.op2.kind == CONSTANT){
				fprintf(fp, "t%d := t%d * #%d\n",ir->code.u.binop.result.u.var_no,ir->code.u.binop.op1.u.var_no,ir->code.u.binop.op2.u.value);
				//printf("t%d := t%d * #%d\n",ir->code.u.binop.result.u.var_no,ir->code.u.binop.op1.u.var_no,ir->code.u.binop.op2.u.value);
			}
		}
		else if(ir->code.kind == DIV){
			if(ir->code.u.binop.op1.kind==VARIABLE && ir->code.u.binop.op2.kind==VARIABLE){//double variable
				fprintf(fp, "t%d := t%d / t%d\n",ir->code.u.binop.result.u.var_no,ir->code.u.binop.op1.u.var_no,ir->code.u.binop.op2.u.var_no);
				//printf("t%d := t%d / t%d\n",ir->code.u.binop.result.u.var_no,ir->code.u.binop.op1.u.var_no,ir->code.u.binop.op2.u.var_no);
			}
		}
		else if(ir->code.kind == Return){
			fprintf(fp, "RETURN t%d\n",ir->code.u.op.u.var_no);
			//printf("RETURN t%d\n",ir->code.u.op.u.var_no);
		}
		else if(ir->code.kind == LABEL){
			fprintf(fp, "LABEL label%d :\n",ir->code.label_no);
			//printf("LABEL label%d :\n",ir->code.label_no);
		}
		else if(ir->code.kind == GOTO){
			fprintf(fp, "GOTO label%d\n",ir->code.label_no);
			//printf("GOTO label%d\n",ir->code.label_no);
		}
		else if(ir->code.kind == If){
			if(ir->code.u.double_op.op1.kind == VARIABLE && ir->code.u.double_op.op2.kind == VARIABLE){
				fprintf(fp, "IF t%d %s t%d GOTO label%d\n",ir->code.u.double_op.op1.u.var_no,ir->code.relop,ir->code.u.double_op.op2.u.var_no,ir->code.label_no);
				//printf("IF t%d %s t%d GOTO label%d\n",ir->code.u.double_op.op1.u.var_no,ir->code.relop,ir->code.u.double_op.op2.u.var_no,ir->code.label_no);
			}
			else if(ir->code.u.double_op.op1.kind == VARIABLE && ir->code.u.double_op.op2.kind == CONSTANT){
				fprintf(fp, "IF t%d %s #%d GOTO label%d\n",ir->code.u.double_op.op1.u.var_no,ir->code.relop,ir->code.u.double_op.op2.u.value,ir->code.label_no);
				//printf("IF t%d %s #%d GOTO label%d\n",ir->code.u.double_op.op1.u.var_no,ir->code.relop,ir->code.u.double_op.op2.u.value,ir->code.label_no);
			}
		}
		else if(ir->code.kind == READ){
			fprintf(fp, "READ t%d\n",ir->code.var_no);
			//printf("READ t%d\n",ir->code.var_no);
		}
		else if(ir->code.kind == WRITE){
			fprintf(fp,"WRITE t%d\n",ir->code.var_no);
			//printf("WRITE t%d\n",ir->code.var_no);
		}
		else if(ir->code.kind == ARG){
			if(ir->code.u.op.kind == VARIABLE){
				fprintf(fp, "ARG t%d\n",ir->code.u.op.u.var_no);
				//printf("ARG t%d\n",ir->code.u.op.u.var_no);
			}
			else if(ir->code.u.op.kind == ADDRESS){
				fprintf(fp, "ARG &t%d\n",ir->code.u.op.u.var_no);
				//printf("ARG &t%d\n",ir->code.u.op.u.var_no);
			}
		}
		else if(ir->code.kind == CALL){
			fprintf(fp, "t%d := CALL %s\n",ir->code.var_no,ir->code.u.function_name);
			//printf("t%d := CALL %s\n",ir->code.var_no,ir->code.u.function_name);
		}
		else if(ir->code.kind == DEC){
			fprintf(fp, "DEC t%d %d\n",ir->code.u.op.u.var_no,ir->code.space);
			//printf("DEC t%d %d\n",ir->code.u.op.u.var_no,ir->code.space);
		}
		else if(ir->code.kind == GET_ADDRESS){
			fprintf(fp, "t%d := &t%d\n",ir->code.u.double_op.op1.u.var_no,ir->code.u.double_op.op2.u.var_no);
			//printf("t%d := &t%d\n",ir->code.u.double_op.op1.u.var_no,ir->code.u.double_op.op2.u.var_no);
		}
		else if(ir->code.kind == GET_VALUE){
			if(ir->code.u.double_op.op1.kind == VARIABLE && ir->code.u.double_op.op2.kind == ADDRESS){
				fprintf(fp, "t%d := *t%d\n",ir->code.u.double_op.op1.u.var_no,ir->code.u.double_op.op2.u.var_no);
				//printf("t%d := *t%d\n",ir->code.u.double_op.op1.u.var_no,ir->code.u.double_op.op2.u.var_no);
			}
			else if(ir->code.u.double_op.op1.kind == ADDRESS && ir->code.u.double_op.op2.kind == VARIABLE){
				fprintf(fp, "*t%d := t%d\n",ir->code.u.double_op.op1.u.var_no,ir->code.u.double_op.op2.u.var_no);
				//printf("*t%d := t%d\n",ir->code.u.double_op.op1.u.var_no,ir->code.u.double_op.op2.u.var_no);
			}
		}
		ir = ir->next;
	}
	fclose(fp);
}

void cat_ir(struct InterCodes *code1,struct InterCodes *code2){
	assert(code1 != NULL);
	struct InterCodes *tp = code1;
	while(tp ->next != NULL){
		tp = tp->next;
	}
	tp->next = code2;
}


//some little functions
int check_has_else(struct Node *Stmt){
	if(Stmt->left_child->right_child->right_child->right_child->right_child->right_child==NULL)
		return 0;
	else return 1;
}

void check_ir(struct InterCodes *ir){
	while(ir != NULL){
		if(ir->code.kind == FUNCTION){
			printf("FUNCTION %s :\n",ir->code.u.function_name);
		}
		else if(ir->code.kind == PARAM){
			printf("PARAM %s%d\n","t",ir->code.u.arg_no);
		}
		else if(ir->code.kind == ASSIGN){
			if(ir->code.u.assign.right.kind == CONSTANT){
				printf("t%d := #%d\n",ir->code.u.assign.left.u.var_no,ir->code.u.assign.right.u.value);
			}
			else if(ir->code.u.assign.right.kind == VARIABLE){
				printf("t%d := t%d\n",ir->code.u.assign.left.u.var_no,ir->code.u.assign.right.u.var_no);
			}
		}
		else if(ir->code.kind == ADD){
			if(ir->code.u.binop.op1.kind==VARIABLE && ir->code.u.binop.op2.kind==VARIABLE){//double variable
				printf("t%d := t%d + t%d\n",ir->code.u.binop.result.u.var_no,ir->code.u.binop.op1.u.var_no,ir->code.u.binop.op2.u.var_no);
			}
		}
		else if(ir -> code.kind == SUB){
			if(ir->code.u.binop.op1.kind==CONSTANT && ir->code.u.binop.op2.kind==VARIABLE){//1 constant 2 variable
				printf("t%d := t%d - t%d\n",ir->code.u.binop.result.u.var_no,ir->code.u.binop.op1.u.value,ir->code.u.binop.op2.u.var_no);
			}
		}
		else if(ir->code.kind == Return){
			printf("RETURN t%d\n",ir->code.u.op.u.var_no);
		}
		else if(ir->code.kind == LABEL){
			printf("LABEL label%d :\n",ir->code.label_no);
		}
		else if(ir->code.kind == GOTO){
			printf("GOTO label%d\n",ir->code.label_no);
		}
		else if(ir->code.kind == If){
			printf("IF t%d %s t%d GOTO label%d\n",ir->code.u.double_op.op1.u.var_no,ir->code.relop,ir->code.u.double_op.op2.u.var_no,ir->code.label_no);
		}
		ir = ir->next;
	}
}

int get_space(int *arr_info,int i,int index1){
	int temp = 1;
	for(i = i + 1;i < index1;++ i)
		temp *= arr_info[i];
	return 4 * temp;
}