# PicoBit for ESP-WROOM-32

A Scheme interpreter to be used both on a Linux based workstation and in an
ESP32 IOT processor.

Early piece of code. This is a work in progress and is still under heavy development.
At this point in time, programs are able to be run on both Linux, OSX and ESP32
platforms. Please look at the to-do list below for more information about the state
of development.

This is a port of the PicoBit Scheme system you can find at the following
[location](https://github.com/stamourv/picobit).

The ESP32 processor provides an environment that requires a lot of changes to
the way the scheme virtual machine (vm) operate. As such, I decided to develop a new interpreter
from scratch, using as much as possible from the PicoBit vm implementation.

A major aspect of this decision is related to the relocation of the code space
constants, RAM, ROM and vector heaps to be managed through C vector such that
addresses calculation is done through GCC instead of being manipulated manually
through the code. Indexing RAM heap from index 0 requires the relocation of
ROM space and small numbers/booleans constants in the higher portion of the
address space.

RAM and ROM data spaces are accessed with indexes in separate vectors. The
address space is divided into three zones:

  * 0x0000 - 0xDFFF: RAM Space
  * 0xE000 - 0xFDFF: ROM Space
  * 0xFE00 - 0xFFFF: Coded small ints, true, false and ()

Each ROM and RAM data cell are composed of 5 bytes. Please look at file
`inc/vm-arch.h` for details regarding the vm architecture. As the address space
architecture is different than the original PicoBit, small changes were
required for the picobit compiler in support of this setup (file `assembler.rkt`).

The code produced by the compiler is accessed through a separate vector.
The code addresses start at 0x0000 and are byte addressable.

esp32-scheme-vm is released under the GPLv3.

Guy Turcotte  
December 2017

## Todo for the interpreter

1. As First priorities

   - [ ] Test test Test **All Scheme tests completed, Unit tests at 50%**
   - [x] Implement bignums (no limit integers)
   - [x] ESP32 ESP-IDF integration
   - [ ] Network primitives and suspend/resume capabilities
   - [ ] Implement some primitives to get access to the hardware sensors and IOT interfaces
   - [ ] Produce an easy compile-test-run Makefile context for picobit application development
   - [x] cell separation of flags and pointers to get pointer alignment on 16
         and 32 bits address boundaries and more flexibility on RAM usage

2. Second priorities

   - [ ] Load mechanism through the NET for ESP32
   - [ ] Primitives development and integration in the ESP32 for IOT I/O and networking
   - [ ] Documentation **At 30%**
   - [ ] Implement 32 bits Fixnums
   - [ ] Enlarge the global space (from the maximum of 256 global values)
   - [ ] Enlarge the ROM heap space (from the maximum of 256 ROM constants)
   - [ ] Version numbering

## Todo for the compiler

1. As first priorities to get something working with the interpreter

   - [x] Cells formatting on 5 bytes on constants ROM space, not 4
   - [x] Little-Endian
   - [x] ROM Heap space (for constants) start at virtual address 0xE000
   - [x] Code address space starts at 0x0000, not x8000
   - [x] Change coding of small integers, FALSE, TRUE and ()

2. Second priorities

   - [ ] New 32 bits Fixnums
   - [ ] Enlarge the global space (from the maximum of 256 global values)
   - [ ] Enlarge the ROM heap space (from the maximum of 256 ROM constants)
   - [ ] Add version numbering in code (Major + Minor numbers)

## Installation

Here are the steps to get a development environment that will allow
for both workstations (Linux or OSX) and ESP32 scheme programming with this PicoBit
implementation (A Microsoft Windows context of development will eventually be
documented):

1. Install the development tools:

   * ESP-IDF - As per the ESP-IDF Installation guidelines at that
    [location](http://esp-idf.readthedocs.io/en/latest/get-started/linux-setup.html)
     for Linux, or that
     [location](http://esp-idf.readthedocs.io/en/latest/get-started/macos-setup.html)
     for OSX.

   * GCC and other tools - As per your OS environment. The following products
     are required: **GCC, gawk, git, doxygen**

   * DrRacket from that [location](http://racket-lang.org/).

2. Download the esp32-scheme-vm from github

   * From a terminal screen, go to a folder into which this repository will
     be retrieved.

   * execute the following command:

    ```
      $ git clone https://github.com/turgu1/picobit.git
    ```

3. Compile the workstation version of the compiler, picobit-vm and hex2bin
   programs.

   * Do the following commands in a terminal screen:

    ```
      $ cd esp32-scheme-vm
      $ make
      $ cd hex2bin
      $ make
      $ cd ..
    ```

4. Prepare your scheme program. A demonstration program named "fibo.scm" is
supplied that compute the Fibonacci value of the 200 first numbers of the
Fibonacci suite. You can compile it and run it on your workstation with the
following commands:

    ```
      $ ./picobit fibo.scm
      $ ./picobit-vm fibo.hex
    ```

You can get the same result using the following command:

    ```
      $ ./p fibo.scm
    ```

5. Compile and run the program on a ESP32 platform. For this, you will need
an ESP32 electronic circuit **hooked to your computer through a USB serial port**.
The author uses a ESP-WROOM-32 development board (Nodemcu) similar to the
one offered by [amazon](https://www.amazon.com/HiLetgo-ESP-WROOM-32-Development-Microcontroller-Integrated/dp/B0718T232Z/ref=sr_1_1?ie=UTF8&qid=1515534535&sr=8-1&keywords=esp32+nodemcu). The example here will use the `fibo.scm` program compiled in the preceding step.

   * Configure ESP-IDF to identify the USB port on which the development board is
     connected. Use the following command:

     ```
       $ make -f Makefile.esp32 menuconfig
     ```

   * The application `fibo.scm` is already compiled. We need to create a binary
     version of it through the following command:

     ```
       $ ./h2b fibo.hex fibo.bin
     ```

   * The resulting `fibo.bin` file must be made available to picobit-vm as it
     will be integrated with the virtual machine code and pushed in the ESP32:

     ```
       $ cp fibo.bin main/program.bin
     ```

     You can also produce the binary from the scheme source code with the
     following command. The binary will be placed in main/program.bin:

     ```
       $ ./pb filo.scm
     ```

   * Now, we compile and burn the code in the ESP32. The first time picobit-vm
     will be built for the ESP32, it will take some time as the entire ESP-IDF
     environment will also be compiled:

     ```
       $ make -f Makefile.esp32 flash
     ```

   * At this point, the program is now on the ESP32 platform. You can monitor
     the resulting output using the following command:

     ```
       $ make -f Makefile.esp32 monitor
     ```

# Original PicoBit Readme file

This is a very small Scheme system designed to run on small
microcontrollers, currently PIC18 and ARM.

It consists of

* a bytecode compiler which compiles Scheme source code to bytecode.
  The bytecode compiler is run using Racket, usually on a
  PC/workstation.

* a bytecode interpreter which can be either compiled to run on a
  microcontroller, or to be run on some common operating systems, at
  least GNU/Linux (Windows support hasn't been tested in a while,
  though, and especially the networking part might not work out of the
  box). It is written in (mostly) portable C.


## USAGE:

1. Install Racket (needed to run the bytecode compiler)
     racket-lang.org

2. Build the VM:
     make
 This will build the VM with the default configuration (workstation,
 no debugging). To configure the VM for another architecture, or with
 different options, run
     make help
 from the `vm' directory.

Note: gawk is required to build the VM.

2. Compile a Scheme program:
     ./picobit prog.scm

3. Run the resulting program:
     ./picobit-vm prog.hex

Note: The `p' script is a shortcut for compiling and running programs:
     ./p prog.scm
 is equivalent to
     ./picobit prog.scm ; ./picobit-vm prog.hex


## SEE ALSO:

  * A paper describing PICOBIT has been presented to IFL 2009:
    http://www.ccs.neu.edu/home/stamourv/papers/picobit.pdf
    Slides from the presentation:
    http://www.ccs.neu.edu/home/stamourv/slides/picobit-ifl09.pdf

  * S3 (Small Scheme Stack) : A Scheme TCP/IP Stack Targeting Small
    Embedded Applications
    http://www.ccs.neu.edu/home/stamourv/papers/s3.pdf
    Slides from the presentation:
    http://www.ccs.neu.edu/home/stamourv/slides/s3-sw08.pdf

 PICOBIT is a descendant of the BIT and PICBIT systems. You can find
 papers describing these systems at:
    http://w3.ift.ulaval.ca/~dadub100/


## HISTORY:

 Marc Feeley originally wrote PICOBIT around 2006.
 Vincent St-Amour took over development in 2008.
 Jim Shargo worked on a port to Racket in 2011.
 The Racket port was completed in June 2011.
 Peter Zotov (whitequark) ported PICOBIT to ARM in August 2011.

## LICENCE:

 PICOBIT is released under the GPLv3.
