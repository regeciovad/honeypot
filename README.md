# Honeypot
The school project for ISA 2015.

The private repository!

./fakesrv -m ftp -a 10.10.10.10 -p 30 -l hello
Tests of IP address
IPv4 OK
10.10.10.10
127.0.0.1
IPv4 KO
10.10.10
1234

IPv6 OK
1::8
2001:0db8::ff00:42:6854

IPv6 KO
1..2
1::8:
234

# To Do List:
* Change error return na

void Print_Error(string message)
{
    cerr << message << endl;
    exit(RESULT_FAILURE);
}
Where? logging.cpp

