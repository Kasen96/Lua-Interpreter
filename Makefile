LEX = flex
YACC = bison
CC = g++
OBJECT = int

$(OBJECT): lex.yy.c task.tab.o main.cpp node.cpp
	$(CC) task.tab.o lex.yy.c main.cpp node.cpp -o $(OBJECT)

task.tab.o: task.tab.cc
	$(CC) -c task.tab.cc

task.tab.cc: task.yy
	$(YACC) -d task.yy

lex.yy.c: task.ll task.tab.cc
	$(LEX) task.ll

clean:
	@rm task.tab.* lex.yy.c* stack.hh $(OBJECT)