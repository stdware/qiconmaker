cmake_minimum_required(VERSION 3.22)

project(qiconmaker
    VERSION 0.1.0
    LANGUAGES CXX
    DESCRIPTION "Writes .ico and .icns files from drawings in SVG and PNG"
)

option(QICONMAKER_BUILD_TESTS "Build the tests" OFF)
option(QICONMAKER_INSTALL "Generate installation rules" ON)

include(GNUInstallDirs)

# Sources are encoded in UTF-8. Without /utf-8, MSVC decodes them in the active code page of the
# host, which corrupts non-ASCII comments and may cause the following line to be discarded.
if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Svg)

# ----------------------------------
# Library
# ----------------------------------
set(_headers
    include/qiconmaker/IcnsFormat.h
    include/qiconmaker/IcoFormat.h
    include/qiconmaker/IconSet.h
    include/qiconmaker/IconSource.h
)
add_library(QIconMaker STATIC
    ${_headers}
    src/IcnsFormat.cpp
    src/IcoFormat.cpp
    src/IconSet.cpp
    src/IconSource.cpp
)
add_library(qiconmaker::QIconMaker ALIAS QIconMaker)

target_include_directories(QIconMaker
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/qiconmaker
)
target_link_libraries(QIconMaker PUBLIC Qt6::Gui PRIVATE Qt6::Svg)
target_compile_features(QIconMaker PUBLIC cxx_std_17)

# Debug and Release builds may be installed to the same prefix. The postfix keeps the debug
# library from replacing the release library of the same name.
set_target_properties(QIconMaker PROPERTIES DEBUG_POSTFIX d)

# ----------------------------------
# Command line program
# ----------------------------------
add_executable(qiconmaker tools/cli/main.cpp)
target_link_libraries(qiconmaker PRIVATE QIconMaker)
target_compile_definitions(qiconmaker PRIVATE QICONMAKER_VERSION="${PROJECT_VERSION}")

# ----------------------------------
# Tests
# ----------------------------------
if(QICONMAKER_BUILD_TESTS)
    enable_testing()
    find_package(Qt6 REQUIRED COMPONENTS Test)

    set(_data "${CMAKE_CURRENT_SOURCE_DIR}/tests/data")
    # The tests run without a display, and find Qt beside them on Windows, where the DLLs of Qt
    # are in its bin directory.
    set(_environment "QT_QPA_PLATFORM=set:offscreen")
    if(WIN32)
        list(APPEND _environment "PATH=path_list_prepend:${QT6_INSTALL_PREFIX}/${QT6_INSTALL_BINS}")
    endif()

    foreach(_name IcnsFormat IcoFormat IconSet IconSource)
        add_executable(test_${_name} tests/test_${_name}.cpp)
        set_target_properties(test_${_name} PROPERTIES AUTOMOC ON)
        target_link_libraries(test_${_name} PRIVATE QIconMaker Qt6::Test)
        target_compile_definitions(test_${_name} PRIVATE QICONMAKER_TEST_DATA="${_data}")
        add_test(NAME test_${_name} COMMAND test_${_name})
        set_tests_properties(test_${_name} PROPERTIES ENVIRONMENT_MODIFICATION "${_environment}")
    endforeach()

    # The program, run as a build runs it, with a source of its own for the size 16
    set(_out "${CMAKE_CURRENT_BINARY_DIR}/tests")
    add_test(NAME cli_ico COMMAND qiconmaker "${_out}/cli.ico" "${_data}/drawing.svg"
        "16=${_data}/drawing16.svg")
    add_test(NAME cli_icns COMMAND qiconmaker "${_out}/cli.icns" "${_data}/drawing.svg"
        "16=${_data}/drawing16.svg")
    add_test(NAME cli_rejects_another_format COMMAND qiconmaker "${_out}/cli.png"
        "${_data}/drawing.svg")
    set_tests_properties(cli_rejects_another_format PROPERTIES WILL_FAIL TRUE)
    set_tests_properties(cli_ico cli_icns cli_rejects_another_format PROPERTIES
        ENVIRONMENT_MODIFICATION "${_environment}")

    # Apple publishes no specification of ICNS. iconutil, which turns an ICNS back into an
    # iconset, is the reader that the file is checked against.
    if(APPLE)
        find_program(ICONUTIL_EXECUTABLE iconutil)
        if(ICONUTIL_EXECUTABLE)
            add_test(NAME cli_icns_iconutil COMMAND ${CMAKE_COMMAND}
                -D "qiconmaker=$<TARGET_FILE:qiconmaker>"
                -D "iconutil=${ICONUTIL_EXECUTABLE}"
                -D "data=${_data}"
                -D "out=${_out}/iconutil"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/check_icns.cmake"
            )
            set_tests_properties(cli_icns_iconutil PROPERTIES
                ENVIRONMENT_MODIFICATION "${_environment}")
        endif()
    endif()
endif()

# ----------------------------------
# Install
# ----------------------------------
if(QICONMAKER_INSTALL)
    include(CMakePackageConfigHelpers)

    install(TARGETS QIconMaker qiconmaker
        EXPORT qiconmakerTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
    install(FILES ${_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qiconmaker)
    install(EXPORT qiconmakerTargets
        FILE qiconmakerTargets.cmake
        NAMESPACE qiconmaker::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qiconmaker
    )
    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/qiconmakerConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/qiconmakerConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qiconmaker
    )
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/qiconmakerConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/qiconmakerConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/qiconmakerConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qiconmaker
    )
endif()
