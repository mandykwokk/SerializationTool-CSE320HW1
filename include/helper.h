#ifndef HELPER_H
#define HELPER_H

#include "transplant.h"

int currentType;

int currentDepth;

int currentSize;

int previousType;

int previousDepth;

int currentSTmode = -1;

int currentSTsize = -1;

int count0=0;

int count1=0;

int count2=0;

int count3=0;

int count4=0;

int count5=0;

int test = 0;

struct stat stat_buf;

int equalstring(char* str1, char* str2);

int validDir(char* x);

int checkName(char *name);

int processHeader();

int power(int a,int b);

void clearNameBuf();

void putMagic();

void putFour(int depth);

void putEight(int x);

void putHeader(int a,int b, int c);

int length(char* s);

#endif
