all:
	gcc -g login_shell.c -o login_shell
	gcc -g -pthread server.c -o server

run_login:
	./login_shell

clean:
	rm -rf login_shell login_shell.dSYM server
