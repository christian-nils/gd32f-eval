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

# Build

```bash
cmake -Bbuild -GNinja && cmake --build build
```

If successfully compiled, you can now flash the MCU:

```bash
st-flash write build/app/gd32f3-eval.bin 0x08000000
```