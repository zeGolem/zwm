# zwm - zegolem's window manager

This is a window manager written in c++ that I made… It's mostly something I made for myself, but feel free to use it as a base for your own, or as it is.  
This was based off of [Nick Welch's TinyWM](http://incise.org/tinywm.html)

Feel free to make a pull request if you want to fix some bug, make some changes, or even add a brand new feature! As long as you don't break what I put in place, I don't mind

## How to use

### Dependencies

For now I'm only using XCB and XCB Cursors in the code, and Xephyr as an emulator to test. You will also need make and g++ to build it.

### Get running

1. Build

```
make build
```

2. Test

```
make run-xephyr
```

3. Install

**This is not produciton ready, there are still loads of crashes, and bugs that makes this unusable on a main system**

If you want to install this despite all those warnings, you'll have to figure it out yourself, I do not recommend doing this, and emulator is enough for development purposes

### Keyboard shortcuts

*currently not working due to recent refactoring*

- **Alt+Mouse1** moves the window you're hovering over
- **Alt+Mouse3** resizes the window you're hovering over
- **Alt+F4** closes the focused window
- Clicking on a window raises it to the top

## TODO

### Features to add

- Buttons to close, maximize/restore, and iconify a window
- Some sort of configuration? I don't think I'd need that many things configurable, so maybe using environment variables could be a good way to go?
- Add back all the features in Xlib that no longer work since switching to XCB.

### Currently working on

- Getting the frame drawing working
- Making events work
