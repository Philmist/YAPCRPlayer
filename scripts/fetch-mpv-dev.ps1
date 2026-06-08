#Requires -Version 7.0
<#
.SYNOPSIS
    libmpv の dev パッケージ（ヘッダ＋ランタイム DLL）を取得し、MSVC 用 import lib を生成する。

.DESCRIPTION
    zhongfly/mpv-winbuild の最新リリースから GPL ビルドの mpv-dev-x86_64-*.7z を取得し、
    third_party/mpv-dev/ へ展開する。展開後、mpv.def から lib.exe で mpv.lib（MSVC import lib）を生成する。
    生成物:
      third_party/mpv-dev/include/mpv/client.h   … ヘッダ
      third_party/mpv-dev/libmpv-2.dll           … ランタイム（配布時に exe へ同梱）
      third_party/mpv-dev/mpv.lib                … MSVC リンク用 import lib（CMake が参照）

    冪等。既に client.h と mpv.lib があれば何もしない（-Force で再取得）。

.NOTES
    展開はこの環境に 7z.exe が無いため Windows 同梱の tar(bsdtar) を使う（bsdtar は 7z 書庫に対応）。
    7z.exe があればそちらを優先。Expand-Archive は .zip 専用のため .7z には使えない。
#>
[CmdletBinding()]
param(
    [switch]$Force
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$RepoRoot   = Split-Path -Parent $PSScriptRoot
$ThirdParty = Join-Path $RepoRoot 'third_party'
$DevDir     = Join-Path $ThirdParty 'mpv-dev'
$Header     = Join-Path $DevDir 'include\mpv\client.h'
$ImportLib  = Join-Path $DevDir 'mpv.lib'

function Write-Step([string]$msg) { Write-Host "==> $msg" -ForegroundColor Cyan }

# ---- 冪等チェック ----
if (-not $Force -and (Test-Path $Header) -and (Test-Path $ImportLib)) {
    Write-Step "既に取得済み: $DevDir （再取得は -Force）"
    return
}

New-Item -ItemType Directory -Force -Path $ThirdParty | Out-Null

# ---- 最新リリースの mpv-dev-x86_64（GPL/汎用）資産を解決 ----
Write-Step '最新リリース情報を取得中 (zhongfly/mpv-winbuild)'
$headers = @{ 'User-Agent' = 'YAPCRPlayer-fetch-mpv-dev' }
if ($env:GITHUB_TOKEN) { $headers['Authorization'] = "Bearer $env:GITHUB_TOKEN" }
$release = Invoke-RestMethod -Headers $headers `
    -Uri 'https://api.github.com/repos/zhongfly/mpv-winbuild/releases/latest'

# lgpl / v3(AVX2必須) / aarch64 を除外した汎用 x86_64 GPL ビルド
$asset = $release.assets |
    Where-Object { $_.name -match '^mpv-dev-x86_64-\d+-git-[0-9a-f]+\.7z$' } |
    Select-Object -First 1
if (-not $asset) {
    throw "mpv-dev-x86_64 資産が見つからない。リリース: $($release.tag_name)"
}
Write-Step "対象: $($asset.name)  ($([math]::Round($asset.size/1MB,1)) MB)"

# ---- ダウンロード ----
$archive = Join-Path $ThirdParty $asset.name
if ($Force -or -not (Test-Path $archive)) {
    Write-Step "ダウンロード中 -> $archive"
    Invoke-WebRequest -Headers $headers -Uri $asset.browser_download_url -OutFile $archive
} else {
    Write-Step "書庫は取得済み: $archive"
}

# ---- 展開 ----
if (Test-Path $DevDir) { Remove-Item -Recurse -Force $DevDir }
New-Item -ItemType Directory -Force -Path $DevDir | Out-Null

$sevenZip = Get-Command '7z.exe' -ErrorAction SilentlyContinue
if (-not $sevenZip) {
    $candidate = 'C:\Program Files\7-Zip\7z.exe'
    if (Test-Path $candidate) { $sevenZip = Get-Item $candidate }
}

if ($sevenZip) {
    Write-Step "展開中 (7z): $($sevenZip.Source)"
    & $sevenZip.Source x -y "-o$DevDir" $archive | Out-Null
    if ($LASTEXITCODE -ne 0) { throw "7z 展開に失敗 (exit $LASTEXITCODE)" }
} else {
    $tar = Get-Command 'tar.exe' -ErrorAction SilentlyContinue
    if (-not $tar) {
        throw "7z.exe も tar.exe も見つからない。7-Zip を導入するか Windows 同梱の tar を有効にしてください。"
    }
    Write-Step "展開中 (tar/bsdtar): $($tar.Source)"
    & $tar.Source -xf $archive -C $DevDir
    if ($LASTEXITCODE -ne 0) { throw "tar 展開に失敗 (exit $LASTEXITCODE)。bsdtar が 7z 非対応の可能性。" }
}

if (-not (Test-Path $Header)) {
    throw "展開後に $Header が無い。書庫構成が想定と異なる可能性。"
}

# ---- MSVC import lib を生成 ----
# この dev 書庫には MinGW 用 libmpv.dll.a のみで mpv.def が無いため、
# dumpbin /EXPORTS で DLL のエクスポート表から .def を起こし、lib /def で MSVC 用 mpv.lib を作る。
$dll = Join-Path $DevDir 'libmpv-2.dll'
if (-not (Test-Path $dll)) { throw "libmpv-2.dll が無い: $dll" }
$def = Join-Path $DevDir 'mpv.def'

Write-Step 'MSVC ツールチェーン(dumpbin/lib)を解決中'
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path $vswhere)) { throw "vswhere.exe が無い: $vswhere（VS2022 が必要）" }
$vsPath = & $vswhere -latest -products * `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -property installationPath
if (-not $vsPath) { throw 'C++ ツール付き VS インスタンスが見つからない' }
$vcvars = Join-Path $vsPath 'VC\Auxiliary\Build\vcvars64.bat'
if (-not (Test-Path $vcvars)) { throw "vcvars64.bat が無い: $vcvars" }

Write-Step 'DLL エクスポートから mpv.def を生成中'
$exportsTxt = Join-Path $DevDir 'exports.txt'
cmd.exe /c "`"$vcvars`" && dumpbin /nologo /exports `"$dll`" > `"$exportsTxt`""
if ($LASTEXITCODE -ne 0) { throw "dumpbin に失敗 (exit $LASTEXITCODE)" }

# dumpbin /exports の表本体: "   ordinal hint RVA name" 形式。RVA 列(16進)と関数名を持つ行のみ採用。
$names = foreach ($line in Get-Content $exportsTxt) {
    if ($line -match '^\s+\d+\s+[0-9A-Fa-f]+\s+[0-9A-Fa-f]{8}\s+(\S+)') { $Matches[1] }
}
if (-not $names) { throw 'dumpbin 出力からエクスポート名を抽出できなかった' }
Write-Step "エクスポート数: $($names.Count)"
@('EXPORTS') + $names | Set-Content -Path $def -Encoding ascii
Remove-Item $exportsTxt -ErrorAction SilentlyContinue

Write-Step "import lib を生成中 -> $ImportLib"
$cmd = "`"$vcvars`" && lib /nologo /def:`"$def`" /name:libmpv-2.dll /machine:x64 /out:`"$ImportLib`""
cmd.exe /c $cmd
if ($LASTEXITCODE -ne 0) { throw "lib.exe による import lib 生成に失敗 (exit $LASTEXITCODE)" }
if (-not (Test-Path $ImportLib)) { throw "生成されたはずの $ImportLib が無い" }

Write-Step '完了'
Write-Host "  ヘッダ      : $Header"
Write-Host "  import lib  : $ImportLib"
Write-Host "  ランタイム  : $(Join-Path $DevDir 'libmpv-2.dll')"
