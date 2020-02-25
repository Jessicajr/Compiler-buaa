#define _CRT_SECURE_NO_WARNINGS
#include "lib.h"
#include "lexical.h"
#include "table.h"
#include"quad.h"
#include"register.h"
extern FILE* in, * out;

void openFile() {
	in = fopen("testfile.txt", "r");
	out = fopen("mips.txt", "w");
	//setbuf(out,NULL);
	//fprintf(out, "open");
	//fflush(out);
}

int main() {
	openFile();
	programme();
	mipsCode();
	fclose(in);
	fclose(out);
}


