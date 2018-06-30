default: 
	gcc -Wall -Wextra -lmraa -lm -g -o lab4c_tcp lab4c_tcp.c 
	gcc -Wall -Wextra -lmraa -L/usr/local/lib -lm -g -lssl -lcrypto -o lab4c_tls lab4c_tls.c


clean:
	rm -f lab4c-004782343.tar.gz lab4c_tcp lab4c_tls

dist:
	tar -czvf lab4c-004782343.tar.gz Makefile README lab4c_tcp.c lab4c_tls.c

