# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/YQRepo/pico/pico-sdk/tools/pioasm"
  "D:/YQRepo/picohost/pico-hostII/build/pioasm"
  "D:/YQRepo/picohost/pico-hostII/build/pioasm-install"
  "D:/YQRepo/picohost/pico-hostII/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/tmp"
  "D:/YQRepo/picohost/pico-hostII/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/pioasmBuild-stamp"
  "D:/YQRepo/picohost/pico-hostII/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src"
  "D:/YQRepo/picohost/pico-hostII/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/pioasmBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/YQRepo/picohost/pico-hostII/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/pioasmBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/YQRepo/picohost/pico-hostII/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/pioasmBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
