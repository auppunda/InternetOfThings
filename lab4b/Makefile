
default: 
	gcc -o lab4b -Wall -Wextra -lm -lmraa lab4b.c

check: clean default 
	chmod +x test.sh
	./test.sh
	#smoketest

clean:
	rm -f *.tar.gz lab4b *.txt
dist: 
	tar -zcvf lab4b-004782343.tar.gz lab4b.c Makefile README test.sh
