# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/YQRepo/pico/pico-sdk/tools/pioasm"
  "D:/YQRepo/supercarriermonitoringsystem/pico-hostII/build/pioasm"
  "D:/YQRepo/supercarriermonitoringsystem/pico-hostII/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm"
  "D:/YQRepo/supercarriermonitoringsystem/pico-hostII/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/tmp"
  "D:/YQRepo/supercarriermonitoringsystem/pico-hostII/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/PioasmBuild-stamp"
  "D:/YQRepo/supercarriermonitoringsystem/pico-hostII/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src"
  "D:/YQRepo/supercarriermonitoringsystem/pico-hostII/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/PioasmBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/YQRepo/supercarriermonitoringsystem/pico-hostII/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/PioasmBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/YQRepo/supercarriermonitoringsystem/pico-hostII/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/PioasmBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
