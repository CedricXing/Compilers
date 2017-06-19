#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "create_gramm_tree.h"

extern struct InterCodes *head;
extern char *output_file;
extern int var_no;

int tempValueLocations[200];

void generate_targetcodes(){
	FILE *fp = fopen(output_file,"w");
	initTempValueLocations();
	outputBasicTargetCodes(fp);
	if(fp == NULL){
		printf("file open error!\n");
		return;
	}
	struct InterCodes *ir = head;
	//int sp = 0;
	while(ir != NULL){
		if(ir->code.kind == FUNCTION){
			fprintf(fp, "%s:\n",ir->code.u.function_name);
			//fprintf(fp, "	move $fp,$sp\n");
			//sp = 0;
			if(strcmp(ir->code.u.function_name,"main")==0){
				fprintf(fp, "	move $fp,$sp\n");
				int hashcode = hash(ir->code.u.function_name);
				fprintf(fp, "	subu $sp,$sp,%d\n",functionTable[hashcode]->numTemps * 4);
			}
		}
		else if(ir->code.kind == PARAM){
			struct InterCodes *tempIR = ir;
			int index = 0;
			while(tempIR->code.kind == PARAM){
				if(index < 4)
					fprintf(fp, "	sw $a%d,%d($fp)\n",index,tempValueLocations[tempIR->code.u.arg_no]);
				else{
					fprintf(fp, "	lw $t0,%d($fp)\n",(index - 2) * 4);
					fprintf(fp, "	sw $t0,%d($fp)\n",tempValueLocations[tempIR->code.u.arg_no]);
				}
				tempIR = tempIR->next;
				++index;
			}
			ir = tempIR;
			continue;
		}
		else if(ir->code.kind == ASSIGN){
			if(ir->code.u.assign.right.kind == CONSTANT){
				int tempNo = ir->code.u.assign.left.u.var_no;
				/*
				if(tempValueLocations[tempNo] == -1){
					fprintf(fp, "	li $t0,%d\n",ir->code.u.assign.right.u.value);
					fprintf(fp, "	subu $sp,$sp,4\n");
					fprintf(fp, "	sw $t0,0($sp)\n");
					sp -= 4;
					tempValueLocations[tempNo] = sp;
					printf("%d  %d\n",tempNo,sp);
				}
				else{
					fprintf(fp, "	li $t0,%d\n",ir->code.u.assign.right.u.value);
					fprintf(fp, "	sw $t0,%d($fp)\n",tempValueLocations[tempNo]);
				}
				*/
				fprintf(fp, "	li $t0,%d\n",ir->code.u.assign.right.u.value);
				fprintf(fp, "	sw $t0,%d($fp)\n",tempValueLocations[tempNo]);
			}
			else if(ir->code.u.assign.right.kind == VARIABLE){
				int tempLeft = ir->code.u.assign.left.u.var_no;
				int tempRight = ir->code.u.assign.right.u.var_no;
				fprintf(fp, "	lw $t0,%d($fp)\n",tempValueLocations[tempRight]);
				/*
				if(tempValueLocations[tempLeft] == -1){
					//printf("%d\n",tempLeft);
					fprintf(fp, "	subu $sp,$sp,4\n");
					fprintf(fp, "	sw $t0,0($sp)\n");
					sp -= 4;
					tempValueLocations[tempLeft] = sp;
					printf("%d  %d\n",tempLeft,sp);
				}
				else{
					fprintf(fp, "	sw $t0,%d($fp)\n",tempValueLocations[tempLeft]);
				}
				*/
				fprintf(fp, "	sw $t0,%d($fp)\n",tempValueLocations[tempLeft]);
			}
		}
		else if(ir->code.kind == ADD){
			if(ir->code.u.binop.op1.kind == VARIABLE && ir->code.u.binop.op2.kind == VARIABLE){
				int tempOP1 = ir->code.u.binop.op1.u.var_no;
				int tempOP2 = ir->code.u.binop.op2.u.var_no;
				int tempResult = ir->code.u.binop.result.u.var_no;
				fprintf(fp, "	lw $t0,%d($fp)\n",tempValueLocations[tempOP1]);
				fprintf(fp, "	lw $t1,%d($fp)\n",tempValueLocations[tempOP2]);
				/*
				if(tempValueLocations[tempResult] == -1){
					fprintf(fp, "	add $t2,$t0,$t1\n");
					fprintf(fp, "	subu $sp,$sp,4\n");
					fprintf(fp, "	sw $t2,0($sp)\n");
					sp -= 4;
					tempValueLocations[tempResult] = sp;
				}
				else{
					fprintf(fp, "	add $t2,$t0,$t1\n");
					fprintf(fp, "	sw $t2,%d($fp)\n",tempValueLocations[tempResult]);
				}
				*/
				fprintf(fp, "	add $t2,$t0,$t1\n");
				fprintf(fp, "	sw $t2,%d($fp)\n",tempValueLocations[tempResult]);
			}
		}
		else if(ir->code.kind == SUB){
			if(ir->code.u.binop.op1.kind == CONSTANT && ir->code.u.binop.op2.kind==VARIABLE){
				fprintf(fp, "	li $t0,0\n");
				int tempOP2 = ir->code.u.binop.op2.u.var_no;
				int tempResult = ir->code.u.binop.result.u.var_no;
				fprintf(fp,"	lw $t1,%d($fp)\n",tempValueLocations[tempOP2]);
				/*
				if(tempValueLocations[tempResult] == -1){
					fprintf(fp, "	sub $t2,$t0,$t1\n");
					fprintf(fp, "	subu $sp,$sp,4\n");
					fprintf(fp, "	sw $t2,0($sp)\n");
					sp -= 4;
					tempValueLocations[tempResult] = sp;
				}
				else{
					fprintf(fp, "	sub $t2,$t0,$t1\n");
					fprintf(fp, "	sw $t2,%d($fp)\n",tempValueLocations[tempResult]);
				}
				*/
				fprintf(fp, "	sub $t2,$t0,$t1\n");
				fprintf(fp, "	sw $t2,%d($fp)\n",tempValueLocations[tempResult]);
			}
			else if(ir->code.u.binop.op1.kind == VARIABLE && ir->code.u.binop.op2.kind == VARIABLE){
				int tempOP1 = ir->code.u.binop.op1.u.var_no;
				int tempOP2 = ir->code.u.binop.op2.u.var_no;
				int tempResult = ir->code.u.binop.result.u.var_no;
				fprintf(fp, "	lw $t0,%d($fp)\n",tempValueLocations[tempOP1]);
				fprintf(fp, "	lw $t1,%d($fp)\n",tempValueLocations[tempOP2]);
				/*
				if(tempValueLocations[tempResult] == -1){
					fprintf(fp, "	sub $t2,$t0,$t1\n");
					fprintf(fp, "	subu $sp,$sp,4\n");
					fprintf(fp, "	sw $t2,0($sp)\n");
					sp -= 4;
					tempValueLocations[tempResult] = sp;
				}
				else{
					fprintf(fp, "	sub $t2,$t0,$t1\n");
					fprintf(fp, "	sw $t2,%d($fp)\n",tempValueLocations[tempResult]);
				}
				*/
				fprintf(fp, "	sub $t2,$t0,$t1\n");
				fprintf(fp, "	sw $t2,%d($fp)\n",tempValueLocations[tempResult]);
			}
		}
		else if(ir->code.kind == STAR){
			if(ir->code.u.binop.op1.kind == VARIABLE && ir->code.u.binop.op2.kind == VARIABLE){
				int tempOP1 = ir->code.u.binop.op1.u.var_no;
				int tempOP2 = ir->code.u.binop.op2.u.var_no;
				int tempResult = ir->code.u.binop.result.u.var_no;
				fprintf(fp, "	lw $t0,%d($fp)\n",tempValueLocations[tempOP1]);
				fprintf(fp, "	lw $t1,%d($fp)\n",tempValueLocations[tempOP2]);
				/*
				if(tempValueLocations[tempResult] == -1){
					fprintf(fp, "	mul $t2,$t0,$t1\n");
					fprintf(fp, "	subu $sp,$sp,4\n");
					fprintf(fp, "	sw $t2,0($sp)\n");
					sp -= 4;
					tempValueLocations[tempResult] = sp;
				}
				else{
					fprintf(fp, "	mul $t2,$t0,$t1\n");
					fprintf(fp, "	sw $t2,%d($fp)\n",tempValueLocations[tempResult]);
				}
				*/
				fprintf(fp, "	mul $t2,$t0,$t1\n");
				fprintf(fp, "	sw $t2,%d($fp)\n",tempValueLocations[tempResult]);
			}
			else if(ir->code.u.binop.op1.kind == VARIABLE && ir->code.u.binop.op2.kind == CONSTANT){
				int tempOP1 = ir->code.u.binop.op1.u.var_no;
				int tempResult = ir->code.u.binop.result.u.var_no;
				fprintf(fp, "	lw $t0,%d($fp)\n",tempValueLocations[tempOP1]);
				fprintf(fp, "	li $t1,%d\n",ir->code.u.binop.op2.u.value);
				/*
				if(tempValueLocations[tempResult] == -1){
					fprintf(fp, "	mul $t2,$t0,$t1\n");
					fprintf(fp, "	subu $sp,$sp,4\n");
					fprintf(fp, "	sw $t2,0($sp)\n");
					sp -= 4;
					tempValueLocations[tempResult] = sp;
				}
				else{
					fprintf(fp, "	mul $t2,$t0,$t1\n");
					fprintf(fp, "	sw $t2,%d($fp)\n",tempValueLocations[tempResult]);
				}
				*/
				fprintf(fp, "	mul $t2,$t0,$t1\n");
				fprintf(fp, "	sw $t2,%d($fp)\n",tempValueLocations[tempResult]);
			}
		}
		else if(ir->code.kind == DIV){
			if(ir->code.u.binop.op1.kind == VARIABLE && ir->code.u.binop.op2.kind == VARIABLE){
				int tempOP1 = ir->code.u.binop.op1.u.var_no;
				int tempOP2 = ir->code.u.binop.op2.u.var_no;
				int tempResult = ir->code.u.binop.result.u.var_no;
				fprintf(fp, "	lw $t0,%d($fp)\n",tempValueLocations[tempOP1]);
				fprintf(fp, "	lw $t1,%d($fp)\n",tempValueLocations[tempOP2]);
				/*
				if(tempValueLocations[tempResult] == -1){
					fprintf(fp, "	div $t0,$t1\n");
					fprintf(fp, "	mflo $t2\n");
					fprintf(fp, "	subu $sp,$sp,4\n");
					fprintf(fp, "	sw $t2,0($sp)\n");
					sp -= 4;
					tempValueLocations[tempResult] = sp;
				}
				else{
					fprintf(fp, "	div $t0,$t1\n");
					fprintf(fp, "	mflo $t2\n");
					fprintf(fp, "	sw $t2,%d($fp)\n",tempValueLocations[tempResult]);
				}
				*/
				fprintf(fp, "	div $t0,$t1\n");
				fprintf(fp, "	mflo $t2\n");
				fprintf(fp, "	sw $t2,%d($fp)\n",tempValueLocations[tempResult]);
			}
		}
		else if(ir->code.kind == Return){
			fprintf(fp, "	lw $v0,%d($fp)\n",tempValueLocations[ir->code.u.op.u.var_no]);
			fprintf(fp, "	jr $ra\n");
		}
		else if(ir->code.kind == LABEL){
			fprintf(fp, "label%d:\n",ir->code.label_no);
		}
		else if(ir->code.kind == GOTO){
			fprintf(fp, "	j label%d\n",ir->code.label_no);
		}
		else if(ir->code.kind == If){
			int tempOP1 = ir->code.u.double_op.op1.u.var_no;
			fprintf(fp, "	lw $t0,%d($fp)\n",tempValueLocations[tempOP1]);
			if(ir->code.u.double_op.op1.kind == VARIABLE && ir->code.u.double_op.op2.kind == VARIABLE){	
				int tempOP2 = ir->code.u.double_op.op2.u.var_no;
				fprintf(fp, "	lw $t1,%d($fp)\n",tempValueLocations[tempOP2]);
			}
			else if(ir->code.u.double_op.op1.kind == VARIABLE && ir->code.u.double_op.op2.kind == CONSTANT){
				fprintf(fp, "	li $t1,%d\n",ir->code.u.double_op.op2.u.value);
			}
			if(strcmp(ir->code.relop,"==")==0)
				fprintf(fp, "	beq $t0,$t1,label%d\n",ir->code.label_no);
			else if(strcmp(ir->code.relop,"!=")==0)
				fprintf(fp, "	bne $t0,$t1,label%d\n",ir->code.label_no);
			else if(strcmp(ir->code.relop,">")==0)
				fprintf(fp, "	bgt $t0,$t1,label%d\n",ir->code.label_no);
			else if(strcmp(ir->code.relop,"<")==0)
				fprintf(fp, "	blt $t0,$t1,label%d\n",ir->code.label_no);
			else if(strcmp(ir->code.relop,">=")==0)
				fprintf(fp, "	bge $t0,$t1,label%d\n",ir->code.label_no);
			else if(strcmp(ir->code.relop,"<=")==0)
				fprintf(fp, "	ble $t0,$t1,label%d\n",ir->code.label_no);
		}
		else if(ir->code.kind == READ){
			fprintf(fp, "	addi $sp,$sp,-4\n");
			fprintf(fp, "	sw $ra,0($sp)\n");
			fprintf(fp, "	jal read\n");
			fprintf(fp, "	lw $ra,0($sp)\n");
			fprintf(fp, "	addi $sp,$sp,4\n");
			fprintf(fp, "	sw $v0,%d($fp)\n",tempValueLocations[ir->code.var_no]);
		}
		else if(ir->code.kind == WRITE){
			fprintf(fp, "	lw $a0,%d($fp)\n",tempValueLocations[ir->code.var_no]);
			fprintf(fp, "	addi $sp,$sp,-4\n");
			fprintf(fp, "	sw $ra,0($sp)\n");
			fprintf(fp, "	jal write\n");
			fprintf(fp, "	lw $ra,0($sp)\n");
			fprintf(fp, "	addi $sp,$sp,4\n");
		}
		else if(ir->code.kind == ARG){
			int argumentNO[20];
			struct InterCodes *tempIR = ir;
			int index = 0;
			while(tempIR->code.kind != CALL){
				argumentNO[index] = tempIR->code.u.op.u.var_no;
				tempIR = tempIR->next;
				++index;
			}
			int num = 0;
			while(index > 0){
					-- index;
					if(num < 4)
						fprintf(fp, "	lw $a%d,%d($fp)\n",num,tempValueLocations[argumentNO[index]]);
					else{
						for(int j = 0;j <= index;++ j){
							fprintf(fp, "	lw $t0,%d($fp)\n",tempValueLocations[argumentNO[j]]);
							fprintf(fp, "	subu $sp,$sp,4\n");
							fprintf(fp, "	sw $t0,0($sp)\n");
						}
						break;
					}
					++num;
			}
			/*
			struct InterCodes *paraIR = getIRByFunctionName(tempIR->code.u.function_name);
			while(paraIR->code.kind == PARAM){
				fprintf(fp, "	lw $t0,%d($fp)\n",tempValueLocations[argumentNO[--index]]);
				fprintf(fp, "	sw $t0,%d($fp)\n",tempValueLocations[paraIR->code.u.arg_no]);
				paraIR = paraIR -> next;
			}
			*/
			ir = tempIR;
			continue;
		}
		else if(ir->code.kind == CALL){
			fprintf(fp, "	addi $sp,$sp,-4\n");
			fprintf(fp, "	sw $ra,0($sp)\n");
			fprintf(fp, "	addi $sp,$sp,-4\n");
			fprintf(fp, "	sw $fp,0($sp)\n");
			fprintf(fp, "	move $fp,$sp\n");
			int hashcode = hash(ir->code.u.function_name);
			fprintf(fp, "	subu $sp,$sp,%d\n",functionTable[hashcode]->numTemps * 4);
			fprintf(fp, "	jal %s\n",ir->code.u.function_name);
			fprintf(fp, "	move $sp,$fp\n");
			fprintf(fp, "	lw $fp,0($sp)\n");
			fprintf(fp, "	addi $sp,$sp,4\n");
			fprintf(fp, "	lw $ra,0($sp)\n");
			fprintf(fp, "	addi $sp,$sp,4\n");
			fprintf(fp, "	sw $v0,%d($fp)\n",tempValueLocations[ir->code.var_no]);
		}
		else if(ir->code.kind == DEC){
			fprintf(fp, "	subu $sp,$sp,%d\n",ir->code.space);
			//fprintf(fp, "	subu $t0,$sp,$fp\n");
			//fprintf(fp, "	sw $t0,%d($fp)\n",tempValueLocations[ir->code.u.op.u.var_no]);
			fprintf(fp, "	sw $sp,%d($fp)\n",tempValueLocations[ir->code.u.op.u.var_no]);
		}
		else if(ir->code.kind == GET_ADDRESS){
			fprintf(fp, "	lw $t0,%d($fp)\n",tempValueLocations[ir->code.u.double_op.op2.u.var_no]);
			fprintf(fp, "	sw $t0,%d($fp)\n",tempValueLocations[ir->code.u.double_op.op1.u.var_no]);
		}
		else if(ir->code.kind == GET_VALUE){
			if(ir->code.u.double_op.op1.kind == VARIABLE && ir->code.u.double_op.op2.kind == ADDRESS){
				fprintf(fp, "	lw $t0,%d($fp)\n",tempValueLocations[ir->code.u.double_op.op2.u.var_no]);
				//fprintf(fp, "	addu $t0,$t0,$fp\n");
				fprintf(fp, "	lw $t1,0($t0)\n");
				fprintf(fp, "	sw $t1,%d($fp)\n",tempValueLocations[ir->code.u.double_op.op1.u.var_no]);
			}
			else if(ir->code.u.double_op.op1.kind == ADDRESS && ir->code.u.double_op.op2.kind == VARIABLE){
				fprintf(fp, "	lw $t0,%d($fp)\n",tempValueLocations[ir->code.u.double_op.op2.u.var_no]);
				fprintf(fp, "	lw $t1,%d($fp)\n",tempValueLocations[ir->code.u.double_op.op1.u.var_no]);
				//fprintf(fp, "	addu $t1,$t1,$fp\n");
				fprintf(fp, "	sw $t0,0($t1)\n");
			}
		}
		ir = ir -> next;
	}
	fclose(fp);	
}

