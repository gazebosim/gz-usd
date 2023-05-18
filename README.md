Gazebo version | Branch
-- | --
Fortress | [fortress](https://github.com/gazebosim/gz-usd/tree/fortress) |
Garden and higher versions | [main](https://github.com/gazebosim/gz-usd) |

USD is a high-performance extensible software platform for collaboratively constructing animated 3D
scenes, designed to meet the needs of large-scale film and visual effects production.

This repo provides tools to convert between SDF and USD files.

**USD requires CMAKE 3.12; this package is available from Ubuntu 20.04**

# Tutorials

If you have already installed `gz-usd` you might want to visit [the tutorial section](./tutorials/README.md).

# Requirements

You will need all of the dependencies for sdformat, along with the following additional dependencies:
* [USD](https://github.com/PixarAnimationStudios/USD/tree/v21.11#getting-and-building-the-code):
    Note: USD support is only available when building sdformat from source. USD requires CMAKE 3.12 this package is available from Ubuntu 20.0.

    Clone the USD repository
    ```bash
    git clone --depth 1 -b v21.11 https://github.com/PixarAnimationStudios/USD.git
    ```
    Note: Only v21.11 supported currently

    Install dependencies not managed by the build script

    ```bash
    sudo apt install libpyside2-dev python3-opengl cmake libglu1-mesa-dev freeglut3-dev mesa-common-dev
    ```
    Use the build script to compile USD. In order to speed up compilation, it is recommended to disable unneeded components.
    ```bash
    cd USD
    python3 build_scripts/build_usd.py --build-variant release --no-tests --no-examples --no-tutorials --no-docs --no-python <install_dir>
    ```
    For more information regarding the build options, see the USD docs at https://github.com/PixarAnimationStudios/USD/tree/v21.11#getting-and-building-the-code.

    Add USD to system paths (replace <install_dir> with the path to your USD install directory)

    ```bash
    export PATH=<install_dir>/bin:$PATH
    export LD_LIBRARY_PATH=<install_dir>/lib:$LD_LIBRARY_PATH
    export CMAKE_PREFIX_PATH=<install_dir>:$CMAKE_PREFIX_PATH
    ```
* [gz-usd](https://github.com/gazebosim/gz-usd)
* [sdformat](https://github.com/gazebosim/sdformat)

# Setup

Build `gz-usd`. The steps below follow a traditional cmake build, but `gz-usd`
can also be built with [colcon](https://colcon.readthedocs.io/en/released/index.html):

**Note: Be sure to build gz-usd on a terminal with the above environment variables exported.**

```bash
git clone https://github.com/gazebosim/gz-usd
cd gz-usd
mkdir build
cd build
cmake ..
make
```

You should now have an executable named `sdf2usd` in the `./build/bin` directory.
This executable can be used to convert a SDF world file to a USD file.
To see how the executable works, run the following command from the `./build/bin` directory:
```bash
./sdf2usd -h
```


### Note about building with colcon

You may need to add the USD library path to your `LD_LIBRARY_PATH` environment variable after sourcing the colcon workspace.
If the USD library path is not a part of `LD_LIBRARY_PATH`, you will probably see the following error when running the `sdf2usd` executable:

```bash
sdf2usd: error while loading shared libraries: libusd_usd.so: cannot open shared object file: No such file or directory
```

The typical USD library path is `<usd_installation_path>/lib`.
So, if you installed USD at `/usr/local/USD`, the following command on Linux properly updates the `LD_LIBRARY_PATH` environment variable:
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/USD/lib
```

Another thing to note if building with colcon is that after sourcing the workspace with sdformat,
the `sdf2usd` executable can be run without having to go to the `./build/bin` directory.
So, instead of going to that directory and running `./sdf2usd ...`, you should be able to run `sdf2usd ...` from anywhere.

## ROSCon 2022

[![](img/video_img.png)](https://vimeo.com/767140085)
