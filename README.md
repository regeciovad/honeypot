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
./fakesrv -m ftp -a fe80::21e:67ff:ab1a:b1ff -p 21 -l hello
sudo ./fakesrv -m ftp -a ::1 -p 21 -l hello

IPv6 KO
1..2
1::8:
234

# To Do List:
osetri pripadne chyby, pokud nejde otevrit soubor - return + uklid
mozna pridat print_error_no_exit

valgrind --leak-check=yes ./fakesrv -m ftp -a 10.0.2.15 -p 21 -l hello

testovat: sudo valgrind --leak-check=yes ./fakesrv -m ftp -a 10.0.2.15 -p 21 -l hello

