#include <stdio.h>
#include "syntax.tab.h"
#include "create_gramm_tree.h"

extern void yyrestart(FILE *f);

int main(int argc,char **argv){
	if (argc <= 2) 
		return 1;
	output_file = argv[2];
	FILE* f = fopen(argv[1], "r");
	if (!f){
		perror(argv[1]);
		return 1;
	}
	yyrestart(f);
	yyparse();
	return 0;
}