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
[7] .space size,fill  
这个指令用于保留`size`个字节的空间，每个字节的值为`fill`，size和fill都是常量表达式，如果逗号和`fill`被省略，则假定fill为0，例如
```armasm
.space 1024
```
该指令表示保留1024字节的空间，并且每个字节的值为0  
[8] .word expressions  
这个表达式表示任意一节中的一个或多个表达式（用逗号分开），表达式的值占两个字节，例如：
```armasm 
gdt_descr: 
.word GDT_ENTRIES*8-1 
表示变量 gdt_descr 的值为 GDT_ENTRIES*8-1 
```
[9] .org new-lc ,fill  
把当前节的位置计数器提前到 new-lc（New Location Counter）。new-lc 或者是一个常量表达式，或者是一个与当前子节处于同一节的表达式。也就是说，你不能用.org 横跨节：如果 new-lc 是个错误的值，则.org 被忽略。.org 只能增加位置计数器的值，或者让其保持
不变；但绝不能用.org 来让位置计数器倒退。 
### 2.4 gcc嵌入式汇编
在 Linux 的源代码中，有很多 C 语言的函数中嵌入一段汇编语言程序段，这就是 gcc 提供的“asm”功能，例如在 `include/asm-i386/system.h` 中定义的，读控制寄存器 CR0 的一个宏 `read_cr0()`：
```armasm
#define read_cr0（） （{ \ 
 unsigned int __dummy; \ 
 __asm__（ \ 
 "movl %%cr0,%0\n\t" \ 
 :"=r" （__dummy））; \ 
 __dummy; \ 
 }）
```
其中`__dummy`为C函数所定义的变量；关键词`__asm__`表示汇编代码的开始。括弧中第一个引号中为汇编指令 movl，紧接着有一个冒号，这种形式阅读起来比较复杂  
[1] 嵌入式汇编一般形式  
`__asm__ __volatile__ （"<asm routine>" : output : input : modify）; `  
其中，`__asm__`表示汇编代码的开始，其后可以跟`__volatile__`，其含义是避免“asm”指令被删除、移动或组合；然后就是小括弧，括弧中的内容是汇编指令的具体实现
*  `"<asm routine>"`为汇编指令部分，例如，`"movl %%cr0,%0\n\t"`。数字前加前缀%，如%1,%2 等表示使用寄存器的样板操作数。可以使用的操作数总数取决于具体 CPU 中通用寄存器的数量,因此，在用到具体的寄存器时就在前面加两个“%”，如`%%cr0`。
* 输出部分output，用以规定对输出变量（目标操作数）如何与寄存器结合的约束
（constraint）,输出部分可以有多个约束，互相以逗号分开。每个约束以“＝”开头，接着用一个字母来表示操作数的类型，然后是关于变量结合的约束。例如，上例中： `:"=r" （__dummy）` “＝r”表示相应的目标操作数（指令部分的%0）可以使用任何一个通用寄存器，并且变量__dummy 存放在这个寄存器中，但如果是： `：“＝m”（__dummy）` “＝m”就表示相应的目标操作数是存放在内存单元`__dummy` 中。 
表示约束条件的字母很多,下表给出了几个主要的约束字母及其含义。

|字母| 含义
|:---|:----|
|m, v,o |表示内存单元 
 |R |表示任何通用寄存器 
 |Q |表示寄存器 eax、ebx、ecx、edx 之一 
 |I, h| 表示直接操作数 
 |E, F |表示浮点数 
 |G| 表示“任意” 
 |a,b,c d| 表示要求使用寄存器 eax/ax/al, ebx/bx/bl, ecx/cx/cl 或 edx/dx/dl 

 * 输入部分:输入部分与输出部分相似，但没有“＝”。如果输入部分一个操作数所要求使用的寄存器，与前面输出部分某个约束所要求的是同一个寄存器，那就把对应操作数的编号（如“1”，“2”等）放在约束条件中
 * 修改部分（modify）:这部分常常以`memory`为约束条件，以表示操作完成后内存中的内容已有改变，如果原来某个寄存器的内容来自内存，那么现在内存中这个单元的内容已经改变。
 ### 2.4 Linux源代码中嵌入式汇编举例
 [1]. 简单应用  
 ```armasm
#define __save_flags（x） __asm__ __volatile__（"pushfl ; popl %0":"=g"(x): /* no input 
*/） 
#define __restore_flags（x） __asm__ __volatile__（"pushl %0 ; popfl": /* no output */ 
 :"g"(x):"memory", "cc"）
 ```
 > 此代码中`pushf`的含义就是`push flag`,它的作用就是将标志寄存器的值压栈，因此`popf`的作用就是将标志寄存器的值弹栈
 
 因此第一个宏用来保存标志寄存器的值，第2个宏用来恢复标志寄存器的值
 [2]. 较复杂应用
 ```armasm
static inline unsigned long get_limit（unsigned long segment） 
{ 
 unsigned long __limit; 
 __asm__（"lsll %1,%0" 
 :"=r" （__limit）:"r" （segment））; 
 return __limit+1; 
} 
 ```
 这是一个设置段界限的函数，汇编代码段中的输出参数为`__limit`即%0，输入参数为`segment`（即%1）,lsll 是加载段界限的指令，即把 `segment`段描述符中的段界限字段装入某个寄存器（这个寄存器与`__limit`结合），函数返回__limit 加 1，即段长。
 [3]. 复杂应用
 在 Linux 内核代码中，有关字符串操作的函数都是通过嵌入式汇编完成的，因为内核及用户程序对字符串函数的调用非常频繁，因此，用汇编代码实现主要是为了提高效率（当然是以牺牲可读性和可维护性为代价的）。在此，我们仅列举一个字符串比较函数`strcmp`，其代码在`arch/i386／string.h`中.
 ```armasm
static inline int strcmp（const char * cs,const char * ct） 
{ 
int d0, d1; 
register int __res; 
__asm__ __volatile__（ 
 "1:\tlodsb\n\t" 
 "scasb\n\t" 
 "jne 2f\n\t" 
 "testb %%al,%%al\n\t" 
 "jne 1b\n\t" 
 "xorl %%eax,%%eax\n\t" 
 "jmp 3f\n" 
 "2:\tsbbl %%eax,%%eax\n\t" 
 "orb $1,%%al\n" 
 "3:" 
 :"=a" （__res）, "=&S" （d0）, "=&D" （d1） 
 :"1" （cs）,"2" （ct））; 
return __res; 
} 
```
其中"\n"是换行符,"\t"是tab符，在每条命令的结束加这两个符号，是为了让gcc 把嵌入式汇编代码翻译成一般的汇编代码时能够保证换行和留有一定的空格，例如上述的嵌入式汇编会被翻译为:
```armasm
1： lodsb //装入串操作数，即从[esi]传送到 al 寄存器，然后 esi 指向串中下一个元素 
 scasb//扫描串操作数，即从 al 中减去 es:[edi]，不保留结果，只改变标志 
 jne2f//如果两个字符不相等，则转到标号 2 
 testb %al %al 
 jne 1b 
 xorl %eax %eax 
 jmp 3f 
2: sbbl %eax %eax 
 orb $1 %al 
3: 
```
该段代码输入和输出部分的结合情况为:
