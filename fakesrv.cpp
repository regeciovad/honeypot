/*
    ISA course project, 2015/2016
    Variant: FTP/SSH Honeypot
    Author: Dominika Regeciova, xregec00
*/

#include "fakesrv.hpp"

int main(int argc, char *argv[])
{
    // ./fakesrv or ./fakesrv --help (-h)
    if (argc == 1)
        Print_Help();

    // Arguments initialization
    int option = 0;
    string mode;
    string address;
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
        }
    }

    // IP adress control (only for missing argument)
    if (address == "")
        Print_Error_And_Exit("Missing argument -a!");

    // Port number control
    // Nummber 0 is reserved
    if (port == 0)
        Print_Error_And_Exit("Missing argument -p!");
    // Maximal value
    else if (port > 65535)
        Print_Error_And_Exit("Wrong argument -p!");

    // Logfile control
    if (logfile == "")
        Print_Error_And_Exit("Missing argument -l!");

    // Mode control, it have to be 'ftp' or 'ssh'
    // If mode is ssh, rsakey must be specified
    if (mode == "ssh")
    {
        if (rsakey == "")
            Print_Error_And_Exit("Missing argument -r!");
        // Fake SSH Server, which needs address, port number, logfile,
        // maximal number of clients, RSA key,
        // and maximal number of login attempts
        Fake_SSH_Server(address, port, logfile, max_clients, rsakey, max_attempts);
    }

    // if mode is ftp, rsakey and max attemps should not be defiend
    else if (mode == "ftp")
    {
        if (rsakey != "" || (max_attempts != 3))
            Print_Error_And_Exit("Too many argument for ftp mode!");
        // Fake FTP Server, which needs address, port number, logfile and
        // maximal number of clients.
        Fake_FTP_Server(address, port, logfile, max_clients);
    }
    // Mode was something else or missing
    else
        Print_Error_And_Exit("Wrong arguments (mode)!");
}
