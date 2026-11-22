# 源码保留策略

天梯只需要 **9 个 GCC 源码包（tarball）+ 一份 SHA256SUMS**，合计约 698 MB。
解开的 9 棵树（约 7.0 GB）是派生物，随时可以用 `run.sh` 的 `ensure_src`
从 tarball 重建，因此不长期保留。

```text
SRC_BASE=/opt/src/gcc
  gcc-4.7.4.tar.bz2   92e61c6d…  82935453 B
  gcc-4.8.5.tar.bz2   22fb1e7e…  86165587 B
  gcc-4.9.4.tar.bz2   6c11d292…  90097606 B
  gcc-5.5.0.tar.xz    530cea13…  71096120 B
  gcc-7.5.0.tar.xz    b81946e7…  62783088 B
  gcc-9.5.0.tar.xz    27769f64…  72462752 B
  gcc-12.3.0.tar.xz   949a5d4f…  85491976 B
  gcc-13.3.0.tar.xz   0845e962…  87909952 B
  gcc-14.2.0.tar.xz   a7b39bc6…  92306460 B
  SHA256SUMS                     753 B
```

完整 sha256 见 `tests/gcc-ladder/SHA256SUMS`（已入库），校验：

```sh
cd /opt/src/gcc && sha256sum -c SHA256SUMS
```

## 为什么只留 tarball

| 类别 | 大小 | 能否从别处重建 | 结论 |
| --- | --- | --- | --- |
| 9 个 tarball | 698 MB | 否（上游发布物，是唯一不可推导的输入） | **保留** |
| 9 棵解压树 | 7.0 GB | 是：`tar xf` + `patch_47`/`patch_55`/`fix_ucontext` | 删 |
| build 树 `/opt/gcc-ladder-2026` | 19 GB | 是，但要重跑 10 个 stage、数小时 | 另行决定 |

解压树里没有私货：里面只有上游源码 **加上** 三个由 driver 施加、
本身幂等的改动（见下），所以"tarball + driver"是等价且更小的不变量。
make 是在 `$BUILD_BASE/<stage>` 里做的，源码树里没有构建产物。

## driver 施加到源码树的三处改动

```text
patch_47   4.7.4：cfns-gnu-inline.patch / tcc-dso-handle.patch / tcc-softfp.patch
patch_55   5.5.0：i386.h 去掉 stringop_strategy 的 const
fix_ucontext 4.7.4/4.8.5/4.9.4：linux-unwind.h 的 struct ucontext -> ucontext_t
```

三者都幂等（patch 先 `--dry-run`，sed 重复执行无差别），
所以"树已存在但补丁没打"和"刚解出来"两条路径收敛到同一棵树。

## 复现

```sh
# 1) 只要 tarball（已存在时会跳过下载，之后做 sha256 校验）
sh tests/gcc-ladder/fetch.sh
# 2) 直接跑天梯；缺哪棵树就地从 tarball 解哪棵，解完再打补丁
SRC_BASE=/opt/src/gcc BUILD_BASE=/opt/gcc-ladder-2026 JOBS=6 \
  sh tests/gcc-ladder/run.sh
```

`run.sh` 的 stage 会按 `$LOG/<stage>-hello.out` 跳过已完成阶段，
所以源码树只在真正要编某个 stage 时才会被解出来 —— 已完成的
`/opt/gcc-ladder-2026` 一次都不需要源码树：

```sh
$ BUILD_BASE=/opt/gcc-ladder-2026 sh tests/gcc-ladder/run.sh
PASS: TCC -> GCC 4.7 (2 passes) -> GCC 14.2
```

## 边界

- tarball 本身来自 `ftp.gnu.org`；信任根仍是 GNU 发布物 + 本地 sha256 记录。
  这跟 `seed.elf` 是同一个性质：链上唯一的外部输入需要被记录、校验，而不是被消除。
- 上游只提供 `4.7.4/4.8.5/4.9.4` 的 `.tar.bz2`，5.x 起是 `.tar.xz`；
  `fetch.sh` 两个扩展名都试。
- 若要留一份可离线跑的天梯镜像，应保留 tarball（698 MB）而不是解压树（7.0 GB）。
