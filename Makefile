welcome.o: welcome.c
	gcc welcome.c -o welcome.out
./customer/login.o: ./customer/login.c
	gcc ./customer/login.c -o ./customer/login.out
