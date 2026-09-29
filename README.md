SETUP:
 
much of the methodology of this setup comes from the recommended method of this source: https://siliconwit.com/education/embedded-programming-atmega328p/avr-toolchain-bare-metal-setup/#project-structure


1. Clone repository
Into a bash terminal:
git clone https://github.com/ENGR290-Team13/Firmware.git


2. Install AVR-GCC / avr-libc / avrdude
<!-- ================= AI-GENERATED START ======================= -->
This section was drafted with AI assistance. a chatgpt made summary of how we set it up on my pc using MSY2 UCRT64, this way we develop here without the need for the arduino software, though one of us will verify our code works there too before submission. 

## AVR Development Environment Setup

Have **MSYS2 UCRT64** installed first. This is used to install the AVR compiler, AVR libraries, AVRDUDE, and Make.

### 1. Fully update MSYS2 UCRT64

Open the **MSYS2 UCRT64** terminal and run:

```bash
pacman -Syu
```

MSYS2 may update its core packages and tell you that the terminal needs to close.

If it closes, reopen **MSYS2 UCRT64** and run:

```bash
pacman -Syu
```

again.

Repeat this process until `pacman -Syu` completes normally without requiring the terminal to close.

Do **not** try to manually remove old GCC libraries if dependency conflicts appear. Fully updating MSYS2 first should resolve these conflicts.

---

### 2. Install the AVR toolchain

From the **MSYS2 UCRT64** terminal, run:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-avr-toolchain mingw-w64-ucrt-x86_64-avrdude
```

When prompted to select members of the AVR toolchain group, press **Enter** to install all of them.

This installs the tools used by the firmware build system, including:

```text
avr-gcc
avr-libc
avr-objcopy
avr-size
avrdude
```

Verify that AVR GCC works:

```bash
avr-gcc --version
```

Verify AVRDUDE:

```bash
avrdude -?
```

---

### 3. Install Make

Still inside **MSYS2 UCRT64**, install Make:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-make
```

The UCRT64 version of Make may be installed under the command:

```bash
mingw32-make
```

Verify it:

```bash
mingw32-make --version
```

---

### 4. Make the AVR tools available inside Git Bash

The project can be developed from **Git Bash / MINGW64** instead of requiring the UCRT64 terminal.

Open Git Bash and temporarily add the UCRT64 binaries to the PATH:

```bash
export PATH="$PATH:/c/msys64/ucrt64/bin"
```

Verify that Git Bash can now find the AVR tools:

```bash
avr-gcc --version
avrdude -?
avr-objcopy --version
avr-size --version
```

You can also verify their locations with:

```bash
which avr-gcc
which avrdude
```

Expected locations should look similar to:

```text
/c/msys64/ucrt64/bin/avr-gcc
/c/msys64/ucrt64/bin/avrdude
```

---

### 5. Make the Git Bash configuration permanent

Instead of entering the PATH command every time Git Bash is opened, add it to the Git Bash configuration file.

Run:

```bash
nano ~/.bashrc
```

Add the following lines:

```bash
export PATH="$PATH:/c/msys64/ucrt64/bin"
alias make='mingw32-make'
```

Save and exit Nano:

```text
Ctrl + O
Enter
Ctrl + X
```

Reload the configuration:

```bash
source ~/.bashrc
```

Now verify:

```bash
make --version
avr-gcc --version
avrdude -?
```

After this, a new Git Bash terminal should automatically have access to the complete AVR development environment.

---

### 6. Verify the complete environment

From Git Bash, all of the following commands should work:

```bash
git --version
make --version
avr-gcc --version
avr-objcopy --version
avr-size --version
avrdude -?
```

If all of these work, the development environment is ready.

---

### 7. Clone and build the firmware

Clone the repository using HTTPS:

```bash
git clone https://github.com/ORG_NAME/REPO_NAME.git
```

Enter the repository:

```bash
cd REPO_NAME
```

Compile the firmware:

```bash
make clean
make
```

A successful build should create:

```text
build/
├── firmware.elf
└── firmware.hex
```

The project currently targets the Arduino Nano's ATmega328P at 16 MHz:

```text
MCU      = atmega328p
F_CPU    = 16000000UL
```

---

### 8. Flashing the Arduino Nano

Connect the Arduino Nano and determine its COM port using:

**Windows Device Manager → Ports (COM & LPT)**

For example:

```text
USB Serial Device (COM4)
```

Then flash the firmware with:

```bash
make flash PORT=COM4
```

Replace `COM4` with the correct port.

If the Nano uses the older bootloader and uploading at 115200 baud fails, try:

```bash
make flash PORT=COM4 UPLOAD_BAUD=57600
```

The upload baud rate is separate from the UART baud rate used by the running firmware.

<!-- ================== AI-GENERATED END ======================== -->



3. Connect Arduino

4. Determine COM port
check teams for diagrams:
 https://liveconcordia.sharepoint.com/:b:/t/ENGR290PROJECT_e76d59/IQBO8V6Zyt0XQLEoqnawDrlOAZipg01H1rBntYHXgxaTZ8U?e=bj0X3I

5. Build project
<!-- ================= AI-GENERATED START ======================= -->
This section was drafted with AI assistance.
Open a Bash terminal (Git Bash on Windows, Linux, or the terminal in GitHub Codespaces). `pwd` prints the current folder, `ls` lists its contents, and `cd` changes folders:

```sh
pwd
ls
cd Firmware
```

If the terminal is already in the repository, do not run `cd Firmware` again. Check that `src/`, `include/`, and `Makefile` are listed.

The Makefile is configured for a classic Nano (ATmega328P at 16 MHz). Run the following from the repository root:

```sh
make
```

`make` runs the default `all` target: it invokes AVR-GCC to compile and link all `src/*.c` files into `build/firmware.elf`, converts that executable to the uploadable Intel HEX file `build/firmware.hex`, and reports the program size. The Makefile supplies `-mmcu=atmega328p`, `-DF_CPU=16000000UL`, size optimization, compiler warnings, and the `include/` header search path.

Other common commands:

```sh
make clean
make MCU=atmega328p F_CPU=16000000UL
```

`make clean` removes generated files under `build/`; the next `make` rebuilds them. The MCU and clock options show how to override the defaults if the board changes. Build output is ignored by Git and should not be committed.

**Current repository status:** this is build infrastructure, not yet a complete firmware application. The source files are empty, so `make` will currently stop at the link step because there is no `main()` function. That is expected until firmware implementation begins. A valid HEX file is required before the board can be programmed.

6. Upload generated HEX through bootloader

Connect the Nano to the computer running `avrdude`, then upload from the repository root. Replace `COM4` with the port found in step 5:

```sh
make flash PORT=COM4
```

For an older Nano bootloader that uses 57600 baud, use:

```sh
make flash PORT=COM4 BAUD=57600
```

On Linux, use the device path instead, such as `make flash PORT=/dev/ttyUSB0`. `make flash` first builds the HEX file and then sends it over the USB serial connection using the Nano's bootloader. It cannot succeed until there is firmware code that builds into a HEX file. A GitHub Codespace can build the file, but usually cannot access a Nano physically connected to your local computer; download the HEX and run the upload command locally in that case.

<!-- ================== AI-GENERATED END ======================== -->