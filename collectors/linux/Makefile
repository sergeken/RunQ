LIBRARY = $(LIBDIR)/liblinux.a

OBJS = linux.o

INCLUDES = -I../../misc -I../../datafile

CXXFLAGS = $(CFLAGS) $(INCLUDES)
CXX = g++

$(LIBRARY): $(OBJS)
	ar cru $(LIBRARY) $(OBJS)

.PHONY: clean
clean:
	rm -f $(LIBRARY)
	rm -f $(OBJS)
