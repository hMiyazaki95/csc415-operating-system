# File: Standard Makefile for CSC415
#
# Description - This make file should be used for all your projects
# It should be modified as needed for each homework
#
# ROOTNAME should be set you your lastname_firstname_HW.  Except for
# and group projects, this will not change throughout the semester
#
# HW should be set to the assignment number (i.e. 1, 2, 3, etc.)
#
# FOPTION can be set to blank (nothing) or to any thing starting with an
# underscore (_).  This is the suffix of your file name.
#
# With these three options above set your filename for your homework
# assignment will look like:  bierman_robert_HW1_main.c
#
# RUNOPTIONS can be set to default values you want passed into the program
# this can also be overridden on the command line
#
# OBJ - You can append to this line for additional files necessary for
# your program, but only when you have multiple files.  Follow the convention
# but hard code the suffix as needed.
#
# To Use the Makefile - Edit as above
# then from the command line run:  make
# That command will build your program, and the program will be named the same
# as your main c file without an extension.
#
# You can then execute from the command line: make run
# This will actually run your program
#
# Using the command: make clean
# will delete the executable and any object files in your directory.
#

FIRSTNAME=hajime
LASTNAME=miyazaki
HW=4

ROOTNAME=$(LASTNAME)_$(FIRSTNAME)_HW

FOPTION=_main
RUNOPTIONS=WarAndPeace.txt 2
CC=gcc
CFLAGS= -g -I.
LIBS =-l pthread
DEPS =
OBJ = $(ROOTNAME)$(HW)$(FOPTION).o
OBJ_DEBUG = $(ROOTNAME)$(HW)$(FOPTION)-debug.o
OBJ_PROFILE = $(ROOTNAME)$(HW)$(FOPTION)-profile.o
OBJ_TIMER = $(ROOTNAME)$(HW)$(FOPTION)-timer.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%-debug.o: %.c $(DEPS)
	$(CC) -c -g -DDEBUG=1 -o $@ $< $(CFLAGS)

%-profile.o: %.c $(DEPS)
	$(CC) -c -g -DPROFILE=1 -pg -o $@ $< $(CFLAGS)

%-timer.o: %.c $(DEPS)
	$(CC) -c -g -DTIMER=1 -o $@ $< $(CFLAGS)

help:
	@echo "Makefile targets:"
	@grep "^#HELP" Makefile | sed 's/^#HELP//'

#HELP	assignment	Run everything we need to complete the assignment
assignment: \
  run-1.log run-2.log run-4.log run-8.log run-16.log \
  vrun.log \
  verify-1.log \
  error-test
	@echo "************** Only difference should the the run times"
	-diff run-1.log run-2.log
	@echo "************** Only difference should the the run times"
	-diff run-1.log run-4.log
	@echo "************** Only difference should the the run times"
	-diff run-1.log run-8.log
	@echo "************** Only difference should the the run times"
	-diff run-1.log run-16.log

$(ROOTNAME)$(HW)$(FOPTION): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

$(ROOTNAME)$(HW)$(FOPTION)-debug: $(OBJ_DEBUG)
	$(CC) -g -DDEBUG=1 -o $@ $^ $(CFLAGS) $(LIBS)

$(ROOTNAME)$(HW)$(FOPTION)-profile: $(OBJ_PROFILE)
	$(CC) -g -pg -DPROFILE=1 -o $@ $^ $(CFLAGS) $(LIBS)

$(ROOTNAME)$(HW)$(FOPTION)-timer: $(OBJ_TIMER)
	$(CC) -g -pg -DTIMER=1 -o $@ $^ $(CFLAGS) $(LIBS)

#HELP	clean		Remove generated files
clean:
	-rm *.o $(ROOTNAME)$(HW)$(FOPTION)
	-rm $(ROOTNAME)$(HW)$(FOPTION)-debug
	-rm $(ROOTNAME)$(HW)$(FOPTION)-timer
	-rm $(ROOTNAME)$(HW)$(FOPTION)-profile
	-rm test-3.txt
	-rm test-4.txt
	-rm *.log
	-rm gmon.out
	-rm bad
	-rm getwords
	-rm bad.txt

#HELP	run		Build and run the program
run: $(ROOTNAME)$(HW)$(FOPTION)
	./$(ROOTNAME)$(HW)$(FOPTION) $(RUNOPTIONS)
#HELP	run-#		Build and run the program with # threads
run-all: run-1.log run-2.log run-4.log run-8.log run-16.log
run-%.log: $(ROOTNAME)$(HW)$(FOPTION)
	./$(ROOTNAME)$(HW)$(FOPTION) WarAndPeace.txt $* > $@
# Need a dependency to make sure getwords is compiled
getwords: getwords.o
# install csh if your computer doesn't have it
/bin/csh:
	sudo apt install csh
# install gawk if your computer doesn't have it
/usr/bin/gawk:
	sudo apt install gawk
# Make sure checkLog is an executable
checkLog: /bin/csh /usr/bin/gawk
	chmod 755 ./checkLog
# generate counts of every work in the run log from WarAndPeace.txt
check-%.log: run-%.log getwords
	./checkLog < $< > $@
# The only difference between the files should only be the run time
verify-%.log: check-%.log run-%.log
	-diff check-$*.log run-$*.log

#HELP	debug-#		Build debuggable version, run WarAndPeace.txt with # threads
debug-%: $(ROOTNAME)$(HW)$(FOPTION)-debug
	./$(ROOTNAME)$(HW)$(FOPTION)-debug WarAndPeace.txt $*

#HELP				SMALLER TEST CASES:
#HELP	debug-1-#		Debug with test-1.txt using # threads
debug-1-%: $(ROOTNAME)$(HW)$(FOPTION)-debug
	./$(ROOTNAME)$(HW)$(FOPTION)-debug test-1.txt $*

