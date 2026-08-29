param(
    [ValidateNotNullOrEmpty()]
    [string]$Message = 'Simplify BarCode and add local PNG output'
)

$ErrorActionPreference = 'Stop'
$repositoryRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$uploadLog = @'
版本更新内容：
- 删除 Visual Studio 旧工程、Demo、测试、CI、bindings、third_party 和历史构建产物。
- 项目收敛为单一 CMake 可执行程序和唯一 main.cpp 入口。
- 保留 EAN、UPC、ITF-14、Code39、Code93、Code128、GS1-128 和 Codabar 编码核心。
- 新增 savePng() 本地图片保存接口，支持原子写入 PNG 文件。
- 程序支持通过命令行指定 Code128 数据和输出路径。
- 保留程序实际生成的 barcode.png 作为运行结果。
- 简化 CMake、README、Git 属性和忽略规则。
'@

if ((git -C $repositoryRoot rev-parse --is-inside-work-tree 2>$null) -ne 'true') {
    throw 'The script directory is not a Git repository.'
}

$currentBranch = git -C $repositoryRoot branch --show-current
if ([string]::IsNullOrWhiteSpace($currentBranch)) {
    throw 'The repository is in detached HEAD state. Check out a branch before uploading.'
}

$originUrl = git -C $repositoryRoot remote get-url origin 2>$null
if ([string]::IsNullOrWhiteSpace($originUrl)) {
    throw 'Git remote origin is not configured.'
}

Write-Host 'Upload log:'
Write-Host $uploadLog
Write-Host ''
git -C $repositoryRoot status --short
git -C $repositoryRoot add --all

$stagedChanges = git -C $repositoryRoot diff --cached --name-only
if ([string]::IsNullOrWhiteSpace(($stagedChanges -join ''))) {
    Write-Host 'No changes to upload.'
    exit 0
}

git -C $repositoryRoot commit -m $Message -m $uploadLog
if ($LASTEXITCODE -ne 0) {
    throw 'Git commit failed.'
}

git -C $repositoryRoot push --set-upstream origin $currentBranch
if ($LASTEXITCODE -ne 0) {
    throw 'Git push failed.'
}

Write-Host "Uploaded branch '$currentBranch' to $originUrl"
