# ergo

Status bar for Wayland compositors

## Dependencies

- wayland
- cairo
- pango

## Usage

```
while true; do echo ' $(date +%R) '; sleep 5; done | ergo -F 'Fantasque Sans Mono 19' -b '3f3f3f'"
```

![example](example.png)

## Thanks

- [Wayland Book](https://wayland-book.com) - code base (7.3)
- [wmenu](https://sr.ht/~adnano/wmenu) - code examples (layer-shell, cairo, etc.)