void output_targetcodes(){

}

void outputBasicTargetCodes(FILE *fp){
	fprintf(fp, ".data\n");
	fprintf(fp, "_ret: .asciiz \"\\n\"\n");
	fprintf(fp, "_prompt: .asciiz \"Enter an integer:\"\n");
	fprintf(fp, ".globl main\n");
	fprintf(fp, ".text\n");
	fprintf(fp, "read:\n");
	fprintf(fp, "	li $v0,4\n");
	fprintf(fp, "	la $a0,_prompt\n");
	fprintf(fp, "	syscall\n");
	fprintf(fp, "	li $v0,5\n");
	fprintf(fp, "	syscall\n");
	fprintf(fp, "	jr $ra\n\n");
	fprintf(fp, "write:\n");
	fprintf(fp, "	li $v0,1\n");
	fprintf(fp, "	syscall\n");
	fprintf(fp, "	li $v0,4\n");
	fprintf(fp, "	la $a0,_ret\n");
	fprintf(fp, "	syscall\n");
	fprintf(fp, "	move $v0,$0\n");
	fprintf(fp, "	jr $ra\n");
}


void initTempValueLocations(){
	for(int i = 0;i < 200;++i)
		tempValueLocations[i] = -(i + 1) * 4;
}

struct InterCodes* getIRByFunctionName(char *name){
	struct InterCodes *ir = head;
	while(ir != NULL){
		if(ir->code.kind == FUNCTION && strcmp(ir->code.u.function_name,name)==0)
			return ir->next;
		ir = ir -> next;
	}
}

int getNumTempsBYName(char *name){
	struct InterCodes *ir = head;
	while(ir != NULL){
		if(ir->code.kind == FUNCTION && strcmp(ir->code.u.function_name,name)==0){
			int max = 0;
			struct InterCodes *temp = ir -> next;
			while(temp!= NULL && temp->code.kind != FUNCTION){
				if(temp->code.kind == Return && temp->code.u.op.u.var_no > max){
					max = temp->code.u.op.u.var_no;
				}
				temp = temp -> next;
			}
			return max;
		}
		ir = ir -> next;
	}
}