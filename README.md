<h1 align="center">virtuawin-bar</h1>

**`virtuawin-bar`** is a [VirtuaWin](https://virtuawin.sourceforge.io/) module
which lists all of the non-empty virtual desktops, and highlights the currently
active one.  It is aesthetically similar to status bars designed to work with
tiling window managers, like i3bar and Polybar.

`virtuawin-bar` is light on system resources, and uses less than 1 MB of memory
and negligible CPU time when idle.

![](docs/bar_taskmgr.png)

This project is unaffiliated with the official VirtuaWin project.


Installing
----------

### Binary releases

TODO: Will be available from the [Releases
page](https://github.com/enjmiah/virtuawin-bar/releases).

Simply drop `virtuawin-bar.exe` into the `modules` directory of your VirtuaWin
installation to install it.  See the [official documentation on
modules][vwmodules] for details.


### Building from source

`virtuawin-bar` requires a C++14 compiler, CMake, and [Cairo][cairo].  For
convenience, (a subset of) Cairo is included in this repository under
`3rdparty`, and will be used if it is not detected on your system.

Create a build folder, and run CMake from there:

    mkdir build && cd build
    cmake ..

Finally, to build the code:

    cmake --build . --config Release

This will produce `virtuawin-bar.exe`, a VirtuaWin module.  Copy the executable
plus all of its required DLLs into the `modules` directory of your VirtuaWin
installation to install it.  See the [official documentation on
modules][vwmodules] for details.

[cairo]: https://www.cairographics.org/
[vwmodules]: https://virtuawin.sourceforge.io/?page_id=50


Configuration
-------------

TODO:


Limitations
-----------

`virtuawin-bar` only refreshes when you switch to a new desktop — to save
resources, it does *not* poll for changes.  VirtuaWin (currently) does not
support setting a handler in response to when you move a window to an empty
desktop, thus when doing so, the bar display will be out of date until the next
desktop switch.
