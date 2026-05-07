# DeepSeek Balance Plugin for TrafficMonitor

在 TrafficMonitor 任务栏/主窗口中显示 DeepSeek API 账户余额。

## 功能

- 定时查询 DeepSeek API 账户余额并显示在 TrafficMonitor 中
- 显示格式：`DS 8.87 CNY`（标签 + 余额 + 货币单位）
- 鼠标悬停显示详细信息：总额、充值金额、赠送金额、账户状态、更新时间
- 可配置 API Token、API URL、刷新间隔
- 支持测试连接功能
- 配置存储在 INI 文件中

## 编译方法（3 种方式，任选其一）

### 方法一：Visual Studio 2022（有 VS 的情况下）

1. 双击打开 `DeepSeekBalancePlugin.vcxproj`
2. 选择 Release/x64
3. 生成解决方案

编译后的 DLL 在 `x64/Release/DeepSeekBalancePlugin.dll`

### 方法二：MSVC Build Tools（免费，命令行，~500MB）

> 适合没有 Visual Studio IDE 但愿意装编译器的情况

1. 下载安装 **Visual Studio 2022 Build Tools**：
   https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022
   - 在安装程序中勾选 "Desktop development with C++"
   - 或者只选：MSVC v143 生成工具 + Windows 10/11 SDK

2. 打开 **Developer Command Prompt for VS 2022**（开始菜单里搜索）
   或者手动运行环境初始化：
   ```
   "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
   ```

3. 进入插件目录，运行：
   ```
   build_msvc.bat
   ```
   输出：`DeepSeekBalancePlugin.dll`

### 方法三：MinGW-w64（免费，轻量，~200MB）

> 适合不想装微软编译器、或者用 MSYS2 的情况

#### Windows (MSYS2)

1. 安装 [MSYS2](https://www.msys2.org/)
2. 打开 MSYS2 UCRT64 终端，安装工具链：
   ```
   pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-winpthreads
   ```

3. 进入插件目录，运行：
   ```
   make
   ```
   输出：`DeepSeekBalancePlugin.dll`

#### Linux / WSL2（交叉编译）

```bash
# 安装 MinGW-w64 交叉编译器
sudo apt install mingw-w64

# 编译
make CROSS=1

# 输出：DeepSeekBalancePlugin.dll（Windows DLL）
```

## 安装

1. 将 `DeepSeekBalancePlugin.dll` 复制到 TrafficMonitor 安装目录下的 `plugins/` 文件夹
2. 重启 TrafficMonitor
3. 在 TrafficMonitor 的"选项"→"插件管理"中启用 DeepSeek Balance 插件
4. 右键插件 → "选项" → 输入你的 DeepSeek API Token

## 配置

| 设置项 | 说明 | 默认值 |
|--------|------|--------|
| API Token | DeepSeek API 密钥 | (空) |
| API URL | DeepSeek 余额查询 API 地址 | `https://api.deepseek.com/user/balance` |
| 刷新间隔 | 余额数据刷新间隔（分钟） | 30 分钟 |

## API 响应格式

插件期望的 API 响应格式：

```json
{
  "is_available": true,
  "balance_infos": [
    {
      "currency": "CNY",
      "total_balance": "8.87",
      "granted_balance": "0.00",
      "topped_up_balance": "8.87"
    }
  ]
}
```

## 文件结构

```
DeepSeekBalancePlugin/
├── PluginInterface.h                # TrafficMonitor 插件接口定义
├── Platform.h                       # 编译器兼容层（MSVC/MinGW）
├── DataManager.h / .cpp            # 数据管理（配置、HTTP 请求、JSON 解析）
├── DeepSeekBalanceItem.h / .cpp    # 显示项目
├── DeepSeekBalancePlugin.h / .cpp  # 主插件类
├── OptionsDlg.h / .cpp             # 选项设置对话框
├── dllmain.cpp                     # DLL 入口
├── resource.h                      # 资源定义
├── DeepSeekBalancePlugin.rc        # 资源文件（对话框模板、版本信息）
├── DeepSeekBalancePlugin.vcxproj   # Visual Studio 项目文件
├── DeepSeekBalancePlugin.vcxproj.filters
├── Makefile                        # MinGW-w64 编译（MSYS2/Linux）
└── build_msvc.bat                  # MSVC Build Tools 编译脚本
```

## 插件接口

- API 版本: 7
- 无需 MFC 运行时依赖
- 使用 WinHTTP 进行 HTTPS 请求
- 手动 JSON 解析（无外部依赖）
