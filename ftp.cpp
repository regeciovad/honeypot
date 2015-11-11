/*
    ISA course project, 2015/2016
    Variant: FTP/SSH Honeypot
    Author: Dominika Regeciova, xregec00
*/

#include "ftp.hpp"
#include "logging.hpp"

using namespace std;

int Fake_FTP_Server(string address, int port, string logfile, int max_clients)
{
    struct sockaddr_in server_address;
    struct sockaddr_in6 server_address6;
    string ip_version;
    int sock;

    if (inet_pton(AF_INET, address.c_str(), &(server_address.sin_addr)) != 0)
        ip_version = "IPv4";
    else if (inet_pton(AF_INET6, address.c_str(), &(server_address6.sin6_addr)) != 0)
        ip_version = "IPv6";
    else
        Print_Error("Wrong format of IP address!");

    // Creating of socket
    if (ip_version == "IPv4")
    {
        bzero(&server_address, sizeof(server_address));
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(port);
        sock = socket(AF_INET, SOCK_STREAM, 0);
    }
    else
    {
        bzero(&server_address6, sizeof(server_address6));
        server_address6.sin6_family = AF_INET6;
        server_address6.sin6_port = htons(port);
        sock = socket(AF_INET6, SOCK_STREAM, 0);
    }
    // Check
    if (sock<0)
        Print_Error("Socket creation error!");

    //cout << Get_Time() << endl;
    return RESULT_OK;
}