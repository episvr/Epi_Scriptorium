<#
.SYNOPSIS
环境变量路径检测脚本 - 用户/系统双模式
.DESCRIPTION
同时检查用户级和系统级PATH环境变量，生成分类统计报告
#>

# 配置参数
$logFile = "D:\path_check_log.txt"
$showConsole = $true

# 初始化统计对象
$globalStats = [PSCustomObject]@{
    User = [PSCustomObject]@{
        Total = 0
        Valid = 0
        Invalid = 0
        Empty = 0
        Errors = 0
    }
    System = [PSCustomObject]@{
        Total = 0
        Valid = 0
        Invalid = 0
        Empty = 0
        Errors = 0
    }
}

# 创建日志头
$header = @"

========================================
  双环境变量检测报告
  生成时间: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
========================================
"@
Add-Content -Path $logFile -Value $header

function Process-Paths {
    param(
        [ValidateSet("User","System")]
        $targetType,
        [System.EnvironmentVariableTarget]$target
    )

    try {
        $envPaths = [System.Environment]::GetEnvironmentVariable("PATH", $target)
        $paths = $envPaths.Split(";", [System.StringSplitOptions]::RemoveEmptyEntries)
        $globalStats.$targetType.Total = $paths.Count
        
        Add-Content -Path $logFile -Value "`n=== $targetType PATH 检测开始 ==="
        if($showConsole) { Write-Host "`n正在检测 $targetType 环境变量..." -ForegroundColor Cyan }

        foreach ($path in $paths) {
            try {
                if ([string]::IsNullOrWhiteSpace($path)) {
                    $globalStats.$targetType.Empty++
                    Add-Content -Path $logFile -Value "$(Get-Date -Format "HH:mm:ss") - [$targetType][空路径]"
                    if($showConsole) { Write-Host "  [$targetType] 发现空路径" -ForegroundColor Yellow }
                    continue
                }

                $exists = Test-Path -Path $path -ErrorAction Stop
                if ($exists) {
                    $globalStats.$targetType.Valid++
                    Add-Content -Path $logFile -Value "$(Get-Date -Format "HH:mm:ss") - [$targetType][有效] $path"
                    if($showConsole) { Write-Host "  [$targetType][√] $path" -ForegroundColor Green }
                }
                else {
                    $globalStats.$targetType.Invalid++
                    Add-Content -Path $logFile -Value "$(Get-Date -Format "HH:mm:ss") - [$targetType][无效] $path"
                    if($showConsole) { Write-Host "  [$targetType][×] $path" -ForegroundColor Red }
                }
            }
            catch {
                $globalStats.$targetType.Errors++
                Add-Content -Path $logFile -Value "$(Get-Date -Format "HH:mm:ss") - [$targetType][错误] $path - $_"
                if($showConsole) { Write-Host "  [$targetType][!] 检测错误: $path ($_)" -ForegroundColor DarkYellow }
            }
        }
    }
    catch {
        Add-Content -Path $logFile -Value "$(Get-Date -Format "HH:mm:ss") - [错误] 无法读取$targetType PATH变量: $_"
        if($showConsole) { Write-Host "读取$targetType PATH失败: $_" -ForegroundColor Red }
    }
}

# 先检测用户变量再检测系统变量
Process-Paths -targetType "User" -target User
Process-Paths -targetType "System" -target Machine

# 生成统计报告
$report = @"

========================================
用户环境变量统计：
  总路径数  : $($globalStats.User.Total)
  有效路径  : $($globalStats.User.Valid) (绿色)
  无效路径  : $($globalStats.User.Invalid) (红色)
  检测错误  : $($globalStats.User.Errors) (黄色)
  空路径    : $($globalStats.User.Empty)

系统环境变量统计：
  总路径数  : $($globalStats.System.Total)
  有效路径  : $($globalStats.System.Valid) (绿色)
  无效路径  : $($globalStats.System.Invalid) (红色)
  检测错误  : $($globalStats.System.Errors) (黄色)
  空路径    : $($globalStats.System.Empty)

合并统计：
  总路径数  : $($globalStats.User.Total + $globalStats.System.Total)
  有效路径  : $($globalStats.User.Valid + $globalStats.System.Valid)
  无效路径  : $($globalStats.User.Invalid + $globalStats.System.Invalid)
========================================
"@

# 输出结果
Add-Content -Path $logFile -Value $report
if($showConsole) {
    Write-Host $report -ForegroundColor Cyan
    Write-Host "检测完成，日志已保存至: $logFile" -ForegroundColor White
}