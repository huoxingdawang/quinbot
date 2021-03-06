$coolqRoot = "D:\CoolQ\CoolQ Pro" # 注意修改 酷Q 目录

$appId = "com.brfish.quinbot"
$libName = "app"
$appOutDir = "E:\quinbot\quinbot\build\Debug\Debug\com.brfish.quinbot"

$coolqAppDevDir = "$coolqRoot\dev\$appId"
$dllName = "$libName.dll"
$dllPath = "$appOutDir\$dllName"
$jsonName = "$libName.json"
$jsonPath = "$appOutDir\$jsonName"

Write-Host "正在拷贝插件到 酷Q 应用文件夹……"

New-Item -Path $coolqAppDevDir -ItemType Directory -ErrorAction SilentlyContinue
Copy-Item -Force $dllPath "$coolqAppDevDir\$dllName"
Copy-Item -Force $jsonPath "$coolqAppDevDir\$jsonName"

Write-Host "拷贝完成" -ForegroundColor Green