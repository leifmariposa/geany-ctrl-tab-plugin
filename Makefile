SHELL   = /bin/sh

TARGET  = ctrl_tab
SOURCES = $(shell echo *.c)
OBJECTS = $(SOURCES:.c=.o)
LIBRARY = $(OBJECTS:.o=.so)
CFLAGS  += $(shell pkg-config --cflags --libs  geany)
CFLAGS  += -W
CFLAGS  += -Wall
CFLAGS  += -O2
PREFIX  = $(DESTDIR)/usr/local
BINDIR  = $(PREFIX)/lib/geany

$(LIBRARY): $(SOURCES)
	$(CC) -c $(SOURCES) $(CFLAGS) -fPIC
	$(CC) -shared -o $(LIBRARY) $(OBJECTS);

install:
	install -D $(LIBRARY) $(BINDIR)

uninstall:
	@rm -f $(BINDIR)/$(LIBRARY)

.PHONY : clean
clean :
	@rm -f $(OBJECTS)
	@rm -f $(LIBRARY)
