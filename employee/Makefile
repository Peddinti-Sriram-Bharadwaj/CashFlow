all: login.out employee.out addcustomer.out logout.out viewHistory.out  updateUsername.out viewLoans.out loan.out


login.out: login.c
	gcc -o login.out login.c -I/opt/homebrew/Cellar/libsodium/1.0.20/include -L/opt/homebrew/Cellar/libsodium/1.0.20/lib -lsodium

employee.out: employee.c
	gcc employee.c -o employee.out 
addcustomer.out: addcustomer.c
	gcc -o addcustomer.out addcustomer.c -I/opt/homebrew/Cellar/libsodium/1.0.20/include -L/opt/homebrew/Cellar/libsodium/1.0.20/lib -lsodium

logout.out: logout.c
	gcc -o logout.out logout.c -I/opt/homebrew/Cellar/libsodium/1.0.20/include -L/opt/homebrew/Cellar/libsodium/1.0.20/lib -libsodium

viewHistory.out: viewHistory.c
	gcc -o viewHistory.out viewHistory.c 

updateUsername.out: updateUsername.c
	gcc -o updateUsername.out updateUsername.c -I/opt/homebrew/Cellar/libsodium/1.0.20/include -L/opt/homebrew/Cellar/libsodium/1.0.20/lib -lsodium

viewLoans.out: viewLoans.c
	gcc -o viewLoans.out viewLoans.c 

loan.out : loan.c
	gcc -o loan.out loan.c -I/opt/homebrew/Cellar/libsodium/1.0.20/include -L/opt/homebrew/Cellar/libsodium/1.0.20/lib -lsodium

