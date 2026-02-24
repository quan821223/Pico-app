@echo off
chcp 65001 > nul
setlocal enabledelayedexpansion

:: ===========================================
:: Raspberry Pi Pico 自動編譯腳本
:: ===========================================

title Pico 自動編譯工具

:: 設定顏色 (綠色文字)
color 0A

echo.
echo ========================================
echo   Raspberry Pi Pico 自動編譯工具
echo ========================================
echo.

:: 檢查是否在正確的專案目錄
if not exist "CMakeLists.txt" (
    color 0C
    echo [錯誤] 找不到 CMakeLists.txt
    echo 請確認您在 Pico 專案根目錄執行此腳本
    pause
    exit /b 1
)

:: 步驟 1: 清除舊的 build 資料夾
echo [步驟 1/4] 清除舊的 build 資料夾...
if exist "build" (
    echo 正在刪除 build 資料夾...
    rmdir /s /q build
    if errorlevel 1 (
        color 0C
        echo [錯誤] 無法刪除 build 資料夾
        pause
        exit /b 1
    )
    echo 刪除完成
) else (
    echo build 資料夾不存在，跳過
)
echo.

:: 步驟 2: 建立新的 build 資料夾
echo [步驟 2/4] 建立新的 build 資料夾...
mkdir build
if errorlevel 1 (
    color 0C
    echo [錯誤] 無法建立 build 資料夾
    pause
    exit /b 1
)
echo 建立完成
echo.

:: 步驟 3: 執行 CMake 設定
echo [步驟 3/4] 執行 CMake 設定...
cd build
cmake -G "NMake Makefiles" ..
if errorlevel 1 (
    color 0C
    echo [錯誤] CMake 設定失敗
    cd ..
    pause
    exit /b 1
)
echo CMake 設定完成
echo.

:: 步驟 4: 編譯專案
echo [步驟 4/4] 開始編譯...
nmake
if errorlevel 1 (
    color 0C
    echo [錯誤] 編譯失敗
    cd ..
    pause
    exit /b 1
)
echo.

:: 返回專案根目錄
cd ..

:: 編譯成功
color 0A
echo ========================================
echo   編譯成功！
echo ========================================
echo.

:: 尋找並顯示生成的 .uf2 檔案
echo 尋找編譯結果...
for /r build %%f in (*.uf2) do (
    echo [成功] 找到 UF2 檔案：
    echo    %%f
    echo.
    
    :: 詢問是否複製到當前目錄
    set /p copy_choice=是否複製 UF2 檔案到當前目錄？(Y/N): 
    if /i "!copy_choice!"=="Y" (
        copy "%%f" .
        if errorlevel 1 (
            echo [警告] 複製失敗
        ) else (
            echo [成功] 已複製到當前目錄
        )
    )
    echo.
    goto found_uf2
)

echo [警告] 未找到 .uf2 檔案
echo 請檢查 build 目錄中的編譯結果
echo.

:found_uf2

:: 顯示編譯資訊
echo ========================================
echo 編譯資訊：
echo ----------------------------------------
echo 專案目錄：%CD%
echo 建置目錄：%CD%\build
echo 完成時間：%date% %time%
echo ========================================
echo.

:: 詢問是否開啟 build 資料夾
set /p open_choice=是否開啟 build 資料夾？(Y/N): 
if /i "%open_choice%"=="Y" (
    start explorer build
)

echo.
echo 按任意鍵結束...
pause > nul