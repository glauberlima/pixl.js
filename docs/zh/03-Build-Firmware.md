# 从源代码构建固件

## 概述

本文档介绍如何从源代码构建 pixl.js 固件。构建支持两种硬件变体：**OLED**（SH1106 1.3"，默认）和 **LCD**（ST7735 1.8"）。

固件组件：

- **应用程序** — 主固件，包含所有应用（Amiibo、AmiiboDB、AmiiboLink、Chameleon、Player、Games、Settings）
- **引导加载程序** — 用于 OTA 更新的安全 BLE DFU 引导加载程序
- **SoftDevice** — Nordic S112 v7.2.0 BLE 协议栈（由 nRF5 SDK 提供的二进制文件）

构建由 `fw/Makefile` 编排，使用 GCC + GNU Make。

## 前提条件

| 组件 | 版本 | 必需？ | 来源 |
|---|---|---|---|
| ARM GCC 工具链 | **15.3.rel1** | 是 | [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) |
| nRF5 SDK | **17.1.0_ddde560** | 是 | [Nordic](https://www.nordicsemi.com/Software-and-tools/Software/nRF5-SDK)（免费注册） |
| nrf-command-line-tools | **10.24.2** | 烧录/OTA 需要 | [Nordic](https://www.nordicsemi.com/Products/Development-tools/nrf-command-line-tools) |
| nrfutil | 最新 | OTA 需要 | [Nordic 下载门户](https://files.nordicsemi.com/) |
| Python 3 | 3.8+ | 代码生成需要 | [python.org](https://python.org) |
| GNU Make | 任意版本 | 是 | 大多数 Linux 发行版已预装 |
| Git | 任意版本 | 是 | [git-scm.com](https://git-scm.com) |

## 环境变量

构建系统读取以下环境变量。执行 `source fw/env.sh` 可自动设置它们。

| 变量 | 必需？ | 典型值 | 用途 |
|---|---|---|---|
| `NRF52_SDK_ROOT` | **是** | `$HOME/tools/nRF5_SDK_17.1.0_ddde560` | nRF5 SDK 根目录。两个 Makefile 均使用 `$(SDK_ROOT)` 引用。 |
| `GNU_INSTALL_ROOT` | **是** | `$HOME/tools/arm-gnu-toolchain-15.3.rel1-x86_64-arm-none-eabi/bin/` | ARM GCC 二进制文件路径（末尾带 `/`）。SDK 的 `Makefile.common` 通过此变量找到编译器。 |
| `NRF_COMMAND_LINE_TOOLS_ROOT` | 烧录需要 | `/opt/nrf-command-line-tools` | nrf-command-line-tools 路径。将 `nrfjprog` 和 `mergehex` 添加到 `PATH`。 |
| `PATH`（扩展） | **是** | 包含 `$GNU_INSTALL_ROOT` 和 `$NRF_COMMAND_LINE_TOOLS_ROOT/bin` | 编译器和 Nordic 工具所需。 |

## 方法 1：Docker（推荐）

Docker 镜像 `solosky/nrf52-sdk:latest`（由 `fw/docker/Dockerfile` 构建）已预装所有依赖。

```bash
# 创建容器
docker run -it --rm solosky/nrf52-sdk:latest

# 克隆仓库
git clone https://github.com/solosky/pixl.js
cd pixl.js

# 初始化子模块
git submodule update --init --recursive

# 构建 OLED 版本（推荐）
cd fw && make all BOARD=OLED RELEASE=1

# 或构建 LCD 版本
cd fw && make all BOARD=LCD RELEASE=1
```

## 方法 2：本地构建（Linux）

### 1. 安装 ARM GCC 工具链

```bash
# 下载（如需要请调整 URL）
wget https://developer.arm.com/-/media/Files/downloads/gnu/15.3.rel1/binrel/arm-gnu-toolchain-15.3.rel1-x86_64-arm-none-eabi.tar.xz
sudo tar -xf arm-gnu-toolchain-15.3.rel1-x86_64-arm-none-eabi.tar.xz -C /opt

# 验证
/opt/arm-gnu-toolchain-15.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc --version
```

### 2. 安装 nRF5 SDK

从 Nordic 网站下载 `nRF5_SDK_17.1.0_ddde560.zip`（需要免费注册），然后：

```bash
unzip nRF5_SDK_17.1.0_ddde560.zip -d $HOME/tools/
```

SDK 包含预编译的 micro-ecc 库，无需额外构建步骤。

### 3. 安装可选工具

```bash
# nrf-command-line-tools（用于烧录和 OTA）
# 从 Nordic 下载 .deb 并安装：
sudo dpkg -i nrf-command-line-tools_10.24.2_amd64.deb

# nrfutil（用于 OTA 包生成）
# 从 Nordic 门户下载并放入 PATH
```

### 4. 设置环境并构建

```bash
# 加载环境辅助脚本
source fw/env.sh

# 如果 env.sh 报告缺少依赖，请先安装

# 克隆并构建
git clone https://github.com/solosky/pixl.js
cd pixl.js
git submodule update --init --recursive
cd fw

# 构建 OLED 版本
make all BOARD=OLED RELEASE=1

# 或构建 LCD 版本
make all BOARD=LCD RELEASE=1
```

## 代码生成

部分源文件由 `fw/scripts/` 中的 Python 脚本自动生成。编辑数据源后，运行 `make gen` 重新生成：

| 脚本 | 生成文件 | 数据源 |
|---|---|---|
| `amiibo_db_gen.py` | `amiidb/db_amiibo.c`, `db_game.c`, `db_link.c` | `fw/data/amiidb_*.csv` |
| `i18n_gen.py` | `i18n/*.c`, `i18n/string_id.h` | `fw/data/i18n.csv` |
| `font_data_gen.py` | u8g2 字体 C 文件 | `fw/data/*.bdf` |
| `resource_gen.py` | 应用图标 | `fw/resources/` |

Python 依赖：`pip install -r fw/scripts/requirements.txt`

**不要直接编辑生成的文件。** 请编辑数据源并重新运行 `make gen`。

## 板型变体

| `BOARD=` | 显示屏 | 默认值位置 | 说明 |
|---|---|---|---|
| `OLED` | SH1106 OLED 1.3" | `fw/application/Makefile` | 推荐大多数用户使用 |
| `LCD` | ST7735 LCD 1.8" | `fw/bootloader/Makefile` | 兼容原始 Espruino 硬件 |

**`BOARD` 是编译时常量**（`-DBOARD_$(BOARD)`），非运行时选择。引导加载程序和应用程序必须为同一板型构建。请勿混用 LCD 应用程序和 OLED 引导加载程序。

## 构建输出

所有输出文件位于 `fw/_build/`：

| 文件 | 内容 | 用途 |
|---|---|---|
| `pixljs.hex` | 仅应用程序 | 部分更新、调试 |
| `bootloader.hex` | 安全 BLE DFU 引导加载程序 | 引导加载程序烧录 |
| `pixljs_all.hex` | SoftDevice + 引导加载程序 + 应用程序 | 首次有线烧录 |
| `pixljs_ota_v*.zip` | OTA DFU 包 | 无线更新 |
| `pixljs.out` | 含调试符号的 ELF | 调试 |
| `fw_readme.txt` | 发布说明（中文） | 发布 |
| `fw_update.bat` | Windows 烧录脚本 | 有线烧录 |

## Make 目标

| 目标 | 说明 |
|---|---|
| `make all` | 构建引导加载程序 + 应用程序 + OTA 包 |
| `make bl` | 仅构建引导加载程序 |
| `make app` | 仅构建应用程序 |
| `make ota` | 从现有 hex 生成 OTA DFU 包 |
| `make full` | 合并 softdevice + 引导加载程序 + 应用程序 + 设置到 `pixljs_all.hex` |
| `make version` | 从 git 重新生成 `version.inc.h` |
| `make gen` | 重新生成所有自动生成的源文件（amiibo 数据库、i18n、字体、图标） |
| `make flash_ocd` | 通过 OpenOCD 构建 + 烧录应用程序 |
| `make flash_all_ocd` | 通过 OpenOCD 构建 + 烧录完整镜像 |
| `make clean` | 删除构建产物 |
| `make privgen` | 生成 DFU 签名密钥对 |

## 故障排除

| 现象 | 可能的原因 |
|---|---|
| `arm-none-eabi-gcc: command not found` | `GNU_INSTALL_ROOT` 未设置或不在 `PATH` 中 |
| `fatal error: sdk_config.h: No such file or directory` | `NRF52_SDK_ROOT` 未设置或 SDK 版本错误 |
| `chameleon-ultra/ ... No such file or directory` | 忘记执行 `git submodule update --init --recursive` |
| `make: nrfjprog: Command not found` | nrf-command-line-tools 未安装或不在 `PATH` 中 |
| `mergehex: command not found` | nrf-command-line-tools 未安装 |
| 刷写后屏幕显示异常或空白 | `BOARD` 变体错误（LCD 与 OLED 不匹配） |
| `region FLASH overflowed` | 应用程序超出可用闪存空间 |
