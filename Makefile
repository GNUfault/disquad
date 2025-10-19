all:
	gcc main.c -o disquad -Ofast -march=native -mtune=native `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.1 libnotify`
	objcopy --strip-all disquad

clean:
	rm disquad
