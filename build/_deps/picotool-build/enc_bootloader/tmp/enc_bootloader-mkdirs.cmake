# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/YQRepo/picohost/pico-hostII/build/_deps/picotool-src/enc_bootloader"
  "D:/YQRepo/picohost/pico-hostII/build/_deps/picotool-build/enc_bootloader"
  "D:/YQRepo/picohost/pico-hostII/build/_deps/picotool-build/enc_bootloader"
  "D:/YQRepo/picohost/pico-hostII/build/_deps/picotool-build/enc_bootloader/tmp"
  "D:/YQRepo/picohost/pico-hostII/build/_deps/picotool-build/enc_bootloader/src/enc_bootloader-stamp"
  "D:/YQRepo/picohost/pico-hostII/build/_deps/picotool-build/enc_bootloader/src"
  "D:/YQRepo/picohost/pico-hostII/build/_deps/picotool-build/enc_bootloader/src/enc_bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/YQRepo/picohost/pico-hostII/build/_deps/picotool-build/enc_bootloader/src/enc_bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/YQRepo/picohost/pico-hostII/build/_deps/picotool-build/enc_bootloader/src/enc_bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
