�˳����ܣ�
�� M2TS �ļ�ת��Ϊ TS �ļ����˳����������ǰĿ¼�� ������չ���� .m2ts �� .mts ���ļ���������ת��Ϊ .ts �ļ���
ת�������л��� TS����ͬ���� 0x47.
ԭʼ�� .m2ts �� .mts �ļ���ɾ����

���˳�����Դ����� 2 GiB ���ļ���

������ regular_extract ���̵Ļ������޸ĵõ��ġ����� regular_extract �� offset = 4, interval = 192, k = 188 ʱ��������

PS��regular_extract ���̣�
��һ���������ļ��ĵ� offset ���ֽڿ�ʼ(�ļ��Ŀ�ʼԼ��Ϊoffset=0)��ÿ interval ���ֽ�(ÿ��interval-k���ֽ�)ȡ��k���ֽڣ�����һ�����ļ���
----------------------------------------
Changes in v0.2:
1. Fixed a log print bug when file creation fails.
2. Avoid calling malloc() and free() for the 192-byte buffer file-by-file, do it once in main().
3. Added input file format detection; now special files (the 1st sync byte is at offset = 0) can be processed.
4. When error occurs, delete outout TS file automatically.
