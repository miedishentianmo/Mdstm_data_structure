#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int u4;
typedef unsigned long long u8;
typedef __int128_t i16;
typedef __uint128_t u16;

/**------------------------- MBint - 大型无符号整数-------------------------
 * 用一个 u8 数组表示大整数，数组每一项表示大整数的 64 个二进制位
 * 为了省事，数组长度不会动态变化，且不会检测是否超出长度限制
 * 该数据结构仅确保能成功计算特定任务，其他情况下使用并不安全！
 * 举例：
 * MBint x;
 * u8 arr[3] = {3, 4, 5};
 * x.a = arr;
 * x.b = arr + 3;
 * 此时 x 表示的整数值为 3 + 4*(2**64) + 5*((2**64)**2)
 * 当且仅当 x.a == x.b 时 x == 0
 */
class MBint
{
public:
    u8 *a, *b; // 包含有效数字的数组的开头和结尾

    // 交换两个指针
    static void swap(MBint *&x, MBint *&y)
    {
        MBint *z = x;
        x = y;
        y = z;
    }

    // 复制数组
    void setBy(const MBint *x)
    {
        u8 len = x->b - x->a;
        b = a + len;
        memcpy(a, x->a, len * 8);
    }

    // 是不是 0
    int isZero() const
    {
        return b == a;
    }

    // this += x; (this >= x)
    void add(const MBint *x)
    {
        u8 *p = a, *q = x->a;
        u16 carry = 0;

        for (; q < x->b; ++p, ++q)
        {
            carry += (u16)*p + *q;
            *p = (u8)carry;
            carry >>= 64;
        }
        for (; (u4)carry != 0; ++p)
        {
            if (p >= b)
            {
                *b++ = 1;
                return;
            }
            carry += *p;
            *p = (u8)carry;
            carry >>= 64;
        }
    }

    // this -= x; (this >= x)
    void sub(const MBint *x)
    {
        u8 *p = a, *q = x->a;
        i16 carry = 0;

        for (; q < x->b; ++p, ++q)
        {
            carry += (i16)*p - *q;
            *p = (u8)carry;
            carry >>= 64;
        }
        for (; (u4)carry != 0; ++p)
        {
            carry += *p;
            *p = (u8)carry;
            carry >>= 64;
        }
        while (b > a && *(b - 1) == 0)
        {
            --b;
        }
    }

    // this *= x; (x != 0)
    void mul(u8 x)
    {
        u16 carry = 0;

        for (u8 *p = a; p < b; ++p)
        {
            carry += (u16)*p * x;
            *p = (u8)carry;
            carry >>= 64;
        }
        if ((u8)carry != 0)
        {
            *b = (u8)carry;
            ++b;
        }
    }

    // quo = this / x; return this % x; (x != 0)
    u4 div(u4 x, MBint *quo) const
    {
        quo->b = quo->a + (b - a);
        u8 carry = 0;

        for (u4 *p = (u4 *)b, *q = (u4 *)quo->b; --p >= (u4 *)a;)
        {
            carry = (carry << 32) + *p;
            *--q = (u4)(carry / x);
            carry %= x;
        }
        if (quo->b > quo->a && *(quo->b - 1) == 0)
        {
            --quo->b;
        }

        return (u4)carry;
    }

    // this <<= x; (0 <= x <= 64)
    void shl(u4 x)
    {
        u4 y = 64 - x;
        u8 *p = b, t;

        if (--p >= a)
        {
            if ((t = *p >> y) != 0)
            {
                *b++ = t;
            }
            *p <<= x;
            while (--p >= a)
            {
                *(p + 1) |= *p >> y;
                *p <<= x;
            }
        }
    }

