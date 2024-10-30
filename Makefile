welcome.out: welcome.c
	gcc welcome.c -o welcome.out -lpthread

clean:
	rm -f welcome.out
