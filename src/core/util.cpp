#include "util.h"
#include <iostream>
using namespace std;

// Primitive helper utilities, clamping, and screen control

bool stringEquals(const char* a, const char* b){
    if(a == nullptr || b==nullptr) return false;

    int i = 0;
    while(a[i]!='\0' && b[i] !='\0'){
        if(a[i] != b[i]) return false;
        i++;
    }

    return a[i] == b[i];
}
int clampInt(int val, int minVal, int maxVal){
    if(val < minVal) val = minVal;
    else if(val > maxVal) val = maxVal;

    return val;
}
void trimWhitespace(char* str){
    if(str == nullptr) return;

    int count = 0;
    while(str[count] == ' ' || str[count] == '\t') count++;

    int length=0;
    while(str[count]!='\0'){
        str[length] = str[count];
        length++;
        count++;
    }

     while (length > 0 &&
           (str[length - 1] == ' ' || str[length - 1] == '\t')) {
        length--;
    }

    str[length] = '\0';
}

void clearScreen()
{
    cout << "\033[2J\033[H";
    cout.flush();
}

void printHeader(const char* title){

    cout << endl << "\033[32m==== " << title << " ====\033[0m" << endl;
}