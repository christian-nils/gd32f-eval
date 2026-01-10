# Prerequisites

Install cross-compiler

```bash

sudo dnf update -y
sudo dnf install arm-none-eabi-gcc-cs -y
sudo dnf install arm-none-eabi-gcc-cs-c++ -y
sudo dnf install arm-none-eabi-newlib -y
sudo dnf install gcc-aarch64-linux-gnu -y
sudo dnf install gcc-arm-linux-gnu -y
sudo dnf install openocd -y

```
I you plan to run the micro-ros (ROS2 kilted) example, you need to install the dependencies:

```bash
python -m pip install -r requirements.txt
```

# Build

```bash
cmake -Bbuild -GNinja -DGD32F3_APP=USART/printf && cmake --build build 
```

Replace `USART/printf` with the actual app you want to compile (do the same below when flashing).

If successfully compiled, you can now flash the MCU:

```bash
st-flash write build/app/USART/printf/gd32f3-eval.bin 0x08000000
```

# Debug

Use the debug launch (called `Debug with OpenOCD`) in VS Code to do the compilation and the flashing. You need to edit the `.env` file with the app you want to debug.