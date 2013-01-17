CC=gcc
MINGW32=i486-mingw32-
CFLAGS= -std=c99 -Wall -O2 -pipe -march=x86-64 -mtune=generic -lm
APP=vlsmsolver

all: unix unix-gtk win32  win32-gtk

unix: ui_cli.c vlsm.c
	$(CC) $(CFLAGS) -o $(APP) $^
	strip $(APP)

win32: ui_cli.c vlsm.c
	$(MINGW32)gcc -lm -o $(APP).exe $^
	$(MINGW32)strip $(APP).exe

unix-gtk: ui_gtk.c gtk_main_window.c vlsm.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs gtk+-2.0` -o $(APP)-gtk  $^ 
	strip $(APP)-gtk
	
win32-gtk: ui_gtk.c gtk_main_window.c vlsm.c
	$(MINGW32)gcc -lm -o $(APP)-gtk.exe  $^ `$(MINGW32)pkg-config --cflags --libs gtk+-2.0` -mwindows
	$(MINGW32)strip $(APP)-gtk.exe

clear:
	rm -f *.o

clean:
	rm -f $(APP) $(APP)-gtk *.o *.exe

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^
