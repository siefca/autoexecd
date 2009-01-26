# autoexecd's Makefile

CC = gcc
CCOPTS = -O3
AUTOEXECD = autoexecd
NEEDED = version.c version.h ad_messages.h
ETCPATH = /etc
RCPATH = $(ETCPATH)/rc.d/init.d
MANPATH = /usr/man
BINPATH = /usr/sbin
TEMP = /tmp
CONFIGFILE = autoexecd.conf
VERSION = 1.0

all: $(AUTOEXECD)

$(AUTOEXECD): $(AUTOEXECD).c $(NEEDED)
	@echo "compiling $(AUTOEXECD).c"
	$(CC) $(CCOPTS) $(AUTOEXECD).c -o $(AUTOEXECD)
	@echo "fixing permisions..."
	@chmod --changes o-x $(AUTOEXECD)
	@echo "now type 'make install' to install the daemon..."

clean:
	@rm -v -f $(AUTOEXECD) core

install: all rinstall clean
	@echo "done."

rinstall:
	@echo "-> installing $(AUTOEXECD)"
	@echo "copying files..."
	@cp -v -f $(AUTOEXECD) $(BINPATH)
	@cp -v -f etc/$(CONFIGFILE) $(ETCPATH)
	@cp -v -f etc/rc.d/init.d/$(AUTOEXECD) $(RCPATH)
	@cp -v -f -R usr/man/* $(MANPATH)
	@echo "fixing permisions..."
	@chmod --changes u=rwx,g+rx $(BINPATH)/$(AUTOEXECD)
	@chmod --changes u=rw $(ETCPATH)/$(CONFIGFILE)
	@chmod --changes u=rwx,go+rx $(RCPATH)/$(AUTOEXECD)
	@echo "adding to chkconfig..."
	@chkconfig --add $(AUTOEXECD) 

source: targz

targz:
	@echo "compressing source..."
	@{ cd .. ; rm -f $(AUTOEXECD)-$(VERSION).tar.gz ; tar -cvzf $(AUTOEXECD)-$(VERSION).tar.gz --exclude $(AUTOEXECD)-$(VERSION)/$(AUTOEXECD) $(AUTOEXECD)-$(VERSION) }
	@echo "done, $(AUTOEXECD)-$(VERSION).tar.gz is placed in `cd .. ; pwd` "

help:
	@echo "just type: make ..."
	@echo "install			-- to install $(AUTOEXECD)"
	@echo "all OR $(AUTOEXECD)	-- to compile $(AUTOEXECD)"
	@echo "clean			-- to remove core"
	@echo "targz			-- to compress the $(AUTOEXECD)'s sources"
	@echo "help			-- to see this."
	@echo
