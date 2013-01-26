# Install script for directory: /home/ixu/FANN-2.2.0-Source/src/include

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "1")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/home/ixu/FANN-2.2.0-Source/src/include/fann.h"
    "/home/ixu/FANN-2.2.0-Source/src/include/doublefann.h"
    "/home/ixu/FANN-2.2.0-Source/src/include/fann_internal.h"
    "/home/ixu/FANN-2.2.0-Source/src/include/floatfann.h"
    "/home/ixu/FANN-2.2.0-Source/src/include/fann_data.h"
    "/home/ixu/FANN-2.2.0-Source/src/include/fixedfann.h"
    "/home/ixu/FANN-2.2.0-Source/src/include/compat_time.h"
    "/home/ixu/FANN-2.2.0-Source/src/include/fann_activation.h"
    "/home/ixu/FANN-2.2.0-Source/src/include/fann_cascade.h"
    "/home/ixu/FANN-2.2.0-Source/src/include/fann_error.h"
    "/home/ixu/FANN-2.2.0-Source/src/include/fann_train.h"
    "/home/ixu/FANN-2.2.0-Source/src/include/fann_io.h"
    "/home/ixu/FANN-2.2.0-Source/src/include/fann_cpp.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

