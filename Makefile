sshell: obj.o
	gcc -o sshell obj.o  -Wall -Wextra -Werror
obj.o: sshell.c
	gcc -c -o obj.o sshell.c -Wall -Wextra -Werror
clean:
	rm -f obj.o
