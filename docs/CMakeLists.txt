cmake_minimum_required(VERSION 3.15)
project(MD2HTML)

find_program(PANDOC_EXEC pandoc REQUIRED)
find_program(QHELPGENERATOR_EXEC qhelpgenerator REQUIRED)

# List of MD files
file(GLOB MD_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.md")

# Path for lua filer with escaped version
file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/md2html-links.lua" LUA_FILTER_PATH)
string(REPLACE ";" "\\;" LUA_FILTER_PATH_ESCAPED "${LUA_FILTER_PATH}")

set(HTML_FILES "")
set(HTML_FILES_XML "")
set(HTML_TOC "")

# Convert MD files and collect lists for collection generation
foreach(MD_FILE ${MD_FILES})
    get_filename_component(FILENAME ${MD_FILE} NAME_WE)

    set(HTML_FILE "${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}.html")
    set(HTML_FILES_XML "${HTML_FILES_XML}      <file>${FILENAME}.html</file>\n")
    set(HTML_TOC "${HTML_TOC}      <section title=\"${FILENAME}\" ref=\"${FILENAME}.html\" />\n")

    list(APPEND HTML_FILES ${HTML_FILE})

    add_custom_command(
        OUTPUT ${HTML_FILE}
        COMMAND "${PANDOC_EXEC}"
                "-s"
                "${MD_FILE}"
                "--lua-filter=${LUA_FILTER_PATH_ESCAPED}"
                "-o"
                "${HTML_FILE}"
        DEPENDS ${MD_FILE} "${LUA_FILTER_PATH}"
        VERBATIM
    )
endforeach()

# Paths for images folder
set(IMAGES_SRC "${CMAKE_CURRENT_SOURCE_DIR}/images")
set(IMAGES_DST "${CMAKE_CURRENT_BINARY_DIR}/images")

# List of images
file(GLOB IMAGE_FILES CONFIGURE_DEPENDS "${IMAGES_SRC}/*.*")

set(COPIED_IMAGE_FILES "")

# Copy (changed and non-existing) images
foreach(IMG ${IMAGE_FILES})
    get_filename_component(IMG_NAME ${IMG} NAME)
    set(DEST_IMAGE "${CMAKE_CURRENT_BINARY_DIR}/images/${IMG_NAME}")

    add_custom_command(
        OUTPUT ${DEST_IMAGE}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${IMG} ${DEST_IMAGE}
        COMMENT "Copying image ${IMG_NAME} if changed"
        VERBATIM
    )

    list(APPEND COPIED_IMAGE_FILES ${DEST_IMAGE})
endforeach()

#Generate HTML files XML
set(HTML_FILES_XML "")
foreach(HTML_FILE ${HTML_FILES})
    get_filename_component(FILENAME ${HTML_FILE} NAME)
    set(HTML_FILES_XML "${HTML_FILES_XML}      <file>${FILENAME}</file>\n")
endforeach()

#Generate images XML
set(IMAGE_FILES_XML "")
foreach(IMAGE_FILE ${IMAGE_FILES})
    get_filename_component(IMG_NAME ${IMAGE_FILE} NAME)
    set(IMAGE_FILES_XML "${IMAGE_FILES_XML}      <file>images/${IMG_NAME}</file>\n")
endforeach()

# Combine HTML and images XMLs
set(DOC_FILES_XML "${HTML_FILES_XML}${IMAGE_FILES_XML}")

add_custom_target(helpFiles ALL
    DEPENDS ${HTML_FILES}
    SOURCES ${MD_FILES}
)

add_custom_target(copy_images ALL
    DEPENDS ${COPIED_IMAGE_FILES}
    COMMENT "Copying all images (incremental)"
)

set(NAMESPACE "org.tracexpert.docs")
set(PROJECT_NAME "TraceXpert Documentation")

string(CONFIGURE "${HTML_FILES_XML}" HTML_FILES_XML @ONLY)
string(CONFIGURE "${HTML_TOC}" HTML_TOC @ONLY)

# Generate docs.qhp from template
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/docs.qhp.in
    ${CMAKE_CURRENT_BINARY_DIR}/docs.qhp
    @ONLY
)

set(QCH_FILE "${CMAKE_CURRENT_BINARY_DIR}/docs.qch")

# Generate docs.qch from docs.qhp
add_custom_command(
    OUTPUT ${QCH_FILE}
    COMMAND ${QHELPGENERATOR_EXEC} ${CMAKE_CURRENT_BINARY_DIR}/docs.qhp -o ${QCH_FILE}
    DEPENDS helpFiles copy_images ${CMAKE_CURRENT_BINARY_DIR}/docs.qhp
    COMMENT "Generating Qt Help (.qch)"
)

add_custom_target(qt_help ALL DEPENDS ${QCH_FILE})

# Generate docs.qhcp from template
set(QHCP_FILE "${CMAKE_CURRENT_BINARY_DIR}/docs.qhcp")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/docs.qhcp.in
    ${QHCP_FILE}
    @ONLY
)

# Generate the Qt Help Collection (.qhc)
set(QHC_FILE "${CMAKE_CURRENT_BINARY_DIR}/docs.qhc")

add_custom_command(
    OUTPUT ${QHC_FILE}
    COMMAND ${QHELPGENERATOR_EXEC} ${CMAKE_CURRENT_BINARY_DIR}/docs.qhcp -o ${QHC_FILE}
    DEPENDS ${QCH_FILE} ${QHCP_FILE}
    COMMENT "Generating Qt Help Collection (.qhc)"
)

add_custom_target(qt_help_collection ALL DEPENDS ${QHC_FILE})
