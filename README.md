DestructibleBox2D
=================

A small demo showing a way to do destructible terrain in Box2D relatively easily. Uses the awesome Boost.Geometry library.

Requirements for building:
* Boost (1.55+)

Building on Windows:
```
mkdir build
cd build
cmake path_to_source
```
And use the generated .SLN to build the demo. In case you have Visual Studio 2013 with the November CTP use
```
cmake path_to_source -T "CTP_Nov2013"
```

On Linux:
```
mkdir build
cd build
cmake path_to_source -DCMAKE_BUILD_TYPE=Release
```
