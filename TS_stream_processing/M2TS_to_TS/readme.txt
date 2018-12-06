此程序功能：
将 M2TS 文件转换为 TS 文件。此程序会搜索当前目录下 所有扩展名是 .m2ts 和 .mts 的文件，将它们转换为 .ts 文件。
转换过程中会检查 TS包的同步码 0x47.
原始的 .m2ts 和 .mts 文件不删除。

【此程序可以处理超过 2 GiB 的文件】

这是在 regular_extract 工程的基础上修改得到的。就是 regular_extract 当 offset = 4, interval = 192, k = 188 时的特例。

PS：regular_extract 工程：
从一个二进制文件的第 offset 个字节开始(文件的开始约定为offset=0)，每 interval 个字节(每隔interval-k个字节)取出k个字节，存入一个新文件。
----------------------------------------
Changes in v0.2:
1. Fixed a log print bug when file creation fails.
2. Avoid calling malloc() and free() for the 192-byte buffer file-by-file, do it once in main().
3. Added input file format detection; now special files (the 1st sync byte is at offset = 0) can be processed.
4. When error occurs, delete outout TS file automatically.
