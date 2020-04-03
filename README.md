
OXEng - strings builder
=======================
	
Use CMAKE in order to have  an "out of source" build. 
CMakeLists.txt file in the root of your project describes compilation steps. 

Then from the root of your project:

`mkdir Release
cd Release
cmake -DCMAKE_BUILD_TYPE=Release ..
make`

And for Debug (again from the root of your project):

`mkdir Debug
cd Debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make`

Debug will add the debug flags appropriate for your compiler. 

You could use Visual Code also.

**C++14** with standard STL for "Modern" c++.

→ scl enable llvm-toolset-7 bash
→ type clang
clang is hashed (/opt/rh/llvm-toolset-7/root/usr/bin/clang)

→ export CC=/opt/rh/llvm-toolset-7/root/usr/bin/clang

→ export CXX=/opt/rh/llvm-toolset-7/root/usr/bin/clang++

then cmake use CC/CXX clang compiler.






Get predefined of gcc.
echo | g++  -c - -dM -E
