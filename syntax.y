%{
	#include <stdio.h>
	#include "lex.yy.c"
%}

/*declared tokens */

%union{
	struct Node *node;
}

%error-verbose

%token <node> INT FLOAT
%token <node> ID
%token <node> SEMI COMMA
%token <node> ASSIGNOP
%token <node> RELOP
%token <node> PLUS MINUS STAR DIV
%token <node> AND OR DOT NOT
%token <node> TYPE
%token <node> LP RP LB RB LC RC
%token <node> STRUCT
%token <node> RETURN
%token <node> IF ELSE
%token <node> WHILE
%token <node> error

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%type <node> Program ExtDefList ExtDef ExtDecList Specifier StructSpecifier OptTag Tag VarDec FunDec VarList ParamDec CompSt StmtList Stmt DefList Def DecList Dec Exp Args

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS STAR DIV
%right NOT 
%left LP RP LB RB LC RC DOT

%%
Program : ExtDefList {$$ = create_node("Program",1,$1);generate_InterCodes($$);output_InterCodes();}
  ;

ExtDefList : {$$ = create_node("ExtDefList",0,-1);}
  | ExtDef ExtDefList {$$ = create_node("ExtDefList",2,$1,$2);}
  ;

ExtDef : Specifier ExtDecList SEMI {$$ = create_node("ExtDef",3,$1,$2,$3);
struct Node *temp = $2;
while(1){
	if($1->attribute==NULL)
		break;
	if(exist(1,temp->left_child->attribute)) 
		print_semantic_error3(yylineno,temp->left_child->attribute);
	else if(temp -> kind == BASIC)
		new_symbol(3,1,$1,temp);
	else if(temp -> kind == ARRAY){
		new_symbol(3,3,$1,temp);
	}
	if(temp ->left_child -> right_child != NULL)
		temp = temp -> left_child -> right_child -> right_child;
	else break;
}
}
  | Specifier SEMI {$$ = create_node("ExtDef",2,$1,$2);}
  | Specifier FunDec CompSt {$$ = create_node("ExtDef",3,$1,$2,$3);
  		if(exist(2,$2->attribute))
			print_semantic_error4($2->attribute);	
  		else if(!check_return_type($1,$3))
  			print_semantic_error8();
  		else{
  			create_function($1,$2);
  		}
  	}
  ;

ExtDecList : VarDec {$$ = create_node("ExtDecList",1,$1);}
  | VarDec COMMA ExtDecList {$$ = create_node("ExtDecList",3,$1,$2,$3);$$->kind=$1->kind;$$->attribute=$1->attribute;
  }
  ;

Specifier : TYPE {$$ = create_node("Specifier",1,$1);}
  | StructSpecifier {$$ = create_node("Specifier",1,$1);}
  ;

StructSpecifier : STRUCT OptTag LC DefList RC {$$ = create_node("StructSpecifier",5,$1,$2,$3,$4,$5);
if($2->left_child != NULL){
	if(exist_struct($2->attribute))
		print_semantic_error16($2->attribute);
	else{
		new_struct_symbol(2,$2,$4);
		$$->attribute = $2->attribute;
	}
}
else {	
	printf("no name struct.\n");
}
}
  | STRUCT Tag {$$ = create_node("StructSpecifier",2,$1,$2);
  		if(!exist_struct_type($2->attribute))
  			print_semantic_error17(yylineno,$2->attribute);
  		else{
  			$$->attribute = $2->attribute;
  		}
  }
  ;

OptTag : {$$ = create_node("OptTag",0,-1);}
  | ID {$$ = create_node("OptTag",1,$1);struct_begin = yylineno;}
  ;

Tag : ID {$$ = create_node("Tag",1,$1);}
  ;

VarDec : ID {$$ = create_node("VarDec",1,$1);$$->kind = BASIC;}
  | VarDec LB INT RB {$$ = create_node("VarDec",4,$1,$2,$3,$4);$$->attribute=$1->attribute;$$->kind = ARRAY;}
  ;

FunDec : ID LP VarList RP {$$ = create_node("FunDec",4,$1,$2,$3,$4);
	$$->attribute = $1->attribute;
	function_begin = yylineno;
	struct Node *temp = $3->left_child;
	while(1){
		if(temp->left_child->attribute==NULL)
			break;
		if(exist(1,temp->left_child->right_child->attribute)) 
			print_semantic_error3(yylineno,temp->left_child->attribute);
		else if(temp -> left_child ->right_child -> kind == BASIC)
			new_symbol(3,1,temp->left_child,temp->left_child->right_child);
		else if(temp -> left_child->right_child -> kind == ARRAY){
			new_symbol(3,3,temp->left_child,temp->left_child->right_child);
	}
	if(temp -> right_child != NULL)
		temp = temp -> right_child -> right_child->left_child;
	else break;
	}
}
  | ID LP RP {$$ = create_node("FunDec",3,$1,$2,$3);
  	$$->attribute = $1->attribute;
  	function_begin = yylineno;
  	}
  ;

VarList : ParamDec COMMA VarList {$$ = create_node("VarList",3,$1,$2,$3);}
  | ParamDec {$$ = create_node("VarList",1,$1);}
  ;

ParamDec : Specifier VarDec {$$ = create_node("ParamDec",2,$1,$2);}
  ;

CompSt : LC DefList StmtList RC {$$ = create_node("CompSt",4,$1,$2,$3,$4);
	$$->attribute = $3->attribute;
	$$->kind = $3->kind;
	}
  ;

StmtList : {$$ = create_node("StmtList",0,-1);}
  | Stmt StmtList {$$ = create_node("StmtList",2,$1,$2);
  	if(strcmp($1->left_child->name,"RETURN")==0){
  		$$->attribute = $1->attribute;
  		$$->kind = $1->kind;
  		}
  	else{
  		$$->attribute = $2->attribute;
  		$$->kind = $2->kind;
  	}
  	}
  ;

