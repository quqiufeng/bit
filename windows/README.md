# Windows reference

The Windows x86-64 implementation is the upstream BitKindle tree, pinned in
`upstream.lock`.  It is deliberately not vendored yet: the Linux port must not
build or link any part of it, and the Windows tree must retain its own PE/.c1o,
CRT and PowerShell driver.

Current state:

- Linux side is independent and self-hosting.
- Windows reference is frozen by commit `4527957e` in `upstream.lock`.
- No Windows object, PE, COFF, `.lib` or MinGW CRT is accepted by the Linux build.
- Future M4 work imports the pinned tree under this directory, then adds the
  Windows ABI/PE integration tests without changing the Linux ABI.

Upstream build entry (after importing/fetching the pinned tree):

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File src/bootstrap-c99/build.ps1
```
