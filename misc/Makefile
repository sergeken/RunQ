LIBRARY = $(LIBDIR)/libmisc.a

OBJS = datastore.o util.o

INCLUDES =

CXXFLAGS = $(CFLAGS) $(INCLUDES)
CXX = g++

$(LIBRARY): $(OBJS)
	ar cru $(LIBRARY) $(OBJS)

.PHONY:	clean
clean:
	rm -f $(LIBRARY)
	rm -f $(OBJS)