Stmt : Exp SEMI {$$ = create_node("Stmt",2,$1,$2);}
  | CompSt {$$ = create_node("Stmt",1,$1);}
  | RETURN Exp SEMI {$$ = create_node("Stmt",3,$1,$2,$3);
  					$$->attribute = $2->attribute;
  					$$->kind = $2->kind;
  					return_line = yylineno;
  }
  | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {$$ = create_node("Stmt",5,$1,$2,$3,$4,$5);}
  | IF LP Exp RP Stmt ELSE Stmt {$$ = create_node("Stmt",7,$1,$2,$3,$4,$5,$6,$7);}
  | WHILE LP Exp RP Stmt {$$ = create_node("Stmt",5,$1,$2,$3,$4,$5);}
  ;

DefList : {$$ = create_node("DefList",0,-1);}
  | Def DefList {$$ = create_node("DefList",2,$1,$2);}
  ;

Def : Specifier DecList SEMI {$$ = create_node("Def",3,$1,$2,$3);
struct Node *temp = $2->left_child;
while(1){
	if($1->attribute==NULL){
		break;
	}
	if(exist(1,temp->left_child->attribute)) 
		print_semantic_error3(yylineno,temp->left_child->attribute);
	else if(temp -> kind == BASIC){
		//printf("%s\n",temp->left_child->attribute);
		new_symbol(3,1,$1,temp);
	}
	else if(temp -> kind == ARRAY){
		new_symbol(3,3,$1,temp);
	}
	if(temp -> right_child != NULL)
		temp = temp -> right_child -> right_child->left_child;
	else break;
}
}
  | error SEMI {is_right = 0;yyerrok;}
  ;

DecList : Dec {$$ = create_node("DecList",1,$1);}
  | Dec COMMA DecList {$$ = create_node("DecList",3,$1,$2,$3);$$->kind = $3->kind;}
  ;

Dec : VarDec {$$ = create_node("Dec",1,$1);}
  | VarDec ASSIGNOP Exp {$$ = create_node("Dec",3,$1,$2,$3);$$->attribute = $1->attribute;}
  ;

Exp : Exp ASSIGNOP Exp {$$ = create_node("Exp",3,$1,$2,$3);
							//check_assignop($1,$3);
						}
  | Exp AND Exp {$$ = create_node("Exp",3,$1,$2,$3);}
  | Exp OR Exp {$$ = create_node("Exp",3,$1,$2,$3);}
  | Exp RELOP Exp {$$ = create_node("Exp",3,$1,$2,$3);}
  | Exp PLUS Exp {$$ = create_node("Exp",3,$1,$2,$3);
  					//if(!check_op_types($1,$3))
  						//print_semantic_error7(yylineno);
  					}
  | Exp MINUS Exp {$$ = create_node("Exp",3,$1,$2,$3);
  					//if(!check_op_types($1,$3))
  						//print_semantic_error7(yylineno);
  					}
  | Exp STAR Exp {$$ = create_node("Exp",3,$1,$2,$3);
  					//if(!check_op_types($1,$3))
  						//print_semantic_error7(yylineno);

  					}
  | Exp DIV Exp {$$ = create_node("Exp",3,$1,$2,$3);
  					//if(!check_op_types($1,$3))
  						//print_semantic_error7(yylineno);
  					}
  | LP Exp RP {$$ = create_node("Exp",3,$1,$2,$3);}
  | MINUS Exp {$$ = create_node("Exp",2,$1,$2);}
  | NOT Exp  {$$ = create_node("Exp",2,$1,$2);}
  | ID LP Args RP {$$ = create_node("Exp",4,$1,$2,$3,$4);
  					//if(exist_var($1->attribute))
  						//print_semantic_error11(yylineno,$1->attribute);
  					//else if(!exist_function($1->attribute))
  						//print_semantic_error2(yylineno,$1->attribute);
  					
  					//else if(!check_args($1->attribute,$3))
  						//print_semantic_error9(yylineno,$1->attribute);
  						}
  | ID LP RP {$$ = create_node("Exp",3,$1,$2,$3);
  					//if(exist_var($1->attribute))
  						//print_semantic_error11(yylineno,$1->attribute);
  					//else if(!exist_function($1->attribute))
  						//print_semantic_error2(yylineno,$1->attribute);
  					//else if(!check_args($1->attribute,NULL))
  						//print_semantic_error9(yylineno,$1->attribute);
  						}
  | Exp LB Exp RB {$$ = create_node("Exp",4,$1,$2,$3,$4);
  					if(!check_is_array($1->attribute))
  						print_semantic_error10(yylineno,$1->attribute);
  					else if(strcmp($3->left_child->name,"FLOAT") == 0)
  						print_semantic_error12(yylineno,$3->left_child->value_float);
  				  }
  | Exp DOT ID {$$ = create_node("Exp",3,$1,$2,$3);
  					if(!check_is_struct($1->attribute))
  						print_semantic_error13(yylineno,$1->attribute);
  					else if(!has_field($1->attribute,$3->attribute))
  						print_semantic_error14(yylineno,$3->attribute);
  					else {$$->attribute=$3->attribute;}
  				}
  | ID {$$ = create_node("Exp",1,$1);
  		if(!exist(1,$$->attribute))
  			print_semantic_error1(yylineno,$$->attribute);
  		}
  | INT {$$ = create_node("Exp",1,$1);
  }
  | FLOAT {$$ = create_node("Exp",1,$1);}
  ;

Args : Exp COMMA Args {$$ = create_node("Args",3,$1,$2,$3);}
  | Exp {$$ = create_node("Args",1,$1);}
  ;

%%

