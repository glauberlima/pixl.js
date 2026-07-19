#!/usr/bin/env bash
# pixl.js firmware build environment
# Usage: source fw/env.sh

set -e

GNU_INSTALL_ROOT="$HOME/tools/arm-gnu-toolchain-15.3.rel1-x86_64-arm-none-eabi/bin/"
if [ ! -d "$GNU_INSTALL_ROOT" ]; then
    echo "ERROR: ARM GCC toolchain not found at $GNU_INSTALL_ROOT"
    echo "Download: https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads"
    return 1
fi

NRF52_SDK_ROOT="$HOME/tools/nRF5_SDK_17.1.0_ddde560"
if [ ! -d "$NRF52_SDK_ROOT" ]; then
    echo "ERROR: nRF5 SDK not found at $NRF52_SDK_ROOT"
    echo "Download nRF5 SDK 17.1.0 from nordicsemi.com (free registration required)"
    return 1
fi

export GNU_INSTALL_ROOT NRF52_SDK_ROOT

# nrf-command-line-tools (optional: nrfjprog, mergehex)
NRF_CLT="/opt/nrf-command-line-tools"
if [ -d "$NRF_CLT/bin" ]; then
    export NRF_COMMAND_LINE_TOOLS_ROOT="$NRF_CLT"
    export PATH="$NRF_CLT/bin:$PATH"
fi

export PATH="$GNU_INSTALL_ROOT:$HOME/tools/bin:$PATH"

echo "pixl.js env: OK"
echo "  Toolchain: $(arm-none-eabi-gcc --version 2>&1 | head -1)"
echo "  SDK:       $NRF52_SDK_ROOT"
command -v mergehex >/dev/null 2>&1 && echo "  mergehex:  $(mergehex --version 2>&1 | head -1)"
command -v nrfjprog >/dev/null 2>&1 && echo "  nrfjprog:  $(nrfjprog --version 2>&1 | head -1)"
command -v nrfutil >/dev/null 2>&1 && echo "  nrfutil:   $(nrfutil --version 2>&1 | tail -1)"
