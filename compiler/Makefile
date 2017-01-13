LIBRARY = $(LIBDIR)/libcompiler.a

OBJS = y.tab.o lex.yy.o

INCLUDES = -I../misc -I../datafile -I../analyzer

CXXFLAGS = $(CFLAGS) $(INCLUDES)
CXX = g++

$(LIBRARY): $(OBJS)
	ar cru $(LIBRARY) $(OBJS)

lex.yy.o:lex.yy.c y.tab.h mytype.h
	$(CXX) -c $(CXXFLAGS) lex.yy.c

y.tab.o: y.tab.c mytype.h
	$(CXX) -c $(CXXFLAGS) y.tab.c

y.tab.c y.tab.h: compiler.y
	yacc -d $<

lex.yy.c: compiler.l
	lex $<

.PHONY:	clean
clean:
	rm -f $(LIBRARY)
	rm -f $(OBJS)
	rm -f lex.yy.[ch] y.tab.[ch]
