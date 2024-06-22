# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/YQRepos/pico/pico-sdk/tools/pioasm"
  "D:/YQRepos/SupercarrierMonitoringSystem/pico-host/build/pioasm"
  "D:/YQRepos/SupercarrierMonitoringSystem/pico-host/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm"
  "D:/YQRepos/SupercarrierMonitoringSystem/pico-host/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/tmp"
  "D:/YQRepos/SupercarrierMonitoringSystem/pico-host/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/PioasmBuild-stamp"
  "D:/YQRepos/SupercarrierMonitoringSystem/pico-host/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src"
  "D:/YQRepos/SupercarrierMonitoringSystem/pico-host/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/PioasmBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/YQRepos/SupercarrierMonitoringSystem/pico-host/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/PioasmBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/YQRepos/SupercarrierMonitoringSystem/pico-host/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/PioasmBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
