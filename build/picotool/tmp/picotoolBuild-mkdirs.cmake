# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/YQRepo/picohost/pico-hostII/build/_deps/picotool-src"
  "D:/YQRepo/picohost/pico-hostII/build/_deps/picotool-build"
  "D:/YQRepo/picohost/pico-hostII/build/_deps"
  "D:/YQRepo/picohost/pico-hostII/build/picotool/tmp"
  "D:/YQRepo/picohost/pico-hostII/build/picotool/src/picotoolBuild-stamp"
  "D:/YQRepo/picohost/pico-hostII/build/picotool/src"
  "D:/YQRepo/picohost/pico-hostII/build/picotool/src/picotoolBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/YQRepo/picohost/pico-hostII/build/picotool/src/picotoolBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/YQRepo/picohost/pico-hostII/build/picotool/src/picotoolBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
