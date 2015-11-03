/*
    ISA course project, 2015/2016
    Variant: FTP/SSH Honeypot
    Author: Dominika Regeciova, xregec00
*/

#include "fakesrv.hpp"
#include "logging.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    // ./fakesrv
    if (argc == 1)
    {
        Print_Help();
        return RESULT_FAILURE;
    }

    // Arguments initialization
    int option = 0;
    string mode;
    string address;
    int port = 0;
    string logfile;
    string rsakey;
    int max_clients = 10;
    int max_attempts = 3;
    struct sockaddr_in sa;
    struct sockaddr_in6 sa6;
    string ip_version;

    // Arguments check
    while ((option = getopt(argc, argv, "m:a:p:l:r:c:t:")) != -1)
    {
        switch (option)
        {
            case 'm':
                mode = optarg;
                break;
            case 'a':
                address = optarg;
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
                Print_Help();
                return RESULT_FAILURE;
        }
    }

    // IP adress control
    if (address == "")
    {
        cerr << "Missing argument -a!" << endl;
        return RESULT_FAILURE;
    }
    else if (inet_pton(AF_INET, address.c_str(), &(sa.sin_addr)) != 0)
    {
        ip_version = "IPv4";
    } 
    else if (inet_pton(AF_INET6, address.c_str(), &(sa6.sin6_addr)) != 0)
    {
        ip_version = "IPv6";
    }
    else
    {
        cerr << "Wrong format of IP address!" << endl;
        return RESULT_FAILURE;
    }

    // Port number control
    if (port == 0)
    {
        cerr << "Missing argument -p!" << endl;
        return RESULT_FAILURE;
    }
    else if (port > 65535)
    {
        cerr << "Wrong argument -p!" << endl;
        return RESULT_FAILURE;
    }

    // Logfile control
    if (logfile == "")
    {
        cerr << "Wrong argument -l!" << endl;
        return RESULT_FAILURE;
    }

    // Mode control, it have to be 'ftp' or 'ssh'
    // If mode is ssh, rsakey must be specified
    if (mode == "ssh")
    {
        if (rsakey == "")
        {
            cerr << "Missing argument -r!" << endl;
            return RESULT_FAILURE;
        }
    }
    // if mode is ftp, rsakey and max attemps should not be defiend
    else if (mode == "ftp")
    {
        if (rsakey != "" || (max_attempts != 3))
        {
            cerr << "Too many argument for ftp mode!" << endl;
            return RESULT_FAILURE;
        }
    }
    // Mode was something else or missing
    else
    {
        cerr << "Wrong arguments (mode)!" << endl;
        return RESULT_FAILURE;
    }

    cout << "Mode: " << mode << endl;
    cout << "Addresss: " << address << endl;
    cout << "Version: " << ip_version << endl;
    cout << "Port: " << port << endl;
    cout << "Logfile: " << logfile << endl;
    cout << "RSAkey: " << rsakey << endl;
    cout << "Max client: " << max_clients << endl;
    cout << "Max attempts: " << max_attempts << endl;
    cout << Get_Time() << endl;
}

// Function prints help for application
void Print_Help()
{
    cout << "FTP/SSH Honeypot" << endl;
    cout << "Usage: ./fakesrv -m mode -a address -p port -l logfile" << endl;
    cout << "                 [-r rsakey] [-c max_clients] [-t max_attempts]" << endl;
    cout << "Required arguments:" << endl;
    cout << "   -m mode       Mode name (ftp or ssh)" << endl;
    cout << "   -a address    IPv4 or IPv6 address" << endl;
    cout << "   -p port       Port number" << endl;
    cout << "   -l logfile    Logfile" << endl;
    cout << "Optional arguments:" << endl;
    cout << "   -r rsakey     Private RSA key, REQUIRED in ssh mode" << endl;
    cout << "   -c number     Maximal number of connected users (default 10)" << endl;
    cout << "   -t number     Maximal number of login tries (default 3) in ssh mode" << endl;
    cout << "Created by Dominika Regeciova" << endl;
}
