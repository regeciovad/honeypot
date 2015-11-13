/*
    ISA course project, 2015/2016
    Variant: FTP/SSH Honeypot
    Author: Dominika Regeciova, xregec00
*/

#include "ftp.hpp"
#include "logging.hpp"

using namespace std;

int signal_detected = 0;

typedef struct
{
    sockaddr_in client_address;
    int client_number;
}thread_data;

// Detection of SIGINT and SIGQUIT
void Signal_Catcher(int n)
{
    signal_detected = 1;
}

void * Connect(void *pointer)
{
    thread_data *client_thread = (thread_data *)pointer;
    string msg="220 Service ready for new user\r\n";
    if (send(client_thread->client_number, msg.c_str(), msg.length(), 0) < 0)
        Print_Error("Sending message error!");
    close(client_thread->client_number);
    pthread_exit(NULL);
}


int Fake_FTP_Server(string address, int port, string logfile, int max_clients)
{
    struct sockaddr_in server_address;
    struct sockaddr_in6 server_address6;
    socklen_t sa_len;
    string ip_version;
    int sock;
    // Detection of exit signals
    struct sigaction signal_action;
    signal_action.sa_handler = Signal_Catcher;
    sigemptyset (&signal_action.sa_mask);
    signal_action.sa_flags = 0;
    sigaction(SIGINT, &signal_action, NULL);
    sigaction(SIGQUIT, &signal_action, NULL);
    char str [INET6_ADDRSTRLEN];
    int optval = 1;

    // Control of IP address and creating of socket, bind
    if (inet_pton(AF_INET, address.c_str(), &server_address.sin_addr) > 0)
    //if (true)
    {
        ip_version = "IPv4";
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            Print_Error("Socket creation error!");
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(port);
        inet_ntop(AF_INET, &server_address.sin_addr, str, INET6_ADDRSTRLEN);
        cout << str << endl;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval, sizeof(optval));
        if (bind(sock,(struct sockaddr *)&server_address, sizeof(server_address))<0)
            Print_Error("Bind error!");
        sa_len = sizeof(server_address);
    }
    else if (inet_pton(AF_INET6, address.c_str(), &(server_address6.sin6_addr)) > 0)
    {
        ip_version = "IPv6";
        if ((sock = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
            Print_Error("Socket creation error!");
        server_address6.sin6_family = AF_INET6;
        server_address6.sin6_port = htons(port);
        inet_ntop(AF_INET6, &server_address6.sin6_addr, str, INET6_ADDRSTRLEN);
        cout << str << endl;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
        if (bind(sock, (struct sockaddr *)&server_address6, sizeof(server_address6))<0)
            Print_Error("Bind error!");
        sa_len = sizeof(server_address6);
    }
    else
        Print_Error("Wrong format of IP address!");

    // Inform OS, that server is accepting connections from users
    if (listen(sock, max_clients)<0)
        Print_Error("Socket queue error!");

    // Until SIGINT(or SIGQUIT) do
    do
    {
        sigaction(SIGINT, &signal_action, NULL);
        sigaction(SIGQUIT, &signal_action, NULL);
        thread_data *data = new thread_data;
        int client;
        sa_len = sizeof(data->client_address);
        if ((client = accept(sock, (struct sockaddr *) &(data->client_address), &sa_len)) < 0)
            Print_Error("Accept error!");
        data->client_number=client;

        if (signal_detected)
            break;

        pthread_t new_thread;
        if (pthread_create(&new_thread, NULL, Connect, (void *)data) != 0)
            Print_Error("New thread creation error!");
        delete [] data;
    }while (!signal_detected);

    //cout << Get_Time() << endl;
    // Cleanup
    if (close(sock)<0)
        Print_Error("Closing socket error!");
    pthread_exit(NULL);
    return RESULT_OK;
}