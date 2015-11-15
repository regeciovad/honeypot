/*
    ISA course project, 2015/2016
    Variant: FTP/SSH Honeypot
    Author: Dominika Regeciova, xregec00
*/

#include "logging.hpp"

using namespace std;
const int BUFFERSIZE = 255;

// Function for printing error message in a whole project
void Print_Error(string message)
{
    cerr << message << endl;
    exit(RESULT_FAILURE);
}

// Function returning actual date and time in format: YYYY-MM-DD HH:MM:SS
string Get_Time()
{
    // Get current time and convert it to tm
    time_t t = time(0);
    tm now = *localtime(&t);
    char current_time[BUFFERSIZE] = {0};
    const char format_time[] = "%Y-%m-%d %X";
    if (strftime(current_time, sizeof(current_time)-1, format_time, &now) > 0)
    {
        //string string_current_time(current_time);
        return string(current_time);
    }
    else
        return "Strftime failed.";
}

void write_log(string logfile, string mode, string address, string name, string password)
{
    FILE *new_file;
    new_file = fopen(logfile.c_str(), "a");
    if (new_file != NULL)
    {
        string timed = Get_Time();
        stringstream ssout;
        ssout << mode << " " << timed + " " + address + " " + name + " " + password + "\n";
        string out = ssout.str();

        fprintf(new_file, "%s", out.c_str());
    }
}