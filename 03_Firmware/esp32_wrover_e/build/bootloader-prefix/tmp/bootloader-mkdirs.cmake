# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/sofeware/esp_idf/v5.3/esp-idf/components/bootloader/subproject"
  "D:/Project/07_ESP32/03_Firmware/hello_world/build/bootloader"
  "D:/Project/07_ESP32/03_Firmware/hello_world/build/bootloader-prefix"
  "D:/Project/07_ESP32/03_Firmware/hello_world/build/bootloader-prefix/tmp"
  "D:/Project/07_ESP32/03_Firmware/hello_world/build/bootloader-prefix/src/bootloader-stamp"
  "D:/Project/07_ESP32/03_Firmware/hello_world/build/bootloader-prefix/src"
  "D:/Project/07_ESP32/03_Firmware/hello_world/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/Project/07_ESP32/03_Firmware/hello_world/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/Project/07_ESP32/03_Firmware/hello_world/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
