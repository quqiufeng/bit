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

### Phase 1：类型表与声明（完成）

- 已创建 `c0v2.c` 和 `c0v2-build.sh`
- 已加入 `g_type/g_elem/g_dims/g_row` 与 `l_type/l_elem/l_dims/l_row`
- 声明解析已填充类型表

### Phase 2：char/数组/二维数组（完成）

- `char` 1 字节 load/store
- 一维/二维数组、行步长、数组退化
- `char a[2][3]`、`char s[4]` 测试通过
- `++/--`、复合赋值、简单 cast 已加入
- c0v2 可继续编译 `bootstrap.c` 并接入 chain
- `selfsource` 当前推进到 `strcmp(const char *a, ...)`，下一个阻塞是指针参数

### Phase 2：char、数组、二维数组

- char 1 字节 load/store
- 一维/二维数组和行步长
- 数组退化
- 测试：`char s[4]; s[0]='A';`

### Phase 3：指针（进行中）

- 指针类型、`char *` 参数、`&x`、`*p`、`p[i]`、指针算术已加入
- `int *p` 和 `char *p` 基本索引已通过
- `selfsource` 已能把 `c0v2.c` 编成约 115KB A1，A1 能装配出 ELF
- `c0v2` 第一代自编译输出仅 836 字节，固定点未达成
- 已确认 stage1 能正确读取 stdin（read 到 27 字节）
- `src_len` 在 read 后为 27，`src[0]` 为 105（`i`）
- 直接调用 `word("int")` 在 stage1 中返回 0
- 局部 `const char *w; w="int"; w[0]` 在 stage1 中返回 105
- 指针参数函数 `int dbg(const char *w){return w[0];}` 在 stage1 中返回 105
- 单独测试 `while (w[i])`、`src[pos]` 都能工作
- 组合测试 `word` 风格：`while (w[i]) { if (src[pos] != w[i]) ...; i++; pos++; }` 在 stage1 中失败
- 已尝试把 `src[pos]` 和 `w[i]` 拆成两个局部变量，仍失败
- 这指向 C0v2 在为“全局 char 数组索引 + 指针参数索引 + while 循环”组合生成代码时的寄存器/栈状态问题
- 新对比结果：
  - `char src[] + char wbuf[]` 双全局数组循环：通过
  - `while(w[i]) + src[pos]`（w 是函数指针参数）：失败
  - 把 `w` 先 `strcpy` 到全局 `wbuf` 后仍失败
  - 说明触发条件与“函数指针参数”本身有关，不只是指针索引表达式
- 已定位并修复：数组变址 `src[pos]` 在解析索引后丢失了元素类型，导致 char 数组被按 4 字节 int 加载；`word` 因此把 `src[pos]` 读成 4 字节值
- 修复后 `word`/`wordtest` 返回 42，char 指针和 `src[pos]` 正常
- 第一代 c0v2 仍只输出 837 字节 header；说明还有后续独立 bug
- 修复后 stage1 中 `type_word` 已能返回真，但紧接着调用 `ident` 失败
- 单独测试：`pos=4; ident(nm)` 成功；`type_word(); ident(nm)` 失败
- 说明 `type_word` 返回后向调用者留下了错误的栈/寄存器状态
- `skip(); type_word()` 单独退出码正常，但后续 `ident` 仍失败
- 已定位并修复一个关键 codegen bug：C0v2 对空条件 `for (;;)` 仍无条件发射 `cmp rax,0; je end`，会读取陈旧 RAX，导致循环被跳过
- 修复后 stage1 已能进入主循环并执行到 `type_word()` 成功（退出码 14 证明）
- 新的精确现象：在循环开头 `src_len-pos<=2` 处退出时 `pos=27`，但 `stage=2` 后的 `ident` 断点（113）没有触发
- 说明 `type_word` 成功后、`stage=2` 前，`pos` 被推进到源码末尾（或控制流/跳转错误），且没有进入 ident 块
- 注入 31/33 拦截点后没有任何退出码触发，stage1 仍只输出 908 字节 header
- 这说明当前故障对代码布局敏感：注入诊断代码本身会改变控制流/栈布局
- 之前退出码 14（type_word 成功）与现在的无拦截结果不一致，说明不是单纯固定路径 bug，而是 codegen 的布局/标签/栈状态问题
- 已反汇编 stage1 的 `main`：在 `word("extern")` 之后，代码进入了一条 `ch(';')` 路径（typedef 跳过逻辑），而不是 `word("typedef")` 检查
- 该路径在输入开头就是 `int` 的情况下仍执行，随后把 `pos` 消费到源码末尾的 `;`，因此 `pos=27`
- 说明 `if (word("typedef"))` 的条件/字符串/调用生成错误，导致把普通输入误判为 typedef 声明
- 下一步：审计 C0v2 生成的 `word("typedef")` 调用和条件跳转
- 下一步：在修复后的 stage1 上检查 `stage=2` 赋值、`char name[64]` 局部块和 `ident` 调用

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
