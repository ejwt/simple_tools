Prerequisite:

Copy stdint.h to c:\Program Files (x86)\VisualStudio9\VC\include\
----------------------------------------
Details:

采用标准数据类型的头文件，增强可移植性

用什么头文件来着？stdint.h
直接在C盘搜这个文件吧，发现N多开发工具都有，内容还不一样。STM32 ARM Cortex-M3 的，x86/Win32 的，当然不一样。

In C90, the "long" data type can serve both as the largest integral type, and as a 32-bit container. C99 removes this ambiguity through the new standard library header files <inttypes.h> and <stdint.h>.
所以这是一个 C99 的特性啊。

Visual C++ 2008 下面没有这个文件，怎么办？复制一个过去就行。
从 C:\Program Files (x86)\VisualStudio12\VC\include 复制到 c:\Program Files (x86)\VisualStudio9\VC\include\
----------------------------------------
PS:

注意，Microsoft Visual Studio Solution File 的版本并不是和 Microsoft Visual Studio 的版本对应。例如：

Microsoft Visual Studio Solution File, Format Version 10.00
# Visual C++ Express 2008

Microsoft Visual Studio Solution File, Format Version 11.00
# Visual Studio 2010
---------
而
Visual C++  9.0 == Visual C++ 2008
Visual C++ 10.0 == Visual C++ 2010

