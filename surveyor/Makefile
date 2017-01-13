PROG=surveyor

OBJS = surveyor.o compiler.tab.o lex.yy.o

INCLUDES = -I../misc -I../datafile -I../analyzer

CXXFLAGS = $(CFLAGS) $(INCLUDES)
CXX = g++

$(PROG):	$(OBJS) $(LIBS)
	$(CXX) $(CXXFLAGS) -o $(PROG) $(OBJS) $(LIBS)

lex.yy.o:lex.yy.c compiler.tab.h mytype.h
	$(CXX) -c $(CXXFLAGS) lex.yy.c

y.tab.o: y.tab.c mytype.h
	$(CXX) -c $(CXXFLAGS) y.tab.c

compiler.tab.o: compiler.tab.c mytype.h
	$(CXX) -c $(CXXFLAGS) compiler.tab.c

compiler.tab.c: compiler.y
	bison -d $<

lex.yy.c: compiler.l
	flex $<

.PHONY:	clean
clean:
	rm -f $(LIBRARY)
	rm -f $(OBJS)
	rm -f lex.yy.c compiler.tab.[ch]
