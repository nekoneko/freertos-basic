#include "shell.h"
#include <stddef.h>
#include "clib.h"
#include <string.h>
#include "fio.h"
#include "filesystem.h"

#include "FreeRTOS.h"
#include "task.h"
#include "host.h"

#include <math.h> 	//sqrt(double)
#include <stdio.h> 	//sscanf(...)

typedef struct {
	const char *name;
	cmdfunc *fptr;
	const char *desc;
} cmdlist;

void ls_command(int, char **);
void man_command(int, char **);
void cat_command(int, char **);
void ps_command(int, char **);
void host_command(int, char **);
void help_command(int, char **);
void host_command(int, char **);
void mmtest_command(int, char **);

void test_command(int, char **);
	void nfib_command(int, char **);
	void ffib_command(int, char **);
		unsigned int ffib(unsigned int);
		unsigned int clz_c(unsigned int);

	void prime_command(int, char **);
		int prime(int);
		unsigned int atoi(const char *);

void _command(int, char **);

cmdfunc *do_test_command(const char *);

#define MKCL(n, d) {.name=#n, .fptr=n ## _command, .desc=d}

cmdlist cl[]={
	MKCL(ls, "List directory"),
	MKCL(man, "Show the manual of the command"),
	MKCL(cat, "Concatenate files and print on the stdout"),
	MKCL(ps, "Report a snapshot of the current processes"),
	MKCL(host, "Run command on host"),
	MKCL(mmtest, "heap memory allocation test"),
	MKCL(help, "help"),
	MKCL(test, "test new function"),
	MKCL(, ""),
};

cmdlist tl[]={
	MKCL(nfib, "show fibonacci by normal way"),
	MKCL(ffib, "show fibonacci by fast way"),
	MKCL(prime, "show prime"),
};

int parse_command(char *str, char *argv[]){
	int b_quote=0, b_dbquote=0;
	int i;
	int count=0, p=0;
	for(i=0; str[i]; ++i){
		if(str[i]=='\'')
			++b_quote;
		if(str[i]=='"')
			++b_dbquote;
		if(str[i]==' '&&b_quote%2==0&&b_dbquote%2==0){
			str[i]='\0';
			argv[count++]=&str[p];
			p=i+1;
		}
	}
	/* last one */
	argv[count++]=&str[p];

	return count;
}

void ls_command(int n, char *argv[]){
    fio_printf(1,"\r\n"); 
    int dir;
    if(n == 0){
        dir = fs_opendir("");
    }else if(n == 1){
        dir = fs_opendir(argv[1]);
        //if(dir == )
    }else{
        fio_printf(1, "Too many argument!\r\n");
        return;
    }
(void)dir;   // Use dir
}

int filedump(const char *filename){
	char buf[128];

	int fd=fs_open(filename, 0, O_RDONLY);

	if( fd == -2 || fd == -1)
		return fd;

	fio_printf(1, "\r\n");

	int count;
	while((count=fio_read(fd, buf, sizeof(buf)))>0){
		fio_write(1, buf, count);
    }
	
    fio_printf(1, "\r");

	fio_close(fd);
	return 1;
}

void ps_command(int n, char *argv[]){
	signed char buf[1024];
	vTaskList(buf);
        fio_printf(1, "\n\rName          State   Priority  Stack  Num\n\r");
        fio_printf(1, "*******************************************\n\r");
	fio_printf(1, "%s\r\n", buf + 2);	
}

void cat_command(int n, char *argv[]){
	if(n==1){
		fio_printf(2, "\r\nUsage: cat <filename>\r\n");
		return;
	}

    int dump_status = filedump(argv[1]);
	if(dump_status == -1){
		fio_printf(2, "\r\n%s : no such file or directory.\r\n", argv[1]);
    }else if(dump_status == -2){
		fio_printf(2, "\r\nFile system not registered.\r\n", argv[1]);
    }
}

void man_command(int n, char *argv[]){
	if(n==1){
		fio_printf(2, "\r\nUsage: man <command>\r\n");
		return;
	}

	char buf[128]="/romfs/manual/";
	strcat(buf, argv[1]);

    int dump_status = filedump(buf);
	if(dump_status < 0)
		fio_printf(2, "\r\nManual not available.\r\n");
}

void host_command(int n, char *argv[]){
    int i, len = 0, rnt;
    char command[128] = {0};

    if(n>1){
        for(i = 1; i < n; i++) {
            memcpy(&command[len], argv[i], strlen(argv[i]));
            len += (strlen(argv[i]) + 1);
            command[len - 1] = ' ';
        }
        command[len - 1] = '\0';
        rnt=host_action(SYS_SYSTEM, command);
        fio_printf(1, "\r\nfinish with exit code %d.\r\n", rnt);
    } 
    else {
        fio_printf(2, "\r\nUsage: host 'command'\r\n");
    }
}

