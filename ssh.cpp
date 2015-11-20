/*
    ISA course project, 2015/2016
    Variant: FTP/SSH Honeypot
    Author: Dominika Regeciova, xregec00
*/

#include "ssh.hpp"

// Global variables and structure
string ssh_server_logfile;
int max_attempts_number;
int ssh_signal_detected = 0;
int ssh_clients_amount = 0;
pthread_mutex_t ssh_mutex_logfile = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ssh_mutex_clients = PTHREAD_MUTEX_INITIALIZER;

struct thread_ssh_data
{
    char client_address [INET6_ADDRSTRLEN];
    ssh_session client_number;
};

// Detection of SIGINT and SIGQUIT
void Ssh_Signal_Catcher(int n)
{
    ssh_signal_detected = 1;
}

// Function for authentication of users
void * Authentication(void *pointer)
{
    struct thread_ssh_data *client_thread = (struct thread_ssh_data *)pointer;
    string address = string(client_thread->client_address);
    string name = "";
    string password = "";
    ssh_message recieved;
    bool continue_bool = true;
    int attempts = 0;
    // Comunicate with clients
    while(continue_bool)
    {
        recieved = ssh_message_get(client_thread->client_number);
        if (recieved == NULL)
        {
            continue_bool = false;
            Print_Error("Closed connection by user.");
            continue;
        }
        if (ssh_message_type(recieved) == SSH_REQUEST_AUTH)
        {
            if (ssh_message_subtype(recieved) == SSH_AUTH_METHOD_PASSWORD)
            {
                attempts = attempts + 1;
                name = ssh_message_auth_user(recieved);
                password = ssh_message_auth_password(recieved);
                pthread_mutex_lock(&ssh_mutex_logfile);
                if (write_log(ssh_server_logfile, "SSH", address, name, password) == RESULT_FAILURE)
                    Print_Error("Logging error!");
                pthread_mutex_unlock(&ssh_mutex_logfile);
                if (attempts == max_attempts_number)
                {
                    continue_bool = false;
                    Print_Error("Connection closed.");
                    continue;
                }
                ssh_message_auth_set_methods(recieved, SSH_AUTH_METHOD_PASSWORD);
                ssh_message_reply_default(recieved);
            }
            else
            {
                name = ssh_message_auth_user(recieved);
                ssh_message_auth_set_methods(recieved, SSH_AUTH_METHOD_PASSWORD);
                ssh_message_reply_default(recieved);
            }
        }
        else
        {
            ssh_message_reply_default(recieved);
        }
        ssh_message_free(recieved);
    }
    pthread_mutex_lock(&ssh_mutex_logfile);
    if (password == "")
    {
        if (write_log(ssh_server_logfile, "SSH", address, name, password) == RESULT_FAILURE)
            Print_Error("Logging error!");
    }
    pthread_mutex_unlock(&ssh_mutex_logfile);
    ssh_disconnect(client_thread->client_number);
    free(pointer);
    pthread_mutex_lock(&ssh_mutex_clients);
    ssh_clients_amount = ssh_clients_amount - 1;
    pthread_mutex_unlock(&ssh_mutex_clients);
    pthread_exit(NULL);
}

// The function to setup of server, find out of ip adress, creating of new pthread and starting Authentication
void Fake_SSH_Server(string address, int port, string logfile, int max_clients, string rsakey, int max_attempts)
{
    ssh_server_logfile = logfile;
    max_attempts_number = max_attempts;
    // Detection of exit signals
    struct sigaction ssh_signal_action;
    ssh_signal_action.sa_handler = Ssh_Signal_Catcher;
    sigemptyset (&ssh_signal_action.sa_mask);
    ssh_signal_action.sa_flags = 0;
    sigaction(SIGINT, &ssh_signal_action, NULL);
    sigaction(SIGQUIT, &ssh_signal_action, NULL);
    socklen_t sa_len;

    ssh_session new_session = ssh_new();
    if (new_session == NULL)
        Print_Error_And_Exit("Creating new session error!");
    ssh_bind new_bind = ssh_bind_new();
    if (new_bind == NULL)
        Print_Error_And_Exit("Bind error!");

    // Bind to address, port and rsakey
    ssh_bind_options_set(new_bind, SSH_BIND_OPTIONS_BINDADDR, address.c_str());
    ssh_bind_options_set(new_bind, SSH_BIND_OPTIONS_BINDPORT, &port);
    ssh_bind_options_set(new_bind, SSH_BIND_OPTIONS_RSAKEY, rsakey.c_str());

    // Initialization of threads
    ssh_threads_set_callbacks(ssh_threads_get_pthread());

    if (ssh_init()<0)
        Print_Error_And_Exit("Initialization of cryptographic data structures!");

    if (ssh_bind_listen(new_bind) < 0 )
        Print_Error_And_Exit(ssh_get_error(new_bind));

    // Until SIGINT(or SIGQUIT) do
    do
    {
        pthread_mutex_lock(&ssh_mutex_clients);
        ssh_clients_amount = ssh_clients_amount + 1;
        pthread_mutex_unlock(&ssh_mutex_clients);
        // Variables for find out clients ip addresss
        struct sockaddr_storage ssh_client_address;
        struct thread_ssh_data * ssh_data = (struct thread_ssh_data *) malloc(sizeof(struct thread_ssh_data));
        sa_len = sizeof(ssh_client_address);
        ssh_data->client_number = ssh_new();
        if (ssh_bind_accept(new_bind, ssh_data->client_number) < 0)
        {
            free(ssh_data);
            ssh_bind_free(new_bind);
            ssh_free(new_session);
            exit (RESULT_OK);
        }
        int client = ssh_get_fd(ssh_data->client_number);
        if (ssh_clients_amount > max_clients)
        {
            ssh_disconnect(ssh_data->client_number);
            free(ssh_data);
            continue;
        }
        // Find out if client ip address is IPv4 or IPv6
        getpeername(client, (struct sockaddr *)&ssh_client_address, &sa_len);
        if (ssh_client_address.ss_family == AF_INET)
        {
            struct sockaddr_in * c_address = (struct sockaddr_in *)&ssh_client_address;
            inet_ntop(AF_INET, &c_address->sin_addr, ssh_data->client_address, INET6_ADDRSTRLEN);
        }
        else
        {
            struct sockaddr_in6 * c_address = (struct sockaddr_in6 *)&ssh_client_address;
            inet_ntop(AF_INET6, &c_address->sin6_addr, ssh_data->client_address, INET6_ADDRSTRLEN);
        }
        // Exchange of keys
        if (ssh_handle_key_exchange(ssh_data->client_number) < 0)
            Print_Error_And_Exit(ssh_get_error(ssh_data->client_number));
        // Server quit
        if (ssh_signal_detected)
            break;

        pthread_t new_thread;
        if (pthread_create(&new_thread, NULL, &Authentication, (void *)ssh_data) != 0)
        {
            ssh_disconnect(ssh_data->client_number);
            free(ssh_data);
            Print_Error("New thread creation error!");
            break;
        }
    }while (!ssh_signal_detected);

    // Cleanup
    ssh_bind_free(new_bind);
    ssh_free(new_session);
    ssh_finalize();
    pthread_exit(NULL);
    pthread_mutex_destroy(&ssh_mutex_logfile);
    pthread_mutex_destroy(&ssh_mutex_clients);
}