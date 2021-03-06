# 计算圆周率

## 计算原理

反正切公式：[点此验证](https://www.wolframalpha.com/input?i=68ArcTan%5B1%2F12943%5D%2B44ArcTan%5B1%2F5357%5D%2B32ArcTan%5B1%2F682%5D%2B51ArcTan%5B1%2F239%5D%2B88ArcTan%5B1%2F172%5D)
![反正切公式](pic/0.png)

反正切函数展开式：
![展开式](pic/1.png)

最终公式：
![最终公式](pic/2.png)

利用这个公式，每计算一项可以得到约 4.471 个有效数字。

## 数据结构

为了计算很长的位数，我设计了一个大型无符号整数类 ```MBint```。

```MBint``` 用一个 u8 数组表示大整数，数组每一项表示大整数的 64 个二进制位。

内部有两个指针 ```u8 *a, *b;``` 分别指向包含有效数字的数组的开头和结尾，当且仅当 ```a == b``` 时表示零。

例如：
```
MBint x;
u8 arr[5] = {3, 4};
x.a = arr;
x.b = arr + 2;
```
此时 x 表示的整数值为 ```3 + 4*(2**64)```。

```MBint``` 实现了一些基本的运算功能：大整数之间的加减法、乘以 64 位整数、除以 32 位整数以及移位运算。

有了这些功能，结合最终公式，就可以计算出 10 的 n 次方乘以圆周率的整数值了。

## 运行效果

使用 64 位操作系统编译好程序后，命令行参数传入要计算小数点后的位数即可运行。

算法复杂度为 ```O(n**2)```，用我的电脑计算十万位需要 3~4 秒。

在服务器上计算了一千万位，未发现错误。
