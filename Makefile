.PHONY: *
default:

# ===== basic commands =====

clean:
	rm -rf build build_release

# https://mesonbuild.com/howtox.html#use-address-sanitizer
# https://mesonbuild.com/Builtin-options.html#details-for-buildtype
# '--debug'          Redundant because of '--buildtype=debug'
# '-Db_ndebug=false' Redundant because of '--buildtype=debug'
# '-Doptimization=g' Redundant because of '--buildtype=debug'
mesonSetupDebug:
	meson setup --reconfigure --prefix=$(CURDIR)/build build \
		--buildtype=debug -Doptimization=g -Db_sanitize=address,undefined

mesonSetupRelease:
	meson setup --reconfigure --prefix=$(CURDIR)/build_release build_release \
		--buildtype=release -Doptimization=2 -Db_ndebug=true

compile:
	meson compile -C build

compileRelease:
	meson compile -C build_release

run:
	LSAN_OPTIONS=suppressions=suppr.txt ./build/sprite

test:
	meson test --interactive -C build

# For running labs:
# make compile && LSAN_OPTIONS=suppressions=suppr.txt LD_LIBRARY_PATH=/etc/lsb/lib/ ./build/mylab
