# The following macros allow certain features and debugging output
# to be disabled / enabled at compile time.

# Debug output from engine
DEFINES += LOG_ENGINE

static: DEFINES += DISABLE_FFT

# Perform spectrum analysis calculation in a separate thread
DEFINES += SPECTRUM_ANALYSER_SEPARATE_THREAD

# Suppress warnings about strncpy potentially being unsafe, emitted by MSVC
win32: DEFINES += _CRT_SECURE_NO_WARNINGS

win32 {
    build_pass {
        CONFIG(release, release|debug): app_build_dir = /release
        CONFIG(debug, release|debug): app_build_dir = /debug
    }
}

