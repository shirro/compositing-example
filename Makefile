CFLAGS:=-I/opt/vc/include/
LDFLAGS:=-L/opt/vc/lib/ -lGLESv2 -lEGL -lopenmaxil -lbcm_host -lvcos -lvchiq_arm

all: compositing compositing2

%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $^

compositing: src/main.o src/display_surface.o src/shared_pixmap_surface.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

compositing2:  src/pixbuf_main.o src/display_surface.o src/shared_pixmap_surface.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f src/*.o compositing compositing2

.PHONY: all clean

