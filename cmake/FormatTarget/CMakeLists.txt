# SPDX-FileCopyrightText: 2022 The dune-localfefunction developers
# mueller@ibb.uni-stuttgart.de SPDX-License-Identifier: LGPL-2.1-or-later

cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
project(dunelocalfeformatFormat)

if(ADD_FORMATTARGET)
  include(CPM.cmake)
  cpmaddpackage(
    NAME
    Format.cmake
    VERSION
    1.7.3
    GITHUB_REPOSITORY
    TheLartians/Format.cmake
    OPTIONS
    "FORMAT_SKIP_CMAKE NO"
    "CMAKE_FORMAT_EXCLUDE config.h.cmake")
endif()
