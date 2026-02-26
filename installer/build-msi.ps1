param(
    [string]$Version = "",
    [string]$SourceDir = "$PSScriptRoot\\..",
    [string]$OutputMsi = ""
)

$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.Drawing

if ([string]::IsNullOrWhiteSpace($Version)) {
    $now = Get-Date
    $minuteOfDay = ($now.Hour * 60) + $now.Minute
    $Version = "0.2.$($now.DayOfYear).$minuteOfDay"
}

if ([string]::IsNullOrWhiteSpace($OutputMsi)) {
    $buildRoot = Join-Path $PSScriptRoot "..\\built"
    $versionDir = Join-Path $buildRoot $Version
    New-Item -ItemType Directory -Path $versionDir -Force | Out-Null
    $OutputMsi = Join-Path $versionDir "anemo-$Version.msi"
} else {
    $outDir = Split-Path -Path $OutputMsi -Parent
    if (-not [string]::IsNullOrWhiteSpace($outDir)) {
        New-Item -ItemType Directory -Path $outDir -Force | Out-Null
    }
}

if (-not (Get-Command wix -ErrorAction SilentlyContinue)) {
    throw "WiX v4 CLI ('wix') not found. Install WiX Toolset v4 and retry."
}

$resolvedSource = (Resolve-Path $SourceDir).Path
$wxs = Join-Path $PSScriptRoot "anemo.wxs"
$logoPngPath = Join-Path $resolvedSource "installer\\anemo-logo.png"
$iconPath = Join-Path $resolvedSource "installer\\anemo-logo.ico"
$wizardBannerPath = Join-Path $resolvedSource "installer\\wizard-banner.bmp"
$wizardDialogPath = Join-Path $resolvedSource "installer\\wizard-dialog.bmp"

if (-not (Test-Path (Join-Path $resolvedSource "anemo.exe"))) {
    throw "Expected compiler binary at '$resolvedSource\\anemo.exe'. Build it first."
}

if (-not (Test-Path $logoPngPath)) {
    throw "Expected branding image at '$logoPngPath'."
}

function New-BrandedAssetBitmaps {
    param(
        [System.Drawing.Bitmap]$Logo,
        [string]$BannerPath,
        [string]$DialogPath
    )

    $banner = New-Object System.Drawing.Bitmap 493,58
    $bannerG = [System.Drawing.Graphics]::FromImage($banner)
    $bannerRect = New-Object System.Drawing.Rectangle 0,0,493,58
    $bannerBrush = New-Object System.Drawing.Drawing2D.LinearGradientBrush(
        $bannerRect,
        ([System.Drawing.Color]::FromArgb(255, 255, 255)),
        ([System.Drawing.Color]::FromArgb(244, 250, 255)),
        90.0
    )
    $bannerG.FillRectangle($bannerBrush, $bannerRect)
    $bannerG.FillRectangle((New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(23, 133, 205))), 0, 56, 493, 2)
    $banner.Save($BannerPath, [System.Drawing.Imaging.ImageFormat]::Bmp)

    $bannerBrush.Dispose()
    $bannerG.Dispose()
    $banner.Dispose()

    $dialog = New-Object System.Drawing.Bitmap 493,312
    $dialogG = [System.Drawing.Graphics]::FromImage($dialog)
    $dialogRect = New-Object System.Drawing.Rectangle 0,0,493,312
    $dialogBrush = New-Object System.Drawing.Drawing2D.LinearGradientBrush(
        $dialogRect,
        ([System.Drawing.Color]::White),
        ([System.Drawing.Color]::White),
        90.0
    )
    $dialogG.FillRectangle($dialogBrush, $dialogRect)
    $leftPanelRect = New-Object System.Drawing.Rectangle 0,0,164,312
    $leftPanelBrush = New-Object System.Drawing.Drawing2D.LinearGradientBrush(
        $leftPanelRect,
        ([System.Drawing.Color]::FromArgb(243, 250, 255)),
        ([System.Drawing.Color]::FromArgb(226, 242, 255)),
        90.0
    )
    $dialogG.FillRectangle($leftPanelBrush, $leftPanelRect)
    $dialogG.FillRectangle((New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(23, 133, 205))), 163, 0, 1, 312)
    $dialogLogoH = 120
    $dialogLogoW = [int]([Math]::Round($dialogLogoH * ($Logo.Width / [double]$Logo.Height)))
    if ($dialogLogoW -gt 140) {
        $dialogLogoW = 140
    }
    $dialogX = [int]((164 - $dialogLogoW) / 2)
    $dialogY = [int]((312 - $dialogLogoH) / 2)
    $dialogG.DrawImage($Logo, $dialogX, $dialogY, $dialogLogoW, $dialogLogoH)
    $dialog.Save($DialogPath, [System.Drawing.Imaging.ImageFormat]::Bmp)

    $leftPanelBrush.Dispose()
    $dialogBrush.Dispose()
    $dialogG.Dispose()
    $dialog.Dispose()
}

function Ensure-InstallerBrandAssets {
    param(
        [string]$PngPath,
        [string]$IcoPath,
        [string]$BannerPath,
        [string]$DialogPath
    )

    $logo = [System.Drawing.Bitmap]::FromFile($PngPath)
    try {
        $header = @()
        if (Test-Path $IcoPath) {
            $header = Get-Content -Path $IcoPath -Encoding Byte -TotalCount 4
        }

        $validIco = $header.Length -ge 4 -and $header[0] -eq 0 -and $header[1] -eq 0 -and $header[2] -eq 1 -and $header[3] -eq 0
        if (-not $validIco) {
            $canvas = [System.Drawing.Bitmap]::new(256, 256, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
            $g = [System.Drawing.Graphics]::FromImage($canvas)
            $g.Clear([System.Drawing.Color]::Transparent)
            $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
            $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality
            $ratio = [Math]::Min(256.0 / $logo.Width, 256.0 / $logo.Height)
            $w = [int]([Math]::Round($logo.Width * $ratio))
            $h = [int]([Math]::Round($logo.Height * $ratio))
            $x = [int]((256 - $w) / 2)
            $y = [int]((256 - $h) / 2)
            $g.DrawImage($logo, $x, $y, $w, $h)
            $icon = [System.Drawing.Icon]::FromHandle($canvas.GetHicon())
            $fs = [System.IO.File]::Create($IcoPath)
            $icon.Save($fs)
            $fs.Close()
            $icon.Dispose()
            $g.Dispose()
            $canvas.Dispose()
        }

        New-BrandedAssetBitmaps -Logo $logo -BannerPath $BannerPath -DialogPath $DialogPath
    } finally {
        $logo.Dispose()
    }
}

Ensure-InstallerBrandAssets `
    -PngPath $logoPngPath `
    -IcoPath $iconPath `
    -BannerPath $wizardBannerPath `
    -DialogPath $wizardDialogPath

wix build $wxs `
    -ext WixToolset.UI.wixext `
    -d SourceDir=$resolvedSource `
    -d ProductVersion=$Version `
    -o $OutputMsi

Write-Host "MSI generated at: $OutputMsi"
