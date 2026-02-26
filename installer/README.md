# Windows MSI Installer

This installer packages `anemo.exe` with Anemo branding, EULA, Start Menu shortcut, and PATH updates.

## Install Location

By default the MSI installs to:

- `C:\Program Files\Anemo`

It also appends that install directory to the system `PATH`.

## Prerequisites

- WiX Toolset v4 (`wix` CLI on PATH)
- A Windows build of the compiler binary named `anemo.exe`
- Branding assets in this folder (`anemo-logo.png`, optional `anemo-logo.ico`)

## Build MSI

From PowerShell in the repository root:

```powershell
.\installer\build-msi.ps1
```

Output is created under:

- `built/<version>/anemo-<version>.msi`

You can still override version/source/output:

```powershell
.\installer\build-msi.ps1 -Version 0.1.0 -SourceDir . -OutputMsi .\built\0.1.0\anemo-0.1.0.msi
```

## Install

```powershell
msiexec /i .\anemo-<version>.msi
```

If double-clicking closes immediately, install from an elevated PowerShell/CMD:

```powershell
msiexec /i .\anemo-<version>.msi /qb /l*v .\installer\msi-install.log
```

This writes a verbose log to help diagnose installer issues.

## Uninstall

Use Windows "Add or remove programs" or:

```powershell
msiexec /x .\anemo-<version>.msi
```
