all:
	gcc -g login_shell.c -o login_shell

run:
	./login_shell

clean:
	rm -rf login_shell login_shell.dSYM
