# Ergo

A minimal status bar for Wayland compositors.

## Dependencies

- wayland
- cairo
- pango

## Usage

Use stdin to pass text for display.
Use `^` to highlight text (can be escaped with a slash `\^`).

### Examples

```
while true; do echo $(date +%R); sleep 5; done | ergo -rN '3f3f3f'
```

```
echo " some ^ awesome ^ text " | ergo
```

![example](example0.png)
![example](example1.png)

## Thanks

- [Wayland Book](https://wayland-book.com) - code base (7.3)
- [wmenu](https://sr.ht/~adnano/wmenu) - code examples (layer-shell, cairo, etc.)

