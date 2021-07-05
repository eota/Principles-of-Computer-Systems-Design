#######################################################################
#
# Copyright (C) 2020-2021 David C. Harrison. All right reserved.
#
# You may not use, distribute, publish, or modify this code without 
# the express written permission of the copyright holder.
#
#######################################################################

CFLAGS = -g -Wall 
CC = gcc

# Assignment No.
a = 4

default: scheduler

scheduler: *.c simulator.a
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) simulator.a 2>&1 | tee make.out

grade: clean scheduler 
	@./grade.sh $(a)

clean:
	@rm -f scheduler *.out

submit: clean
	@echo ""
	@echo "Only these file are submitted:"
	@echo ""
	@tar czvf ~/CSE130-Assignment$(a).tar.gz \
		*.h *.c --exclude=scheduler.h --exclude=simulator.h
	@echo ""
	@echo "To confirm your submission will work in the automated grading system:"
	@echo ""
	@echo "  \044 rm -rf tmp"
	@echo "  \044 mkdir tmp"
	@echo "  \044 cd tmp"
	@echo "  \044 tar xvf /var/classes/CSE130/Winter21/Assignment$(a).tar.gz"
	@echo "  \044 tar xvf ~/CSE130-Assignment$(a).tar.gz"
	@echo "  \044 make grade"
	@echo "  \044 cd .."
	@echo ""
	@echo "If that gets the grade your were expecting, you're all set. If it fails,"
	@echo "fix the problem and try again."
	@echo ""
	@echo "Finally, don't forget to upload ~/CSE130-Assignment$(a).tar.gz to Canvas!"
	@echo ""