void help_command(int n,char *argv[]){
	int i;
	fio_printf(1, "\r\n");
	for(i = 0;i < sizeof(cl)/sizeof(cl[0]) - 1; ++i){
		fio_printf(1, "%s - %s\r\n", cl[i].name, cl[i].desc);
	}
}

void test_command(int n, char *argv[]) {

	if (n>1) {
		cmdfunc *fptr= do_test_command(argv[1]);
		if(fptr!=NULL)
			fptr(n, argv);
		else
			fio_printf(2, " \"%s\" not found for \"test\"", argv[1]);
	}
	else {
        fio_printf(2, "Usage: test 'command'\r\n");
	}
}

cmdfunc *do_test_command(const char *cmd) {

	int i;

	for(i=0; i<sizeof(tl)/sizeof(tl[0]); ++i){
		if(strcmp(tl[i].name, cmd)==0)
			return tl[i].fptr;
	}
	return NULL;
}

void nfib_command(int n, char *argv[]) {
	int i, a, b, number;

	if(n>3) {
		fio_printf(2,"\r\ntoo many argument, Usage: test nfib 'number'\r\n");
		return;
	}

	//test nfib ==> as 'test nfib 10'
	if(n==2) number = 10;

	if(n==3) number = atoi(argv[2]);

	if(number>46) number=46;

	a = 0;
	b = 1;

	fio_printf(1, "\r\nNormal Fibonacci : A0 = 0\r\n");
	if(number<=0) 	return;

	fio_printf(1, "Normal Fibonacci : A1 = 1\r\n");
    if(number==1) 	return;

	for(i = 2;i <= number;i++) {
		a+=b;

		a^=b;
		b^=a;
		a^=b;

    	fio_printf(1, "Normal Fibonacci : A%d = %d\r\n", i, b);
	}
	return;
}

void ffib_command(int n, char *argv[]) {
	
	int number;

	fio_printf(1, "\r\n");

	if(n==2) {
		fio_printf(2, "Usage : test ffib \'number\'\r\n");
		return;
	}

	if(n>3) {
		fio_printf(2, "too many argument, Usage : test ffib \'number\'\r\n");
		return;
	}

	number = atoi(argv[2]);

	number = (number>46)?46:number;

	ffib(number);

	return;
}

unsigned int ffib(unsigned int n) {

	unsigned int a,b,t1,cmp_unit,id;

	a=0;
	b=1;
	id=0;
	cmp_unit = 0x80000000>>clz_c(n);

	for(;cmp_unit > 0;cmp_unit>>=1) {
		t1=a*(2*b-a);
		b=b*b+a*a;
		a=t1;
		id<<=1;
		if(n&cmp_unit) {
			t1=a+b;
			a=b;
			b=t1;
			id+=1;
		}

		fio_printf(2,"Fast Fibonacc A%d = %d\r\n", id, a);
	}
	return a;
}

/*
 * Copyright (C) 2007 The Android Open Source Project
 * http://www.netmite.com/android/mydroid/dalvik/vm/alloc/clz.c
 */
//it maybe be modified with inline assembly 'clz...'
unsigned int clz_c(unsigned int x) {
	//input variable type must be 'unsigned'
	unsigned int e;
	if(!x) return 32;
	e = 31;
	if(x&0xFFFF0000) {e-=16; x>>=16;}
	if(x&0x0000FF00) {e-=8 ; x>>=8; }
	if(x&0x000000F0) {e-=4 ; x>>=4; }
	if(x&0x0000000C) {e-=2 ; x>>=2; }
	if(x&0x00000002) {e-=1;}
	return e;
}

void prime_command(int n, char *argv[]) {

	unsigned int i,number;

	fio_printf(2,"\r\n");

	if(n==2) {
		fio_printf(2,"Usage : test prime \'number\'\r\n");
		return;
	}

	if(n>3) {
		fio_printf(2,"too many argument, Usage : test prime \'number\'\r\n");
		return;
	}

	number = atoi(argv[2]);

	fio_printf(1,"prime below %d\r\n",number);

	for(i=2;i<=number;++i) {
		if(prime(i)) {
			fio_printf(1,"%d\r\n",i);
		}
	}

	return;
}

int prime(int n) {
	unsigned int i;
	for(i=2;i<n;++i) {
		if(!(n%i)) return 0;
	}
	return 1;
}


/*
 * Reference : https://gist.github.com/good5dog5/e97f2fe6d59149006a80#file-gistfile1-c
 * */
unsigned int atoi(const char *str) {
	unsigned int result,c;
	result = 0;
	while(*str!=0) {
		c = *str - '0';
		result = result*10 + c;
		++str;
	}
	return result;
}

void _command(int n, char *argv[]){
    (void)n; (void)argv;
    fio_printf(1, "\r\n");
}

cmdfunc *do_command(const char *cmd){

	int i;

	for(i=0; i<sizeof(cl)/sizeof(cl[0]); ++i){
		if(strcmp(cl[i].name, cmd)==0)
			return cl[i].fptr;
	}
	return NULL;	
}
