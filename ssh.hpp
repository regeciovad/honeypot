/*
    ISA course project, 2015/2016
    Variant: FTP/SSH Honeypot
    Author: Dominika Regeciova, xregec00
*/

#ifndef __ssh_defined__
#define __ssh_defined__

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <libssh/libssh.h>
#include <libssh/server.h>
#include <libssh/callbacks.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#define MAXLENMESS 1024

using namespace std;

// Function declarations
void Ssh_Signal_Catcher(int n);
void * Authentication(void *pointer);
void Fake_SSH_Server(string address, int port, string logfile, int max_clients, string rsakey, int max_attempts);

#endif