all:
	gcc main.c -o disquad `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.1 libnotify`

