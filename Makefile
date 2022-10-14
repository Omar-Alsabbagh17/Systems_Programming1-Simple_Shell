sshell: obj.o vec_str.o
	gcc -o sshell obj.o vec_str.o  -Wall -Wextra -Werror
obj.o: sshell.c
	gcc -c -o obj.o sshell.c -Wall -Wextra -Werror
vec_str.o: string_vector.c string_vector.h
	gcc -c -o vec_str.o string_vector.c -Wall -Wextra -Werror
clean:
	rm -f obj.o vec_str.o

