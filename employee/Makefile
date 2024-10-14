all: login.out employee.out addcustomer.out


login.out: login.c
	gcc -o login.out login.c -I/opt/homebrew/Cellar/libsodium/1.0.20/include -L/opt/homebrew/Cellar/libsodium/1.0.20/lib -lsodium

employee.out: employee.c
	gcc employee.c -o employee.out
addcustomer.out: addcustomer.c
	gcc -o addcustomer.out addcustomer.c -I/opt/homebrew/Cellar/libsodium/1.0.20/include -L/opt/homebrew/Cellar/libsodium/1.0.20/lib -lsodium