#HELP	debug-2-#		test-2.txt using # threads
debug-2-%: $(ROOTNAME)$(HW)$(FOPTION)-debug
	./$(ROOTNAME)$(HW)$(FOPTION)-debug test-2.txt $*
debug-2: \
  debug-2-1 \
  debug-2-2 \
  debug-2-3 \
  debug-2-4 \
  debug-2-5 \

#HELP	debug-3-#		test-3.txt using # threads
debug-3-%: $(ROOTNAME)$(HW)$(FOPTION)-debug
	./$(ROOTNAME)$(HW)$(FOPTION)-debug test-3.txt $*
debug-3: \
  debug-3-1 \
  debug-3-3 \
  debug-3-3 \
  debug-3-4 \
  debug-3-5 \

#HELP	debug-4-#		debug test-4.txt using # threads
debug-4-%: $(ROOTNAME)$(HW)$(FOPTION)-debug
	./$(ROOTNAME)$(HW)$(FOPTION)-debug test-4.txt $*
debug-4: \
  debug-4-1 \
  debug-4-4 \
  debug-4-4 \
  debug-4-4 \
  debug-4-5 \

#HELP	profile-2-1		profile test-2.txt using 1 thread
profile-2-1: $(ROOTNAME)$(HW)$(FOPTION)-profile
	./$(ROOTNAME)$(HW)$(FOPTION)-profile test-2.txt 1
#HELP	profile-2-1-view	view profile results on test-2.txt using 1 thread
profile-2-1-view: $(ROOTNAME)$(HW)$(FOPTION)-profile gmon.out
	gprof $(ROOTNAME)$(HW)$(FOPTION)-profile gmon.out

#HELP	profile-3-#		profile test-3.txt using # threads
profile-3-%: $(ROOTNAME)$(HW)$(FOPTION)-profile test-3.txt
	./$(ROOTNAME)$(HW)$(FOPTION)-profile test-3.txt $*
#HELP	profile-3-1-view	view profile results
profile-3-1-view: $(ROOTNAME)$(HW)$(FOPTION)-profile gmon.out
	gprof $(ROOTNAME)$(HW)$(FOPTION)-profile gmon.out

#HELP	profile-4-#		profile test-4.txt using # threads
profile-4-%: $(ROOTNAME)$(HW)$(FOPTION)-profile test-4.txt
	./$(ROOTNAME)$(HW)$(FOPTION)-profile test-4.txt $*
#HELP	profile-4-1-view	view profile results
profile-4-1-view: $(ROOTNAME)$(HW)$(FOPTION)-profile gmon.out
	gprof $(ROOTNAME)$(HW)$(FOPTION)-profile gmon.out

#HELP	test-2-#		run test-1.txt using # threads
test-2-%: $(ROOTNAME)$(HW)$(FOPTION) test-2.txt
	./$(ROOTNAME)$(HW)$(FOPTION) test-2.txt $*

test-3.txt: WarAndPeace.txt
	head -100 < $< > $@

test-4.txt: WarAndPeace.txt
	head -5000 < $< > $@

bad.txt: bad
	./bad > bad.txt

#HELP	test-3-#		run test-3.txt using # threads
test-3-%: $(ROOTNAME)$(HW)$(FOPTION) test-3.txt
	./$(ROOTNAME)$(HW)$(FOPTION) test-3.txt $*

#HELP	test-4-#		run test-4.txt using # threads
test-4-%: $(ROOTNAME)$(HW)$(FOPTION) test-4.txt
	time ./$(ROOTNAME)$(HW)$(FOPTION) test-4.txt $*

#HELP	timer-#-1		test-#.txt case using 1 thread
timer-%-1: $(ROOTNAME)$(HW)$(FOPTION)-timer test-%.txt
	./$(ROOTNAME)$(HW)$(FOPTION)-timer test-$*.txt 1

#HELP	gdb-4-#			debug using test-4.txt with # threads
gdb-4-%: $(ROOTNAME)$(HW)$(FOPTION) test-4.txt
	@echo "Type: run test-4.txt $@"
	gdb ./$(ROOTNAME)$(HW)$(FOPTION)

#HELP	vrun-#.log		Run valgrind on the program using test-#.txt
vrun-%.log: $(ROOTNAME)$(HW)$(FOPTION) test-%.txt
	valgrind ./$(ROOTNAME)$(HW)$(FOPTION) test-$*.txt 2 >$@ 2>&1

#HELP	vrun		Run valgrind on the program
vrun: $(ROOTNAME)$(HW)$(FOPTION)
	valgrind ./$(ROOTNAME)$(HW)$(FOPTION) $(RUNOPTIONS)
# Ignore valgrind return status since even "0 errors from..." returns Error 1
vrun.log: $(ROOTNAME)$(HW)$(FOPTION)
	-valgrind ./$(ROOTNAME)$(HW)$(FOPTION) $(RUNOPTONS) 2 >$@ 2>&1

#HELP	error-test	Test program for some error conditions
error-test: ./$(ROOTNAME)$(HW)$(FOPTION)
	@echo "************** Expect error when checking error conditions"
	-./$(ROOTNAME)$(HW)$(FOPTION)
	-./$(ROOTNAME)$(HW)$(FOPTION) notEnoughArguments
	-./$(ROOTNAME)$(HW)$(FOPTION) fileDoesNotExist 1
	-./$(ROOTNAME)$(HW)$(FOPTION) WarAndPeace.txt xxxxx
	-./$(ROOTNAME)$(HW)$(FOPTION) WarAndPeace.txt -2

#HELP	backup		Save a working copy
backup:
	tar cf ../jimmy.tgz .
