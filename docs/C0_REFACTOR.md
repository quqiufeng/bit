# C0 类型系统重构计划

目标是让 Linux C0 能编译 `bootstrap/linux/c0/c0.c` 自身，并保持现有
`C0 -> Windows C1 -> SysV Linux C1` 链路逐字节不变。

## 1. 边界

必须支持 `c0.c` 实际使用的 C 子集：

```text
int / char / u64 / u32 / void
static / extern 原型 / typedef
const
char * / const char *
一维和二维数组
函数参数与返回值
if/else / while / for
break / continue
return / 空 return
++ / -- / 复合赋值
取地址、解引用、数组退化、指针算术
字符串和字符常量
```

明确不支持：

```text
struct / union / enum
switch / goto
浮点
可变参数
位域
函数指针
预处理器
```

`c0.c` 本身没有 `#define`；自举源不使用预处理器。

## 2. 类型模型

所有表达式结果仍在 `RAX`，类型只决定加载/存储宽度和指针缩放。

```text
T_VOID
T_INT       4 字节
T_U32       4 字节
T_U64       8 字节
T_CHAR      1 字节
T_PTR       ���针，记录 pointee 类型
T_ARRAY     数组，记录元素类型、维数、行大小、总大小
```

符号表字段：

```text
name
type
elem_size      1/4/8
is_array
dims           1/2
row_bytes      二维数组行字节数
total_bytes
storage        global/local/param/function
offset/label
```

函数符号额外记录：

```text
return_type
param_count
param_type[]
```

局部布局沿用现有规则：

- 标量按 8 字节槽，char 标量仍占 8 字节但 load/store 1 字节
- 数组按实际元素宽度分配，8 字节对齐
- 栈帧 16 字节对齐
- 偏移 >127 使用 `disp32`

## 3. 声明语法

新增解析：

```c
static char src[1048576];
static char g_name[256][64];
static char *p;
static const char *s;
extern long long f(...);   /* 已跳过 */
typedef unsigned long long u64;
char *strcpy(char *d, const char *s);
void f(void);
int main(void);
```

规则：

- `static` 只影响链接性，C0 自举阶段忽略
- `extern` 原型直接跳过
- `typedef` 记录别名到真实类型
- `const` 只跳过
- `*` 可出现在类型和声明符中
- `[]` 可重复，最多二维
- `void` 函数可以没有返回值

## 4. 左值与右值

统一地址优先：

```text
gen_addr(lvalue) -> RAX 地址
load(type, addr) -> RAX 值
store(type, addr, value)
```

规则：

- 标量：`gen_addr` 给变量地址，load/store 按类型宽度
- 一维数组：表达式退化为首元素地址
- 二维数组：`a[i]` 退化为第 i 行首地址，行步长 `row_bytes`
- `a[i][j]` 两级变址
- 指针变量：先 load 8 字节指针，再按 pointee 大小缩放
- `&x`：返回地址，类型为指向 x 的指针
- `*p`：取地址后 load pointee
- 字符串常量：类型 `char *`，放在代码段，用 `%label` 引用

## 5. 指针算术

```text
ptr + int  -> ptr，int 乘以 pointee size
int + ptr  -> ptr
ptr - int  -> ptr
ptr - ptr  -> int，除以 pointee size（如需要）
```

`char *` pointee size 1；`int *` 4；指针的指针 8。

## 6. 自增/自减/复合赋值

必须支持：

```c
i++
++i
i--
--i
p++
*p++ = *s++
p += k
n -= k
generated_bytes += 4
```

规则：

- 前缀：先改值，再作为表达式结果
- 后缀：先保存旧值，改值，表达式结果为旧值
- `*p++`：先取 p 旧值作为地址，再 p += pointee size，最后 deref 旧地址
- `p += k`：按 pointee size 缩放
- 普通复合赋值：load、运算、store

## 7. 函数

- 参数最多 4 个；现有 bootstrap 子集已满足
- 前 4 个 SysV 参数：`RDI/RSI/RDX/RCX`
- 参数按声明类型落栈；指针参数 8 字节
- 返回值在 `RAX`；`void` 返回值未定义
- 调用点记录返回类型，供后续指针/char 使用

## 8. 迁移策略

不直接改现有 `c0.c`，避免破坏已验证链路。

新增并行文件：

```text
bootstrap/linux/c0/c0v2.c
```

阶段：

### Phase 0：基线锁定

- 现有 `c0.c`、A1、seed、bootstrap 链全部保留
- 记录 `c1-stage0/1/2` 哈希

### Phase 1：类型表与声明（进行中）

- 已创建 `c0v2.c` 和 `c0v2-build.sh`
- 已加入 `g_type/g_elem/g_dims/g_row` 与 `l_type/l_elem/l_dims/l_row` 骨架
- 当前仍保持旧的 int 行为；`c0v2` 对 `bootstrap.c` 的输出与旧 `c0` 逐字节一致
- 下一步：让声明解析填充类型表

### Phase 2：char、数组、二维数组

- char 1 字节 load/store
- 一维/二维数组和行步长
- 数组退化
- 测试：`char s[4]; s[0]='A';`

### Phase 3：指针

- `char *` / `const char *`
- `&`、`*`、`p[i]`
- 指针算术
- 测试：手写 `strcpy`/`strcmp`

### Phase 4：自增/复合赋值

- `++/--`
- `+=` / `-=`
- `*d++ = *s++`
- 测试：`b[n++]`、`a++;`

### Phase 5：函数签名

- 指针参数/返回值
- `void`
- 测试：`char *strcpy(char *, const char *)`

### Phase 6：C0 自举

- `c0v2` 编译 `c0v2.c`
- A1 装配 c0v2
- 新 c0v2 再编译自身
- 文本/二进制固定点

### Phase 7：替换

- c0v2 通过所有 C0 测试和 bootstrap 链
- 重命名为 `c0.c`
- 重新生成 c0 seed
- 删除旧实现

## 9. 验收矩阵

每个阶段都必须通过：

1. 现有 `./build.sh`
2. `c0/build.sh`
3. `c0/selfhost.sh`
4. `c0/chain.sh`
5. `c0/linux.sh`
6. A0/A1 selfhost
7. 新增类型单元测试
8. 最终 `c0 v2 -> bootstrap.c -> capacity -> compiler.c -> stage1 == stage2`

最终额外验收：

```text
c0v2 < c0v2.c > c0v2.a1
a1 < c0v2.a1 > c0v2-stage1.elf
c0v2-stage1 < c0v2.c > c0v2-stage2.a1
diff c0v2.a1 c0v2-stage2.a1
```

## 10. 风险与回滚

高风险点：

- 二维数组行步长
- 指针/数组退化
- `*p++` 求值顺序
- char 符号扩展
- 复合赋值的左值只求值一次
- 参数/返回指针类型丢失

回滚方式：

- `c0.c` 在 Phase 7 前不动
- c0v2 独立构建
- 任一阶段失败只删除 c0v2，不影响主线
