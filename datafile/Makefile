LIBRARY = $(LIBDIR)/libdatafile.a

OBJS = perfdata.o

INCLUDES = -I../misc

CXXFLAGS = $(CFLAGS) $(INCLUDES)
CXX = g++

$(LIBRARY): $(OBJS)
	ar cru $(LIBRARY) $(OBJS)

.PHONY:	clean
clean:
	rm -f $(LIBRARY)
	rm -f $(OBJS)
