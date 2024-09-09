# Ergo

A minimal status bar for Wayland compositors

## Dependencies

- wayland
- cairo
- pango

## Usage

Ergo uses stdin to get data for display

```
while true; do echo ' $(date +%R) '; sleep 5; done | ergo -F 'Fantasque Sans Mono 18' -b '3f3f3f'"
```

![example](example.png)

## Thanks

- [Wayland Book](https://wayland-book.com) - code base (7.3)
- [wmenu](https://sr.ht/~adnano/wmenu) - code examples (layer-shell, cairo, etc.)

