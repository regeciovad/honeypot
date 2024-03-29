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
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <netdb.h>
#include "logging.hpp"

#define MAXLENMESS 1024

using namespace std;

// Function declarations
void Signal_Catcher(int n);
void * Connect(void *pointer);
void Fake_FTP_Server(string address, int port, string logfile, int max_clients);

#endif