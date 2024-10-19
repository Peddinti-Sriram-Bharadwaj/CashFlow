all: signup.out loginpage.out login.out customer.out logout.out getBalance.out loanApplication.out depositMoney.out transferMoney.out withdrawMoney.out viewHistory.out

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

depositMoney.out: depositMoney.c
	gcc depositMoney.c -o depositMoney.out

transferMoney.out: transferMoney.c
	gcc transferMoney.c -o transferMoney.out

withdrawMoney.out: withdrawMoney.c
	gcc withdrawMoney.c -o withdrawMoney.out

viewHistory.out: viewHistory.c
	gcc viewHistory.c -o viewHistory.out

updateUsername.out: updateUsername.c
	gcc updateUsername.c -o updateUsername.out

clean:
	rm -f *.out
