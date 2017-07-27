
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
**GNU gold** is used for "modern" linking.


For call-handling : {GEA, FR0,GR0, ...}_STU HLP_STR LOCATE
A mkdir output

$ cd output

$ cpio -idv < /tmp/object.cpio

3. Create *.cpio Archive with Selected Files

The following example creates a *.cpio archive only with *.c files.

$ find . -iname *.c -print | cpio -ov >/tmp/c_files.cpio




Get predefined of gcc.
echo | g++  -c - -dM -E
