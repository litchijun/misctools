# Bin文件加密软件开发
## 开发背景
我需要在Bin文件生成后， 对文件进行加密

## 功能描述
### 生成一个程序， 程序名为 BinEncoder.exe, 这是一个命令行工具， 输入参数说明如下：
参数1：bin_file_path：bin 文件路径

### 程序运行逻辑：
1. 读取bin 文件， 获取bin 文件长度 filelength, 如果filelength 大于 0x1013, 那么将 bin 文件第 0x1010~0x1013 字节改写为 filelength。
参考代码如下：
```
BufferData[0x1010] = FileLength & 0xFF;
BufferData[0x1011] = (FileLength >> 8) & 0xFF;
BufferData[0x1012] = (FileLength >> 16) & 0xFF;
BufferData[0x1013] = (FileLength >> 24) & 0xFF;
```
2. 文件名称追加 "_FF" 后保存。比如 bin 文件名为 test.bin， 那么保存的文件名为 test_FF.bin
3. 读取第2步生成的 test_FF.bin， 继续对 Bin 文件进行加密, 输出到 test_enc.bin， 加密流程：
    3.1. 读取Bin文件， 获取文件长度, 每 56字节原始数据取反，加上 8字节包头重新组成一包数据，得到最大的包数 max_packet_num， 顺序写入 test_enc.bin 文件。
       8 字节的包头数据为:
        Byte0: 0xFE
        Byte1: 0x07
        Byte2: 数据包编号低字节
        Byte3: 数据包编号低字节
        Byte4: Byte8-Byte63 的数据累加和， 取最低的字节
        Byte5: Byte8-Byte63 的56字节数据进行 CRC32 校验后， 取 crc32 校验结果的最低字节
        Byte6: Byte8-Byte63 的56字节数据进行 CRC32 校验后， 取 crc32 校验结果的第二字节
        Byte7: 0x00
        Byte8-Byte63: 原始数据取反， 原始数据不足56字节时，填充0xFF

    3.2. test_enc.bin 文件开头插入一包 64字节的引导包数据。包格式为:
        Byte0: 0xFE
        Byte1: 0x06
        Byte2: 原始文件长度+256+1023/1024
        Byte3: max_packet_num 低字节
        Byte4: max_packet_num 高字节
        Byte5: Byte0-Byte4 的累加和
        Byte6: 0x00
        Byte7: 0x00
        Byte8-Byte22: 原始bin的0x1000 - 0x100E数据取反
        Byte23: 0x00
        Byte24-Byte27: Byte8-Byte23 数据 CRC32 校验值，小端模式存储
        Byte28-Byte63: 0x00

### 附录
1. max_packet_num 最大包号计算方法：假设原始文件长度 4097字节， 每包数据是 56字节， 那么最大包号 max_packet_num = ((4097 + (56 -1)) / 56) - 1
2. CRC32 校验方法：
CRC 使用的多项式 poly = 0x04C11DB7，待计算数据的前后分别插入一个 32 位的数据，头数据data_start = 0x7ce69b00 | BOARD_TYPE，尾数据 data_end = 0x67e67e00 | BOARD_TYPE。
```
#define CRC32_POLY 0x04C11DB7
#define BOARD_TYPE 0x12
#define CRC32_INIT (0x7ce69b00 | BOARD_TYPE)
unsigned int crc32(unsigned int *message, unsigned int msgsize, unsigned int crc = 0xFFFFFFFF)
{
    unsigned int i, j; // byte counter, bit counter
    unsigned int byte;
    unsigned int poly = CRC32_POLY;
    i = 0;
    for (i = 0; i < msgsize; i++)
    {
        byte = message[i];
        for (j = 0; j < 32; j++)
        {
            if ((int)(crc ^ byte) < 0)
            {
                crc = (crc << 1) ^ poly;
            }
            else
            {
                crc = crc << 1;
            }
            byte = byte << 1;    // Ready next msg bit.
        }
    }
    return crc;
}
```

## 开发约束
1. 使用 C++ 编程, 使用 C++11 标准， 要求静态链接， 不使用动态链接库
2. 运行环境为 Windows 10
3. 本机编译环境为 Mingw64

## 测试素材生成并测试
1. 创建一个test 文件夹
2. 创建一个 8K 字节的bin 文件， 文件为 0x00 - 0xFF的重复
