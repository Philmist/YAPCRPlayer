#Requires -Version 7.0
<#
.SYNOPSIS
    YAPCRPlayer の Release 配布物をビルドし、zip に圧縮して SHA-256 を併出する。

.DESCRIPTION
    CMake Release プリセット(msvc-ninja-release)で構成・ビルドし、deploy ターゲットで
    windeployqt + libmpv-2.dll + ライセンス一式を含む dist/ を生成する。その dist/ を
    バージョン付きフォルダ(YAPCRPlayer-v<ver>-win-x64)へステージし(.pdb/.ilk/.exp は除外)、
    Compress-Archive で zip 化、Get-FileHash で <zip>.sha256 を出力する。

    成果物: build/release-artifacts/YAPCRPlayer-v<ver>-win-x64.zip (+ .sha256)

.NOTES
    - MSVC + Ninja の Native Tools 環境(= vcvars64 読込済み)で実行すること。
      cl/ninja が PATH に無ければ停止する(docs/build.md §3 参照)。
    - libmpv dev 未取得なら先に scripts/fetch-mpv-dev.ps1 を実行すること。
    - GPLv3 配布物には LICENSE / CORRESPONDING-SOURCE.md / licenses/ が deploy で同梱される。

.PARAMETER Version
    成果物名に使うバージョン。既定は CMakeLists.txt の project(... VERSION x.y.z) を読む。

.PARAMETER SkipBuild
    構成・ビルド・deploy を省略し、既存の build/msvc-ninja-release/dist/ を再パッケージする。

.PARAMETER Force
    出力先に同名 zip / ステージフォルダがあっても上書きする。
#>
[CmdletBinding()]
param(
    [string]$Version,
    [switch]$SkipBuild,
    [switch]$Force
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$RepoRoot     = Split-Path -Parent $PSScriptRoot
# cmake --preset は CMakePresets.json を CWD から読むため、呼び出し元 CWD に依存しないよう
# リポジトリルートへ移動する。
Set-Location $RepoRoot
$Preset       = 'msvc-ninja-release'
$BuildDir     = Join-Path $RepoRoot 'build\msvc-ninja-release'
$DistDir      = Join-Path $BuildDir 'dist'
$ArtifactsDir = Join-Path $RepoRoot 'build\release-artifacts'

function Write-Step([string]$msg) { Write-Host "==> $msg" -ForegroundColor Cyan }
function Write-Info([string]$msg) { Write-Host "    $msg" -ForegroundColor DarkGray }

# ---- バージョン決定（未指定なら CMakeLists.txt から） ----
if (-not $Version) {
    $cmake = Get-Content (Join-Path $RepoRoot 'CMakeLists.txt') -Raw
    if ($cmake -match 'project\s*\(\s*YAPCRPlayer[\s\S]*?VERSION\s+([0-9]+\.[0-9]+\.[0-9]+)') {
        $Version = $Matches[1]
    } else {
        throw 'CMakeLists.txt から VERSION を取得できませんでした。-Version で指定してください。'
    }
}
$PkgName = "YAPCRPlayer-v$Version-win-x64"
Write-Step "パッケージ: $PkgName"

# ---- ビルド～deploy ----
if (-not $SkipBuild) {
    if (-not (Get-Command cl -ErrorAction SilentlyContinue)) {
        throw @'
cl (MSVC コンパイラ) が PATH にありません。x64 Native Tools / vcvars64 を読み込んでから実行してください。
例: & "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
詳細は docs/build.md §3 を参照。
'@
    }

    Write-Step "構成 (cmake --preset $Preset)"
    cmake --preset $Preset
    if ($LASTEXITCODE -ne 0) { throw "cmake configure に失敗しました (exit $LASTEXITCODE)" }

    Write-Step "ビルド (cmake --build --preset $Preset)"
    cmake --build --preset $Preset
    if ($LASTEXITCODE -ne 0) { throw "cmake build に失敗しました (exit $LASTEXITCODE)" }

    Write-Step "配布物生成 (deploy → dist/)"
    cmake --build --preset $Preset --target deploy
    if ($LASTEXITCODE -ne 0) { throw "deploy に失敗しました (exit $LASTEXITCODE)" }
} else {
    Write-Step 'ビルドをスキップ（既存 dist/ を再パッケージ）'
}

if (-not (Test-Path (Join-Path $DistDir 'YAPCRPlayer.exe'))) {
    throw "dist/ に YAPCRPlayer.exe がありません: $DistDir （先にビルドを実行してください）"
}

# ---- ステージング（.pdb/.ilk/.exp を除外して複製） ----
New-Item -ItemType Directory -Force -Path $ArtifactsDir | Out-Null
$StageDir = Join-Path $ArtifactsDir $PkgName
$ZipPath  = Join-Path $ArtifactsDir "$PkgName.zip"

foreach ($p in @($StageDir, $ZipPath, "$ZipPath.sha256")) {
    if (Test-Path $p) {
        if (-not $Force) { throw "既に存在します: $p （上書きするには -Force）" }
        Remove-Item $p -Recurse -Force
    }
}

Write-Step "ステージング ($PkgName)"
New-Item -ItemType Directory -Force -Path $StageDir | Out-Null
$exclude = @('*.pdb', '*.ilk', '*.exp')
Copy-Item -Path (Join-Path $DistDir '*') -Destination $StageDir -Recurse -Force -Exclude $exclude
# サブディレクトリに残った除外対象も掃除する
Get-ChildItem $StageDir -Recurse -Include $exclude -File | Remove-Item -Force
$fileCount = (Get-ChildItem $StageDir -Recurse -File | Measure-Object).Count
Write-Info "$fileCount ファイルをステージしました"

# ---- 圧縮 ----
Write-Step "圧縮 (Compress-Archive → $($ZipPath | Split-Path -Leaf))"
Compress-Archive -Path $StageDir -DestinationPath $ZipPath -CompressionLevel Optimal -Force

# ---- SHA-256 ----
Write-Step 'SHA-256 を計算'
$hash = (Get-FileHash -Algorithm SHA256 -Path $ZipPath).Hash.ToLower()
$shaFile = "$ZipPath.sha256"
"$hash  $PkgName.zip" | Set-Content -Path $shaFile -Encoding ascii -NoNewline

# ---- 結果表示 ----
$zipSize = '{0:N1} MB' -f ((Get-Item $ZipPath).Length / 1MB)
Write-Host ''
Write-Step '完了'
Write-Info "zip   : $ZipPath ($zipSize)"
Write-Info "sha256: $shaFile"
Write-Info "SHA256: $hash"
