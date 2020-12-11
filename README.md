<h1 align="center">virtuawin-tiling</h1>

**`virtuawin-tiling`** is a [VirtuaWin](https://virtuawin.sourceforge.io/)
module which adds support for tiling windows.

`virtuawin-tiling` also comes with a minimalistic bar which lists all of the
non-empty virtual desktops, and highlights the currently active one.

This project is unaffiliated with the official VirtuaWin project.


Installing
----------

### Binary releases

TODO: Will be available from the [Releases
page](https://github.com/enjmiah/virtuawin-tiling/releases).

Simply drop `virtuawin-tiling.exe` into the `modules` directory of your
VirtuaWin installation to install it.  See the [official documentation on
modules](https://virtuawin.sourceforge.io/?page_id=50) for details.


### Building from source

`virtuawin-tiling` requires a C++14 compiler, CMake, and
[Cairo](https://www.cairographics.org/).  The easiest way to obtain Cairo and
all its dependencies on Windows is via
[vcpkg](https://github.com/microsoft/vcpkg): `vcpkg install cairo`.

Create a build folder, and run CMake from there:

    mkdir build && cd build
    cmake ..

Finally, to build the code:

    cmake --build . --config Release

This will produce `virtuawin-tiling.exe`, a VirtuaWin module.  Simply drop the
executable plus all of its required DLLs into the `modules` directory of your
VirtuaWin installation to install it.  See the [official documentation on
modules](https://virtuawin.sourceforge.io/?page_id=50) for details.


Configuration
-------------

TODO:
