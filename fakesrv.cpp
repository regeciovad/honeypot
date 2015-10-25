/*
    ISA course project, 2015/2016
    Variant: FTP/SSH Honeypot
    Author: Dominika Regeciova, xregec00
*/

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <ctime>
#include <getopt.h>
#include "fakesrv.hpp"

using namespace std;
const int BUFFERSIZE = 255;

// Function declarations
void printHelp();
string getTime();

int main(int argc, char *argv[])
{
    // ./fakesrv
    if (argc == 1)
    {
        printHelp();
        return RESULT_FAILURE;
    }

    // Arguments initialization
    int option = 0;
    string mode;
    string addr;
    int port = 0;
    string logfile;
    string rsakey;
    int max_clients = 10;
    int max_attempts = 3;

    // Arguments check
    while ((option = getopt(argc, argv, "m:a:p:l:r:c:t:")) != -1)
    {
        switch (option)
        {
            case 'm':
                mode = optarg;
                break;
            case 'a':
                addr = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'l':
                logfile = optarg;
                break;
            case 'r':
                rsakey = optarg;
                break;
            case 'c':
                max_clients = atoi(optarg);
                break;
            case 't':
                max_attempts = atoi(optarg);
                break;
            default:
                printHelp();
                return RESULT_FAILURE;
        }
    }
    // Mode control, it have to be 'ftp' or 'ssh'
    if (!mode.compare("ftp") and !mode.compare("ssh"))
    {
        cerr << "Wrond arguments (mode)!" << endl;
        return RESULT_FAILURE;
    }
    // If mode is ssh, rsakey must be specified
    if (mode == "ssh")
    {
        if (rsakey == "")
        {
            cerr << "Missing argument -r!" << endl;
            return RESULT_FAILURE;
        }
    }
    // IP adress control
    // DO TO: Control IP addrs
    // DO TO: Check if ip is ipv4 or ipv6
    if (addr == "")
    {
        cerr << "Missing argument -a!" << endl;
        return RESULT_FAILURE;
    }
    // Port number control
    if (port == 0)
    {
        cerr << "Missing argument -p!" << endl;
        return RESULT_FAILURE;
    }

    cout << "Mode: " << mode << endl;
    cout << "Addrs: " << addr << endl;
    cout << "Port: " << port << endl;
    cout << "Logfile: " << logfile << endl;
    cout << "RSAkey: " << rsakey << endl;
    cout << "Max client: " << max_clients << endl;
    cout << "Max attempts: " << max_attempts << endl;
    cout << getTime() << endl;
}

// Function prints help for application
void printHelp()
{
    cout << "FTP/SSH Honeypot" << endl;
    cout << "There will be help..." << endl;
}

// Function returning actual date and time in format: YYYY-MM-DD HH:MM:SS
string getTime()
{
    // Get current time and convert it to tm
    time_t t = time(0);
    tm now = *localtime(&t);
    char current_time[BUFFERSIZE] = {0};
    // hodiny jsou jinak %H-%M-%S
    const char format_time[] = "%Y-%m-%d %X";
    if (strftime(current_time, sizeof(current_time)-1, format_time, &now) > 0)
    {
        //string string_current_time(current_time);
        return string(current_time);
    }
    else
        return "Strftime failed.";
}
