<h1 align="center">virtuawin-bar</h1>

`virtuawin-bar` is a [VirtuaWin](https://virtuawin.sourceforge.io/) module which
lists all of the non-empty virtual desktops, and highlights the currently active
one.  It is similar to status bars designed to work with tiling window managers,
like `i3bar` and `polybar`, but for Windows.

This project is unaffiliated with the official VirtuaWin project.


Installing
----------

### Binary releases

TODO: Will be available from the [Releases
page](https://github.com/enjmiah/virtuawin-bar/releases).

Simply drop `virtuawin-bar.exe` into the `modules` directory of your VirtuaWin
installation to install it.  See the [official documentation on
modules](https://virtuawin.sourceforge.io/?page_id=50) for details.


### Building from source

`virtuawin-bar` requires a C++14 compiler, CMake, and
[Cairo](https://www.cairographics.org/).  The easiest way to obtain Cairo and
all its dependencies on Windows is via
[vcpkg](https://github.com/microsoft/vcpkg): `vcpkg install cairo`.

Create a build folder, and run CMake from there:

    mkdir build && cd build
    cmake ..

Finally, to build the code:

    cmake --build . --config Release

This will produce `virtuawin-bar.exe`, a VirtuaWin module.  Simply drop the
executable plus all of its required DLLs into the `modules` directory of your
VirtuaWin installation to install it.  See the [official documentation on
modules](https://virtuawin.sourceforge.io/?page_id=50) for details.


Configuration
-------------

TODO:
