## Actual steps

* nix-shell
* ./subprojects/fetch-deps.py
* make mesonSetupDebug
* make compile
* steam-run make run

make compile && LD_LIBRARY_PATH=/etc/lsb/lib/ ./build/wpicker
make compileRelease && LD_LIBRARY_PATH=/etc/lsb/lib/ ./build_release/wpicker

## Quick test

TMP_DIR=/tmp/delme10                   &&
git clone ./woyspritemeta/ $TMP_DIR    &&
cd ./$TMP_DIR/                         &&
./subprojects/fetch-deps.py            &&
make mesonSetupDebug                   &&
make compile                           &&
LD_LIBRARY_PATH=/etc/lsb/lib/ make run &&
cd -
