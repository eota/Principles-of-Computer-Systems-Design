#######################################################################
#
# Copyright (C) 2018 David C. Harrison. All right reserved.
#
# You may not use, distribute, publish, or modify this code without 
# the express written permission of the copyright holder.
#
#######################################################################

CFLAGS = -Wall
CC = gcc -g

all: psort usort

psort: psort.c merge.a
	$(CC) -o $@ $^ $(CFLAGS) -lrt merge.a 2>&1 | tee pmake.out

usort: usort.c merge.a
	$(CC) -o $@ $^ $(CFLAGS) merge.a 2>&1 | tee umake.out

grade: clean psort usort
	@./grade.sh 

clean:
	@rm -f psort usort *make.out

submit: clean
	@tar czvf ~/CSE130-Assignment1.tar.gz \
		psort.c usort.c
