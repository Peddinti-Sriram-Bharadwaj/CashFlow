all: signup.out loginpage.out login.out customer.out logout.out getBalance.out loanApplication.out

signup.out: signup.c
	gcc signup.c -o signup.out

loginpage.out: loginpage.c
	gcc loginpage.c -o loginpage.out

login.out: login.c
	gcc -o login.out login.c -I/opt/homebrew/Cellar/libsodium/1.0.20/include -L/opt/homebrew/Cellar/libsodium/1.0.20/lib -lsodium


customer.out: customer.c
	gcc customer.c -o customer.out -I/opt/homebrew/Cellar/libsodium/1.0.20/include -L/opt/homebrew/Cellar/libsodium/1.0.20/lib -lsodium

logout.out: logout.c
	gcc logout.c -o logout.out -I/opt/homebrew/Cellar/libsodium/1.0.20/include -L/opt/homebrew/Cellar/libsodium/1.0.20/lib -lsodium

getBalance.out: getBalance.c
	gcc getBalance.c -o getBalance.out 

loanApplication.out: loanApplication.c
	gcc loanApplication.c -o loanApplication.out 