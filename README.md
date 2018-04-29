ViennaTS
--------------------------

Developer repository for ViennaTS, a C++, OpenMP-parallelized Topography simulator.
ViennaTS is currently in a prototype state.

System requirements
--------------------------

* C++ compiler
* OpenMP
* Boost C++ Libraries
* HDF5 (optional - required for TDR file support)
* Qt5 inkl. QtDataVisualization (optional - required for Gui support)

Currently supported operating systems
--------------------------
* GNU/Linux (32/64Bit)

Building instructions
--------------------------

To build ViennaTS, clone the repository and issue the following suggested commands:

<pre>
$> cd viennats-dev    # the checked-out GIT folder
$> mkdir build        # the build folder
</pre>

Configure the build, default build type is the 'optimized/release' mode:
<pre>
$> cd build/
$> cmake ..
</pre>
Watch for Warning/Error messages during the configuration stage.

Now build the 'viennats' simulation executable 
<pre>
$> make 
</pre>

CMake Options
--------------------------

<pre>
CMAKE_BUILD_TYPE   = debug, (release) # Turn off/on optimizations (default: release, i.e., optimized mode)
</pre>

Using the Gui
--------------------------
In order to use the Gui you need to install Qt5 with the following components: Qt5Widget, Qt5Core, Qt5Gui, Qt5DataVisualization.
Download Qt from the official website (https://www.qt.io/) and follow the instructions. Don't forget to check the Data Visualization
Component if you use the online installer. If you build Qt from Git you need to also install the Data Visualization Module (also on Git).

If you have problems with cmake and Qt, make sure your Qt Path is set correctly.
On Ubuntu, locate your .bashrc file and add the following at the bottom:

<pre>
export QTDIR=/path/to/Qt/version/gcc_64
export PATH=$PATH:$QTDIR/bin
export LD_LIBRARY_PATH=$QTDIR/lib:$LD_LIBRARY_PATH
</pre>

Save the file and check if QMake now has the correct path. Enter in a terminal:

<pre>
Qmake -v
</pre>

You should now see "Using Qt version 5.x.y in /path/to/qt/version/gcc_64/lib"

Authors and Contact
------------------------

Current contributors: Lado Filipovic, Paul Manstetten, Xaver Klemenschits and Josef Weinbub

Contact us via: viennats@iue.tuwien.ac.at

Founder and initial developer was Otmar Ertl; not active anymore.

ViennaTS was developed under the aegis of the 'Institute for Microelectronics' at the 'TU Wien'.
http://www.iue.tuwien.ac.at/

License
--------------------------
See file LICENSE in the base directory.
