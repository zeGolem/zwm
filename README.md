# zwm - zegolem's window manager

This is a window manager written in c++ that I made… It's mostly something I made for myself, but feel free to use it as a base for your own, or as it is.   
This was based off of [Nick Welch's TinyWM](http://incise.org/tinywm.html)

Feel free to make a pull request if you want to fix some bug, make some changes, or even add a brand new feature! As long as you don't break what I put in place, I don't mind

## How to use

### Dependencies

For now I'm only using Xlib in the code, and Xephyr as an emulator to test. You will also need make and g++ to build it.

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

If you want to install this despite all those warnings, you'll have to figure it out yourself, I do not recommend doing this, and emulator is enough for developement purposes

### Keyboard shortcuts

* **Alt+Mouse1** moves the window you're hovering over
* **Alt+Mouse3** resizes the window you're hovering over
* **Alt+F4** closes the focused window
* Clicking on a window raises it to the top


## TODO

### Bugs

* You should be able to click anywhere on a window to raise it to the from
* Focus shouldn't move with the mouse, the last clicked window should be focused
* Multiple physical displays are not supported

### Features to add

* Buttons to close, maximuze/restore, and inconify a window
* Some sort of configuration? I don't think I'd need that many things configurable, so maybe using environement variables could be a good way to go?

### Code improvements

* There are loads of things that are computed multiple times that could be only computed once, such as fonts…
