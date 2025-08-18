# Bin文件 CRC32 校验增加
## 开发背景
我需要在Bin文件生成后， 在文件末尾添加一个CRC校验， 验证文件是否被修改。
## 功能描述
### 生成一个程序， 程序名为 BinCRC.exe, 这是一个命令行工具， 输入参数说明如下：
参数1：crc_method， 可选值：CRC32、CRC16、CRC8
参数2：bin_file_path：bin 文件路径
参数3：crc_calculation_length：CRC校验长度, 如果未指定，则默认为整个bin文件的长度

### 程序运行逻辑：
1. 读取bin 文件， 获取bin 文件长度， 如果参数3未指定，则默认为bin 文件长度， 如果指定参数3，则使用参数3的值，
需要注意的是参数3的值如果大于bin 文件长度， 则填充 0xFF 字节在 bin 文件末尾， 填充字节数等于参数3的值减去bin 文件长度，再减去CRC结果值长度。
举例一： 参数3的值为20，bin 文件长度为5，使用CRC8 校验会生成一个字节的校验结果，则填充（20 - 5 - 1）个0xFF字节， CRC校验结果填充在bin 文件末尾，总文件长度就是 20
举例二： 假设参数3的值为20，bin 文件长度为25，不需要填充0xFF字节，CRC8 校验结果直接填充在bin 文件末尾, 总文件长度就是 26
2. 根据 crc_method 计算CRC校验值，并填充CRC校验值到bin 文件末尾
3. 将bin 文件命名追加 CRC校验值
举例： 原文件名称 test.bin， CRC校验值是0x01，则生成的文件名称为 test_01.bin

### CRC校验方法
1. CRC8 校验方法
```
unsigned char crc8(unsigned char *ptr,unsigned int len, unsigned short crc=0xFF)
{
    unsigned char i;
    while (len--)
    {
        crc ^= *ptr++;
        for (i=0; i<8; i++)
        {
            if (crc&0x01)
            {
                crc = (crc>>1) ^ 0x8C;
            }   
            else
            {
                crc >>= 1;
            }
        }   
    }
    
    return(crc);
}
```
2. CRC16 校验方法
```
unsigned short crc16(unsigned char *ptr,unsigned int len, unsigned short crc=0xFFFF) 
{
    unsigned char i;
    
    while (len-- != 0)
    {
        for (i=0x80; i!=0; i>>=1)
        {
            if ((crc&0x8000) != 0) 
            {
                crc <<= 1;
                crc ^= 0x1021;
            }
            else 
            {
                crc <<= 1;
            }
                 
            if ((*ptr&i) != 0) 
            {
                crc ^= 0x1021;
            }
        }        
        ptr++;    
    }
    
    return(crc);
}
```
3. CRC32 校验方法
```
unsigned int crc32(unsigned int *message, unsigned int msgsize, unsigned int crc = 0xFFFFFFFF)
{
    unsigned int i, j; // byte counter, bit counter
    unsigned int byte;
    unsigned int poly = 0x04C11DB7;
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
1. 使用 C++ 编程, 使用 C++11 标准
2. 运行环境为 Windows 10
3. 本机编译环境为 Mingw64

## 测试素材生成并测试
1. 创建一个test 文件夹
2. 创建一个 4096 字节的bin 文件， 文件为 0x00 - 0xFF的重复
3. 分别使用 CRC8, CRC16, CRC32 追加校验值到 bin 问末尾
