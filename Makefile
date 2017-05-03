all: clean
	gtags
	#gcc -g login_shell.c -o login_shell
	#gcc -g -pthread server.c -o server
	g++ -g login_shell_cpp.cpp -o login_shell
	g++ -g -pthread server_cpp.cpp -o server


server:
	gcc -g -pthread server.c -o server

run_login:
	./login_shell localhost 8080

run_server:
	./server 8080

clean:
	rm -rf login_shell login_shell.dSYM server server.dSYM GPATH GRTAGS GTAGS 
