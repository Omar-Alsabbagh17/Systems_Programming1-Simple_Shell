sshell: obj.o vec_str.o
	gcc -o sshell obj.o vec_str.o  -w
obj.o: sshell.c
	gcc -c -o obj.o sshell.c -w
vec_str.o: string_vector.c string_vector.h
	gcc -c -o vec_str.o string_vector.c
clean:
	rm -f obj.o vec_str.o


# will change to -Wall -Wextra -Werror when submitting to grade scope
