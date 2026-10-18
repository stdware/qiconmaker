# Writes an ICNS with qiconmaker and turns it back into an iconset with iconutil, the reader that
# the file is checked against, because Apple publishes no specification of ICNS.
#
#     cmake -D qiconmaker=<program> -D iconutil=<program> -D data=<dir> -D out=<dir>
#           -P check_icns.cmake

file(REMOVE_RECURSE "${out}")
file(MAKE_DIRECTORY "${out}")

execute_process(
    COMMAND "${qiconmaker}" "${out}/icon.icns" "${data}/drawing.svg" "16=${data}/drawing16.svg"
    RESULT_VARIABLE _code
)
if(NOT _code EQUAL 0)
    message(FATAL_ERROR "qiconmaker failed with ${_code}")
endif()

execute_process(
    COMMAND "${iconutil}" --convert iconset --output "${out}/icon.iconset" "${out}/icon.icns"
    RESULT_VARIABLE _code
    OUTPUT_VARIABLE _output
    ERROR_VARIABLE _output
)
if(NOT _code EQUAL 0)
    message(FATAL_ERROR "iconutil rejected the file:\n${_output}")
endif()

# The images of an iconset, as iconutil names them
foreach(_name
        icon_16x16 icon_16x16@2x icon_32x32 icon_32x32@2x icon_128x128 icon_128x128@2x
        icon_256x256 icon_256x256@2x icon_512x512 icon_512x512@2x)
    if(NOT EXISTS "${out}/icon.iconset/${_name}.png")
        file(GLOB _found RELATIVE "${out}/icon.iconset" "${out}/icon.iconset/*")
        message(FATAL_ERROR "iconutil wrote no ${_name}.png, only: ${_found}")
    endif()
endforeach()
