/*
    ISA course project, 2015/2016
    Variant: FTP/SSH Honeypot
    Author: Dominika Regeciova, xregec00
*/

#ifndef __ftp_defined__
#define __ftp_defined__

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define RESULT_OK 0
#define RESULT_FAILURE 1

using namespace std;

// Function declarations
int Fake_FTP_Server(string address, int port, string logfile, int max_clients);

#endif