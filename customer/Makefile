all: signup.out loginpage.out login.out customer.out

signup.out: signup.c
	gcc signup.c -o signup.out

loginpage.out: loginpage.c
	gcc loginpage.c -o loginpage.out

login.out: login.c
	gcc -o login.out login.c -I/opt/homebrew/Cellar/libsodium/1.0.20/include -L/opt/homebrew/Cellar/libsodium/1.0.20/lib -lsodium


customer.out: customer.c
	gcc customer.c -o customer.out
