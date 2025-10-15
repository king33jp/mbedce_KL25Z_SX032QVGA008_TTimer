# Mbed CE project 定年たいまぁ
定年60歳までをカウントダウンする、ストレス解消ツール

## How to set up this project:

1. Clone it to your machine.  Don't forget to use `--recursive` to clone the submodules: `git clone --recursive https://github.com/king33jp/mbedce_KL25Z_SX032QVGA008_TTimer.git`
2. You may want to update the mbed-os submodule to the latest version, with `cd mbedce_KL25Z_SX032QVGA008_TTimer/mbed-os && git fetch origin && git reset --hard origin/master`
3. Set up the GNU ARM toolchain (and other programs) on your machine using [the toolchain setup guide](https://github.com/mbed-ce/mbed-os/wiki/Toolchain-Setup-Guide).
4. Set up the CMake project for editing.  We have three ways to do this:
    - On the [command line](https://github.com/mbed-ce/mbed-os/wiki/Project-Setup:-Command-Line)
    - Using the [CLion IDE](https://github.com/mbed-ce/mbed-os/wiki/Project-Setup:-CLion)
    - Using the [VS Code IDE](https://github.com/mbed-ce/mbed-os/wiki/Project-Setup:-VS-Code)
5. Build the `flash-HelloWorld` target to upload the code to a connected device.
