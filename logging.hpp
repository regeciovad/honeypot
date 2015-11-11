/*
    ISA course project, 2015/2016
    Variant: FTP/SSH Honeypot
    Author: Dominika Regeciova, xregec00
*/

#ifndef __logging_defined__
#define __logging_defined__

#include <iostream>
#include <string.h>
#include <ctime>

using namespace std;

#define RESULT_FAILURE 1

// Function declarations
void Print_Error(string message);
string Get_Time();

#endif