/*
    ISA course project, 2015/2016
    Variant: FTP/SSH Honeypot
    Author: Dominika Regeciova, xregec00
*/

#include "ssh.hpp"
#include "logging.hpp"

using namespace std;

string ssh_server_logfile;
int ssh_signal_detected = 0;
int max_attempts_number;

pthread_mutex_t ssh_mutex_logfile = PTHREAD_MUTEX_INITIALIZER;

struct thread_ssh_data
{
    char client_address [INET6_ADDRSTRLEN];
    ssh_session client_number;
};

void Ssh_Signal_Catcher(int n)
{
    ssh_signal_detected = 1;
}

void * Authentication(void *pointer)
{
    struct thread_ssh_data *client_thread = (struct thread_ssh_data *)pointer;
    string address = string(client_thread->client_address);
    cout << address << endl;
    string name = "";
    string password = "";
    ssh_message recieved;
    bool continue_bool = true;
    int attempts = 0;
    while(continue_bool)
    {
        recieved = ssh_message_get(client_thread->client_number);
        if (recieved == NULL)
        {
            continue_bool = false;
            cout << "Closed connection by user." << endl;
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
                    cout << "Logging error!" << endl;
                pthread_mutex_unlock(&ssh_mutex_logfile);
                if (attempts == max_attempts_number)
                {
                    continue_bool = false;
                    cout << "Connection closed." << endl;
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
            cout << "Logging error!" << endl;
    }
    pthread_mutex_unlock(&ssh_mutex_logfile);
    ssh_disconnect(client_thread->client_number);
    //free(pointer);
    pthread_exit(NULL);
}

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
        Print_Error("Creating new session error!");
    ssh_bind new_bind = ssh_bind_new();
    if (new_bind == NULL)
        Print_Error("Bind error!");

    // Bind to address, port and rsakey
    ssh_bind_options_set(new_bind, SSH_BIND_OPTIONS_BINDADDR, address.c_str());
    ssh_bind_options_set(new_bind, SSH_BIND_OPTIONS_BINDPORT, &port);
    ssh_bind_options_set(new_bind, SSH_BIND_OPTIONS_RSAKEY, rsakey.c_str());

    // Initialization of threads
    ssh_threads_set_callbacks(ssh_threads_get_pthread());

    if (ssh_init()<0)
        Print_Error("Initialization of cryptographic data structures!");

    if (ssh_bind_listen(new_bind) < 0 )
        Print_Error(ssh_get_error(new_bind));

    // Until SIGINT(or SIGQUIT) do
    do
    {
        // Variables for find out clients ip addresss
        struct sockaddr_storage ssh_client_address;
        struct thread_ssh_data ssh_data;
        sa_len = sizeof(ssh_client_address);
        if (ssh_bind_accept(new_bind, new_session) < 0)
            exit (RESULT_OK);
        ssh_data.client_number = new_session;
        int client = ssh_get_fd(new_session);
        getpeername(client, (struct sockaddr *)&ssh_client_address, &sa_len);
        if (ssh_client_address.ss_family == AF_INET)
        {
            struct sockaddr_in * c_address = (struct sockaddr_in *)&ssh_client_address;
            inet_ntop(AF_INET, &c_address->sin_addr, ssh_data.client_address, INET6_ADDRSTRLEN);
        }
        else
        {
            struct sockaddr_in6 * c_address = (struct sockaddr_in6 *)&ssh_client_address;
            inet_ntop(AF_INET6, &c_address->sin6_addr, ssh_data.client_address, INET6_ADDRSTRLEN);
        }
        if (ssh_handle_key_exchange(new_session) < 0)
            Print_Error(ssh_get_error(new_session));

        if (ssh_signal_detected)
            break;

        pthread_t new_thread;
        if (pthread_create(&new_thread, NULL, &Authentication, (void *)&ssh_data) != 0)
            Print_Error("New thread creation error!");
    }while (!ssh_signal_detected);

    // Cleanup
    ssh_bind_free(new_bind);
    ssh_free(new_session);
    ssh_finalize();
    pthread_exit(NULL);
}