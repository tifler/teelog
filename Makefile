
all:
	gcc -o teelog teelog.c -lpthread
	aarch64-linux-gnu-gcc -o teelog-arm64 teelog.c -lpthread

archive:
	git archive --format tgz --prefix=teelog/ -o teelog.tgz master
