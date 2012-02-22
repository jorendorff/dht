dht - Deterministic hash tables
===============================

This repo contains the source code I used to evaluate deterministic hash table performance in the specific context of implementing the ECMAScript Map object for modern JS engines.

Updates will be posted to [the deterministic hash tables page](https://wiki.mozilla.org/User:Jorend/Deterministic_hash_tables).

**To run benchmarks on Mac/Linux:**

* Install matplotlib. If you have MacPorts, you can do `sudo port install py27-matplotlib` but note that this doesn't install matplotlib in your Mac's system python installation. Instead it installs a copy of python in /opt/local/bin (or wherever you've configured ports to install stuff) and that copy has matplotlib.
* Edit the Makefile to set CXX, CXXFLAGS, and PYTHON to values that will work on your system.
* `make`
* Wait. The benchmarks take a while to run.

**To run benchmarks on Windows:**

On Windows, you're going to use a batch file, because Windows is awesome.

* You have to have Visual Studio.
* Install Python(x,y) from [http://code.google.com/p/pythonxy/](http://code.google.com/p/pythonxy/). I used Python(x,y) version 2.7.2.1.
* Open a Visual Studio Command Line (All Programs &rarr; Microsoft Visual Studio 2010 &rarr; Visual Studio Tools &rarr; Visual Studio Command Prompt (2010).
* cd to the dht directory.
* Run build.bat.
* Wait. The benchmarks take a while to run.

**What you get**

* figure-1.png shows how much memory each implementation allocates. figure-1-data.txt is the raw data.
* figure-2.png shows how much memory each implementation uses (that is, how much of the allocated memory is actually accessed). figure-2-data.txt is the raw data.
* The images InsertSmallTest-speed.png and friends show how fast each implementation is at each test. Higher is better. The file hashbench-data.txt contains the raw data for all these graphs. It's JSON.
