/*
    ISA course project, 2015/2016
    Variant: FTP/SSH Honeypot
    Author: Dominika Regeciova, xregec00
*/

#include "ftp.hpp"
#include "logging.hpp"

using namespace std;

int signal_detected = 0;
string server_logfile;

pthread_mutex_t mutex_logfile = PTHREAD_MUTEX_INITIALIZER;

struct thread_data
{
    char client_address [INET6_ADDRSTRLEN];
    int client_number;
};

// Detection of SIGINT and SIGQUIT
void Signal_Catcher(int n)
{
    signal_detected = 1;
}

void * Connect(void *pointer)
{
    struct thread_data *client_thread = (struct thread_data *)pointer;
    cout << client_thread->client_address << endl;
    cout << server_logfile << endl;
    string address = string(client_thread->client_address);
    string name = "Karel";
    string password = "mojexxx";
    string msg="220 Service ready for new user\r\n";
    if (send(client_thread->client_number, msg.c_str(), msg.length(), 0) < 0)
        Print_Error("Sending message error!");
    pthread_mutex_lock(&mutex_logfile);
    write_log(server_logfile, "FTP", address, name, password);
    pthread_mutex_unlock(&mutex_logfile);
    close(client_thread->client_number);
    //free(pointer);
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
        struct sockaddr_storage client_address;
        struct thread_data data;
        int client;
        sa_len = sizeof(client_address);
        if ((client = accept(sock, (struct sockaddr *)&client_address, &sa_len)) < 0)
            return (RESULT_OK);
        getpeername(client, (struct sockaddr *)&client_address, &sa_len);
        if (client_address.ss_family == AF_INET)
        {
            struct sockaddr_in * c_address = (struct sockaddr_in *)&client_address;
            inet_ntop(AF_INET, &c_address->sin_addr, data.client_address, INET6_ADDRSTRLEN);
        }
        else
        {
            struct sockaddr_in6 * c_address = (struct sockaddr_in6 *)&client_address;
            inet_ntop(AF_INET6, &c_address->sin6_addr, data.client_address, INET6_ADDRSTRLEN);
        }
        data.client_number = client;
        server_logfile = logfile;

        if (signal_detected)
            break;

        pthread_t new_thread;
        if (pthread_create(&new_thread, NULL, &Connect, (void *)&data) != 0)
            Print_Error("New thread creation error!");
    }while (!signal_detected);

    // Cleanup
    if (close(sock)<0)
        Print_Error("Closing socket error!");
    pthread_exit(NULL);
    return RESULT_OK;
}