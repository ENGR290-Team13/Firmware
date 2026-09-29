SETUP:
1. Install AVR-GCC / avr-libc / avrdude

2. Install AVR Studio 4.xx

3. Clone repository
Into a bash terminal:
git clone https://github.com/ENGR290-Team13/Firmware.git


4. Connect Arduino

5. Determine COM port
check teams for diagrams:
 https://liveconcordia.sharepoint.com/:b:/t/ENGR290PROJECT_e76d59/IQBO8V6Zyt0XQLEoqnawDrlOAZipg01H1rBntYHXgxaTZ8U?e=bj0X3I

6. Build project
 * chatgpt generated instructions start here
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

7. Upload generated HEX through bootloader

Connect the Nano to the computer running `avrdude`, then upload from the repository root. Replace `COM4` with the port found in step 5:

```sh
make flash PORT=COM4
```

For an older Nano bootloader that uses 57600 baud, use:

```sh
make flash PORT=COM4 BAUD=57600
```

On Linux, use the device path instead, such as `make flash PORT=/dev/ttyUSB0`. `make flash` first builds the HEX file and then sends it over the USB serial connection using the Nano's bootloader. It cannot succeed until there is firmware code that builds into a HEX file. A GitHub Codespace can build the file, but usually cannot access a Nano physically connected to your local computer; download the HEX and run the upload command locally in that case.

 * end chatgpt generated instruction