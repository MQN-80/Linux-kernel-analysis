# Linux中的汇编语言
> Linux源码中出现的汇编代码可以分为两种，一种是以.S为扩展名的汇编文件中，整个程序全部由汇编语言组成;另一种是有些汇编命令，会出现在c文件中，在这种文件中，既有c语言又有汇编语言，这种汇编语言称为`嵌入式汇编`，下面我们将对Linux中的汇编语言进行讨论.
## 1.AT&T和Intel汇编语言的比较
> 当`UNIX`被移植到`i386`时，采用了`AT&T`的汇编语言格式，下面我们将比对这两种语法的格式
### 1.1前缀
在Intel语法中，寄存器和立即数都没有前缀，但在`AT&T`中，寄存器冠以"%"前缀，而立即数冠以"$"前缀；在`Intel`语法中，十六进制和二进制立即数后缀为"h"和"b"，在`AT&T`中,十六进制立即数前冠以"0x"
### 1.2操作数方向
两者的操作数方向刚好相反
在`Intel`语法中，第一个操作数是目的操作数，第二个操作数是源操作数;而在`AT&T`中，第一个数是源操作数，第二个数是目的操作数
### 1.3内存单元操作数
在`Intel`语法中，基寄存器用"[]"括起来，而在`AT&T`中，用"()"括起来
### 1.4间接寻址方式
`Intel`的指令格式为$segreg:[base+index*scale+disp]$,而`AT&T`的格式为$segreg:disp(base,index,scale)$,如下给出两种方式间接寻址的例子:

|Intel语法|AT&T语法
|:---|:----|
|foo,segreg:[base+index*scale+disp]|%segreg:disp(base,index,scale),foo
|mov  eax,[ebx+20h]|0x20(%ebx),%eax
|add eax,[ebx+ecx*2h]| Addl (%ebx,%ecx,0x2),%eax
|sub eax,[ebx+ecx*4h-20h]| Subl -0x20(%ebx,%ecx,0x4),%eax

这种寻址方式从定义上就可以看出，用于访问数组某个特定元素内的一个字段，其中base为数组的起始地址，`scale`为每个数组元素的大小，`index`为下标，如果数组元素还是一个结构，则`disp`为具体字段在结构中的位移部分
### 1.5操作码的后缀
> 在`AT&T`的操作码后面有一个后缀，其含义就是指出操作码的大小，其中"l"表示32位长整数,"w"表示字(16位),"b"表示字节(8位)
## 2. AT&T汇编语言相关知识
> 在`Linux`源码中,以`.S`为扩展名的文件为纯汇编文件，这里我们介绍一些基本的`AT&T`汇编语言知识
### 2.1 GNU汇编程序和连接程序
当编写完一个程序后，就需要对其进行汇编和连接，在`Linux`中有两种方式完成，一种是使用汇编程序GAS,一种是使用gcc

在这里我们介绍gcc的用法，可以一步完成汇编和连接，例如
```Makefile
gcc -o example example.S 
```
这里，example.S 是你的汇编程序，输出文件（可执行文件）名为 example。其中，扩展名必须为大写的 S，这是因为，大写的 S 可以使gcc 自动识别汇编程序中的 C 预处理命令，像#include、#define、#ifdef、#endif 等，也就是说，使用 gcc 进行编译，你可以在汇编程序中使用 C 的预处理命令
### 2.2 AT&T中的节
在AT&T的语法中，一个节由关键词`.section`来标识，在汇编语言程序中，有以下三个节:   
`.section .data:`这种节包含已经初始化的数据，也就是包含程序已初始化的数据，也就是包含具有初值的变量，例如:
```armasm
hello: .string "Hello world!\n" 
hello_len : .long 13 
```
`.section .bss:`这个节包含程序还未初始化的数据，也就是没有初值的变量。因此当操作系统装入这个程序时，将这些变量都置为0,例如
```armasm
name : .fill 30 /** 用来请求用户输入名字 **/
name_len : .long 0 /** 名字的长度（尚未定义） **/
```
当该程序被装入时，name和name_len都被置为0，即使赋值，变量值也仍会被赋为0  
使用.bss的优势在于,.bss不占用磁盘空间，一个长整数就足以存放`.bss`节  
注意，编译程序把.data 和.bss 在 4 字节上对齐（align），例如，.data 总共有 34 字节，那么编译程序把它对齐在 36 字节上，也就是说，实际给它 36 字节的空间。  
`section .text:`这个节包括程序的代码，它是只读节，而.data和.bss是读写节
### 2.3 汇编程序指令
>`GNU`汇编程序提供了众多指令，这些指令都是以.开头，后跟指令名，在此我们介绍内核源代码中出现的几个指令  

[1].ascii "string"...
`.ascii`表示零个或多个字符串，并把每个字符串中的字符放在连续的地址单元,该情况下每个字符串结尾不自动加0  
因此有类似的一个指令`.asciz`，其中z代表0，也就是每个字符串结尾自动带一个0字节，例如
```armasm
int_msg: 
 .asciz "Unknown interrupt\n" 
```
[2].byte表达式  
`.byte`表示0或多个表达式，用逗号隔开，每个表达式被放在下一个字节单元  
[3].fill表达式  
形式:$ .fill  repeat,size,value$  
其中，repeat、size 和 value 都是常量表达式，Fill 的含义是反复拷贝 size 个字节。repeat 可以大于等于 0。size 也可以大于等于 0，但不能超过 8，如果超过 8，也只取 8。把repeat 个字节以 8 个为一组，每组的最高 4 个字节内容为 0，最低 4 字节内容置为 value。 size 和 value 为可选项。如果第 2 个逗号和 value 值不存在，则假定 value 为 0。如果第 1 个逗号和 size 不存在，则假定 size 为 1。 
例如，在 Linux 初始化的过程中，对全局描述符表 GDT 进行设置的最后一句为： 
```armasm
.fill NR_CPUS*4,8,0 /* space for TSS's and LDT's */ 
```
因为每个描述符正好占 8 个字节，因此，.fill 给每个 CPU 留有存放 4 个描述符的位置。
[4] .global symbol  
.global 使得连接程序(ld)能够看到`symbol`,因此假如局部程序中定义了symbl,则与这个局部程序连接的其他局部程序也能存取symbl，类似于全局变量的概念，例如
```armasm
.globl SYMBOL_NAME（idt） 
.globl SYMBOL_NAME（gdt） 
```
该程序就定义了`idt`和`gdt`为全局符号  
[5] .quad bignums  
.quad在Linux内核代码中非常常见，其表示零个或多个`bignums`，对每个`bignum`,其缺省值为8字节整数，例如在对全局描述符表的填充中就用到了这个指令
```armasm
.quad 0x00cf9a000000ffff /* 0x10 kernel 4GB code at 0x00000000 */ 
.quad 0x00cf92000000ffff /* 0x18 kernel 4GB data at 0x00000000 */ 
.quad 0x00cffa000000ffff /* 0x23 user 4GB code at 0x00000000 */ 
.quad 0x00cff2000000ffff /* 0x2b user 4GB data at 0x00000000 */
```
[6] .rept count
该指令用于将`.rept`和`endr`指令之间的行重复`count`次,例如
```armasm
.rept 3
.long 0
.endr
```
该指令就相当于
```armasm
.long 0 
.long 0 
.long 0 
```
[7] .space 