    // 转化为十进制字符串存储在 dst_str，之后自己会发生变化
    // tmp_mb 至少有和自己相同大小的空间，dst_str 和 tmp_str 也要有足够空间
    void getDecStr(MBint *tmp_mb, char *dst_str, char *tmp_str)
    {
        MBint *x = this;
        char *p = dst_str, *q = tmp_str;

        // 每次用 self 除以 10**9，并将余数转化为字符串添加到 tmp_str 结尾 9 个字节
        for (;; q += 9)
        {
            u4 r = x->div(1000000000, tmp_mb);
            if (tmp_mb->isZero())
            {
                sprintf(p, "%u", r);
                break;
            }
            sprintf(q, "%09u", r);
            swap(x, tmp_mb);
        }
        // tmp_str 类似于一个栈，每次将 tmp_str 最后 9 个字节移动到 dst_str 末尾
        for (p += strlen(p); q > tmp_str; p += 9)
        {
            memcpy(p, q -= 9, 9);
        }
        *p = '\0';
    }

    // self = 10 ** n;
    void setBy10En(u8 n)
    {
        u8 q = 10, r = n % 19;

        // 快速幂先计算 10 ** (n % 19)
        for (b = a + 1, *a = 1; r != 0; r >>= 1)
        {
            if ((r & 1) != 0)
            {
                *a *= q;
            }
            q *= q;
        }
        // 再连续乘以 (n / 19) 次 10**19
        for (q = n / 19; q != 0; --q)
        {
            mul(10000000000000000000ull);
        }
    }

    // this *= pi; 每计算一次有 4.471 个有效位
    void formula_1(
        MBint *x0, MBint *x1, MBint *x2, MBint *x3, MBint *x4,
        MBint *t0, MBint *t1)
    {
        div(43, x0);
        x0->mul(22);
        div(239, x1);
        x1->mul(51);
        div(341, x2);
        x2->shl(4);
        div(5357, x3);
        x3->mul(44);
        div(12943, x4);
        x4->mul(68);
        setBy(x0);
        add(x1);
        add(x2);
        add(x3);
        add(x4);
        for (u4 k = 3, c = 0;; k += 2, c = !c)
        {
            x0->div(29584, t0);
            swap(x0, t0);
            x1->div(57121, t0);
            swap(x1, t0);
            x2->div(465124, t0);
            swap(x2, t0);
            x3->div(28697449, t0);
            swap(x3, t0);
            x4->div(167521249, t0);
            swap(x4, t0);
            t0->setBy(x0);
            t0->add(x1);
            t0->add(x2);
            t0->add(x3);
            t0->add(x4);
            t0->div(k, t1);
            if (t1->isZero())
                break;
            if (c)
                add(t1);
            else
                sub(t1);
        }
        shl(2);
    }
};

void printPI(u8 precision)
{
    u8 n = precision + 10; // 多计算 10 位，减少误差
    // pi*10**n < (2**64)**len <==> len > (n*log_2(10)+log_2(pi))/64
    u8 len = (u8)(n * 0.05190512648261505 + 0.025804627023004984) + 1;
    MBint x, x0, x1, x2, x3, x4, t0, t1;
    char *str;

    x.a = (u8 *)malloc(len * 8 * 8);
    x0.a = x.a + len;
    x1.a = x0.a + len;
    x2.a = x1.a + len;
    x3.a = x2.a + len;
    x4.a = x3.a + len;
    t0.a = x4.a + len;
    t1.a = t0.a + len;
    str = (char *)x1.a;

    x.setBy10En(n);
    x.formula_1(&x0, &x1, &x2, &x3, &x4, &t0, &t1);

    // 此时 x 的值应为 pi*(10**n)
    x.getDecStr(&x0, str, str + n + 1);
    str[precision + 1] = '\0';
    printf("%c.%s\n", *str, str + 1);

    free(x.a);
}

int main(int argc, char **argv)
{
    long long precision;

    if (argc < 2 || (precision = atoll(argv[1])) <= 0)
    {
        return 1;
    }
    printPI((u8)precision);
    return 0;
}
