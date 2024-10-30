all: login.out manager.out logout.out viewLoans.out viewEmployees.out assignLoan.out changePassword.out getFeedback.out

login.out: login.c
	gcc -o login.out login.c -I/opt/homebrew/Cellar/libsodium/1.0.20/include -L/opt/homebrew/Cellar/libsodium/1.0.20/lib -lsodium


manager.out: manager.c
	gcc manager.c -o manager.out -I/opt/homebrew/Cellar/libsodium/1.0.20/include -L/opt/homebrew/Cellar/libsodium/1.0.20/lib -lsodium

logout.out: logout.c
	gcc -o logout.out logout.c -I/opt/homebrew/Cellar/libsodium/1.0.20/include -L/opt/homebrew/Cellar/libsodium/1.0.20/lib -lsodium

viewLoans.out: viewLoans.c
	gcc -o viewLoans.out viewLoans.c 

viewEmployees.out: viewEmployees.c
	gcc -o viewEmployees.out viewEmployees.c

assignLoan.out: assignLoan.c
	gcc -o assignLoan.out assignLoan.c

changePassword.out: changePassword.c
	gcc -o changePassword.out changePassword.c -I/opt/homebrew/Cellar/libsodium/1.0.20/include -L/opt/homebrew/Cellar/libsodium/1.0.20/lib -lsodium

getFeedback.out: getFeedback.c
	gcc -o getFeedback.out getFeedback.c

clean:
	rm -f *.out