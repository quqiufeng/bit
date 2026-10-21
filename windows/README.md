# Windows reference

The Windows x86-64 implementation is the upstream BitKindle tree, pinned in
`upstream.lock` and vendored under `upstream/bootstrap-c99` for reference and
future M4 work.

Rules:

- The Linux build must never include or link this tree.
- The Windows tree keeps its own PE/.c1o object format, custom PE linker,
  CRT/runtime, kernel32 imports and PowerShell driver.
- No COFF, `.lib`, MinGW CRT or Windows SDK headers are integrated.
- Upstream MIT license text is in `upstream/LICENSE`; project-level attribution
  is in the repository `NOTICE`.

Pinned upstream commit: `4527957e530d193ad5a81f7af9afeba1d1d0634f`.

Upstream build entry (requires Windows + PowerShell):

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File windows/upstream/bootstrap-c99/build.ps1
```
