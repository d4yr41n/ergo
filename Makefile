CC ?= cc
PREFIX ?= /usr/local
BIN ?= ergo

CFLAGS += $(shell pkg-config --cflags wayland-client cairo pangocairo) -Wall -Wextra -g -Wno-unused-parameter
LDLIBS += $(shell pkg-config --libs wayland-client cairo pangocairo) -lrt
WAYLAND_PROTOCOLS = $(shell pkg-config --variable=pkgdatadir wayland-protocols)

all:
	wayland-scanner client-header $(WAYLAND_PROTOCOLS)/stable/xdg-shell/xdg-shell.xml xdg-shell-client-protocol.h
	wayland-scanner private-code $(WAYLAND_PROTOCOLS)/stable/xdg-shell/xdg-shell.xml xdg-shell-protocol.c
	wayland-scanner client-header protocols/wlr-layer-shell-unstable-v1.xml wlr-layer-shell-unstable-v1-client-protocol.h
	wayland-scanner private-code protocols/wlr-layer-shell-unstable-v1.xml wlr-layer-shell-unstable-v1-protocol.c
	$(CC) -o $(BIN) main.c xdg-shell-protocol.c wlr-layer-shell-unstable-v1-protocol.c $(CFLAGS) $(LDLIBS)

clean:
	rm -f $(BIN) xdg-shell-client-protocol.h xdg-shell-protocol.c \
		wlr-layer-shell-unstable-v1-client-protocol.h \
		wlr-layer-shell-unstable-v1-protocol.c

install: all
	mkdir -p $(PREFIX)/bin
	cp -f $(BIN) $(PREFIX)/bin

uninstall:
	rm -f $(PREFIX)/bin/$(BIN)

.PHONY: all clean install uninstall
