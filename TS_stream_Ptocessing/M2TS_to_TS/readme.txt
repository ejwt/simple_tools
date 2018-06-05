此程序功能：
将 M2TS 文件转换为 TS 文件。此程序会搜索当前目录下 所有扩展名是 .m2ts 和 .mts 的文件，将它们转换为 .ts 文件。
原始的 .m2ts 和 .mts 文件不删除。

【此程序可以处理超过 2 GiB 的文件】

这是在 regular_extract 工程的基础上修改得到的。就是 regular_extract 当 offset = 4, interval = 192, k = 188 时的特例。

PS：regular_extract 工程：
从一个二进制文件的第 offset 个字节开始(文件的开始约定为offset=0)，每 interval 个字节(每隔interval-k个字节)取出k个字节，存入一个新文件。
