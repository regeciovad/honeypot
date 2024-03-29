/*
    ISA course project, 2015/2016
    Variant: FTP/SSH Honeypot
    Author: Dominika Regeciova, xregec00
*/

#include "ftp.hpp"

// Global variables and structure
int signal_detected = 0;
int clients_amount = 0;
string ftp_server_logfile;
// Two mutex locks - one for logfile and second for variable watching active clientes
pthread_mutex_t mutex_logfile = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_clients = PTHREAD_MUTEX_INITIALIZER;

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

// Function for authentication of users
void * Connect(void *pointer)
{
    
    struct thread_data *client_thread = (struct thread_data *)pointer;
    char buffer[MAXLENMESS];
    int recieved;
    string address = string(client_thread->client_address);
    string name = "";
    string password = "";
    string msg = "220 Service ready for new user.\r\n";
    string client_msg;
    bool continue_bool = true;

    // Send information about server is ready
    if (send(client_thread->client_number, msg.c_str(), msg.length(), 0) < 0)
    {
        continue_bool = false;
        Print_Error("Sending message error!");
    }

    while(continue_bool)
    {
        // Get message from user
        recieved = recv(client_thread->client_number, buffer, MAXLENMESS, 0);
        if (recieved == 0)
        {
            continue_bool = false;
            Print_Error("Closed connection by user.");
            continue;
        }
        else if (recieved < 0)
        {
            continue_bool = false;
            Print_Error("Recv error!");
            continue;
        }
        // User sent username
        client_msg = string(buffer);
        if (client_msg.substr(0,4) == "USER")
        {
            name = "";
            for (int x=4; x<recieved; x++)
            {
                if (!isspace(buffer[x])) 
                    name.push_back(buffer[x]);
            }
            msg = "331 User name okay, need password.\r\n";
            if (send(client_thread->client_number, msg.c_str(), msg.length(), 0) < 0)
            {
                Print_Error("Sending message error!");
                continue_bool = false;
                continue;
            }
        }
        // User sent password
        else if (client_msg.substr(0,4) == "PASS")
        {
            password = "";
            for (int x=4; x<recieved; x++)
            {
                if (!isspace(buffer[x])) 
                    password.push_back(buffer[x]);
            }
            msg = "530 Not logged in.\r\n";
            if (send(client_thread->client_number, msg.c_str(), msg.length(), 0) < 0)
                Print_Error("Sending message error!");
            continue_bool = false;
            continue;
        }
        // User quits connection before sending password
        else if (client_msg.substr(0,4) == "QUIT")
        {
            msg = "221 Service closing control connection.\r\n";
            if (send(client_thread->client_number, msg.c_str(), msg.length(), 0) < 0)
                Print_Error("Sending message error!");
            continue_bool = false;
            continue;
        }
        else
        {
            continue_bool = false;
            continue;
        }

    }
    // Log in information
    pthread_mutex_lock(&mutex_logfile);
    if (write_log(ftp_server_logfile, "FTP", address, name, password) == RESULT_FAILURE)
        Print_Error("Logging error!");
    // Cleanup after closing connection
    pthread_mutex_unlock(&mutex_logfile);
    close(client_thread->client_number);
    free(pointer);
    pthread_mutex_lock(&mutex_clients);
    clients_amount = clients_amount - 1;
    pthread_mutex_unlock(&mutex_clients);
    pthread_exit(NULL);
}

// The function to setup of server, find out of ip adress, creating of new pthread and starting Connection
void Fake_FTP_Server(string address, int port, string logfile, int max_clients)
{
    ftp_server_logfile = logfile;
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
    {
        ip_version = "IPv4";
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            Print_Error_And_Exit("Socket creation error!");
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(port);
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval, sizeof(optval));
        if (bind(sock,(struct sockaddr *)&server_address, sizeof(server_address))<0)
            Print_Error_And_Exit("Bind error!");
        sa_len = sizeof(server_address);
    }
    else if (inet_pton(AF_INET6, address.c_str(), &(server_address6.sin6_addr)) > 0)
    {
        ip_version = "IPv6";
        if ((sock = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
            Print_Error_And_Exit("Socket creation error!");
        server_address6.sin6_family = AF_INET6;
        server_address6.sin6_port = htons(port);
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
        if (bind(sock, (struct sockaddr *)&server_address6, sizeof(server_address6))<0)
            Print_Error_And_Exit("Bind error!");
        sa_len = sizeof(server_address6);
    }
    else
        Print_Error_And_Exit("Wrong format of IP address!");

    // Inform OS, that server is accepting connections from users
    if (listen(sock, max_clients)<0)
        Print_Error_And_Exit("Socket queue error!");

    // Until SIGINT(or SIGQUIT) do
    do
    {
        // New user
        pthread_mutex_lock(&mutex_clients);
        clients_amount = clients_amount + 1;
        pthread_mutex_unlock(&mutex_clients);
        struct sockaddr_storage client_address;
        struct thread_data * data = (struct thread_data *) malloc(sizeof(struct thread_data));
        int client;
        sa_len = sizeof(client_address);
        if ((client = accept(sock, (struct sockaddr *)&client_address, &sa_len)) < 0)
        {
            free(data);
            exit (RESULT_OK);
        }
        // Too many users, server is busy
        if (clients_amount > max_clients)
        {
            close(client);
            free(data);
            continue;
        }
        data->client_number = client;
        // Find out if client ip address is IPv4 or IPv6
        getpeername(client, (struct sockaddr *)&client_address, &sa_len);
        if (client_address.ss_family == AF_INET)
        {
            struct sockaddr_in * c_address = (struct sockaddr_in *)&client_address;
            inet_ntop(AF_INET, &c_address->sin_addr, data->client_address, INET6_ADDRSTRLEN);
        }
        else
        {
            struct sockaddr_in6 * c_address = (struct sockaddr_in6 *)&client_address;
            inet_ntop(AF_INET6, &c_address->sin6_addr, data->client_address, INET6_ADDRSTRLEN);
        }
        
        // Server quit
        if (signal_detected)
        {
            free(data);
            break;
        }

        // New thread, call connect with data
        pthread_t new_thread;
        if (pthread_create(&new_thread, NULL, &Connect, (void *)data) != 0)
        {
            close(client);
            free(data);
            Print_Error("New thread creation error!");
            break;
        }
    }while (!signal_detected);

    // Cleanup
    if (close(sock)<0)
        Print_Error("Closing socket error!");
    pthread_exit(NULL);
    pthread_mutex_destroy(&mutex_logfile);
    pthread_mutex_destroy(&mutex_clients);
}