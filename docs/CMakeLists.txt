cmake_minimum_required(VERSION 3.15)
project(MD2HTML)

# ----------------------------------------------------------------------
# Tools: Pandoc + qhelpgenerator
# ----------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
find_package(Pandoc REQUIRED)

find_program(QHELPGENERATOR_EXEC qhelpgenerator REQUIRED)

# ----------------------------------------------------------------------
# Markdown input files
# ----------------------------------------------------------------------
file(GLOB MD_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.md")

# ----------------------------------------------------------------------
# Lua filter for normal MD?HTML conversion (your existing one)
# ----------------------------------------------------------------------
file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/md2html-links.lua" LUA_FILTER_PATH)
string(REPLACE ";" "\\;" LUA_FILTER_PATH_ESCAPED "${LUA_FILTER_PATH}")

# ----------------------------------------------------------------------
# Lua filter for TOC generation from README.md :: "## Index"
# ----------------------------------------------------------------------
file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/toc-from-readme.lua" LUA_TOC_FILTER_PATH)
string(REPLACE ";" "\\;" LUA_TOC_FILTER_PATH_ESCAPED "${LUA_TOC_FILTER_PATH}")

set(TOC_XML "${CMAKE_CURRENT_BINARY_DIR}/toc.xml")

# Generate toc.xml at *configure time* so we can file(READ) it
execute_process(
    COMMAND ${CMAKE_COMMAND} -E env TOC_FILE=${TOC_XML}
            "${PANDOC_EXEC}"
            -s
            "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
            "--lua-filter=${LUA_TOC_FILTER_PATH_ESCAPED}"
            -o "${CMAKE_CURRENT_BINARY_DIR}/README-toc-tmp.html"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

# Read generated TOC XML into CMake variable HTML_TOC
file(READ "${TOC_XML}" HTML_TOC)

# ----------------------------------------------------------------------
# Convert all MD files to HTML with Pandoc + md2html-links.lua
# ----------------------------------------------------------------------
set(HTML_FILES "")
set(HTML_FILES_XML "")

foreach(MD_FILE ${MD_FILES})
    get_filename_component(FILENAME ${MD_FILE} NAME_WE)

    set(HTML_FILE "${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}.html")
    list(APPEND HTML_FILES ${HTML_FILE})

    add_custom_command(
        OUTPUT ${HTML_FILE}
        COMMAND "${PANDOC_EXEC}"
                "-s"
                "-f"
                "markdown-implicit_figures"
                "${MD_FILE}"
                "--lua-filter=${LUA_FILTER_PATH_ESCAPED}"
                "--css=img.css"
                "-o"
                "${HTML_FILE}"
        DEPENDS ${MD_FILE} "${LUA_FILTER_PATH}"
        VERBATIM
    )
endforeach()

# ----------------------------------------------------------------------
# Images: copy incrementally to build dir
# ----------------------------------------------------------------------
set(IMAGES_SRC "${CMAKE_CURRENT_SOURCE_DIR}/images")
set(IMAGES_DST "${CMAKE_CURRENT_BINARY_DIR}/images")

file(GLOB IMAGE_FILES CONFIGURE_DEPENDS "${IMAGES_SRC}/*.*")

set(COPIED_IMAGE_FILES "")
set(IMAGE_FILES_XML "")

foreach(IMG ${IMAGE_FILES})
    get_filename_component(IMG_NAME ${IMG} NAME)
    set(DEST_IMAGE "${CMAKE_CURRENT_BINARY_DIR}/images/${IMG_NAME}")
    set(IMAGE_FILES_XML "${IMAGE_FILES_XML}      <file>images/${IMG_NAME}</file>\n")

    add_custom_command(
        OUTPUT ${DEST_IMAGE}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${IMG} ${DEST_IMAGE}
        COMMENT "Copying image ${IMG_NAME} if changed"
        VERBATIM
    )

    list(APPEND COPIED_IMAGE_FILES ${DEST_IMAGE})
endforeach()

# ----------------------------------------------------------------------
# Generate HTML files XML (for <files> in docs.qhp / docs.qhcp)
# ----------------------------------------------------------------------
set(HTML_FILES_XML "")
foreach(HTML_FILE ${HTML_FILES})
    get_filename_component(FILENAME ${HTML_FILE} NAME)
    set(HTML_FILES_XML "${HTML_FILES_XML}      <file>${FILENAME}</file>\n")
endforeach()

# Generate images XML
set(IMAGE_FILES_XML "")
foreach(IMAGE_FILE ${IMAGE_FILES})
    get_filename_component(IMG_NAME ${IMAGE_FILE} NAME)
    set(IMAGE_FILES_XML "${IMAGE_FILES_XML}      <file>images/${IMG_NAME}</file>\n")
endforeach()

# Combined doc files XML (HTML + images) if needed by templates
set(DOC_FILES_XML "${HTML_FILES_XML}${IMAGE_FILES_XML}")

# ----------------------------------------------------------------------
# High-level targets for HTML & images
# ----------------------------------------------------------------------
add_custom_target(helpFiles ALL
    DEPENDS ${HTML_FILES}
    SOURCES ${MD_FILES}
)

add_custom_target(copy_images ALL
    DEPENDS ${COPIED_IMAGE_FILES}
    COMMENT "Copying all images (incremental)"
)

# ----------------------------------------------------------------------
# Variables for qhp/qhcp templates (static bits)
# ----------------------------------------------------------------------
set(NAMESPACE "org.tracexpert.docs")
set(PROJECT_NAME "TraceXpert Documentation")

# Expand any @...@ inside these XML snippets if you ever add placeholders
string(CONFIGURE "${HTML_FILES_XML}" HTML_FILES_XML @ONLY)
string(CONFIGURE "${DOC_FILES_XML}"  DOC_FILES_XML  @ONLY)
string(CONFIGURE "${HTML_TOC}"       HTML_TOC       @ONLY)

# ----------------------------------------------------------------------
# Generate docs.qhp from template (Qt Help Project)
#     docs.qhp.in must contain @HTML_TOC@ inside <toc>...</toc>
# ----------------------------------------------------------------------
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
    DEPENDS ${HTML_FILES} ${COPIED_IMAGE_FILES} ${CMAKE_CURRENT_BINARY_DIR}/docs.qhp
    COMMENT "Generating Qt Help (.qch)"
)

add_custom_target(qt_help ALL DEPENDS ${QCH_FILE})

# ----------------------------------------------------------------------
# Generate docs.qhcp from template (Qt Help Collection Project)
# ----------------------------------------------------------------------
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
    COMMAND ${QHELPGENERATOR_EXEC} ${QHCP_FILE} -o ${QHC_FILE}
    DEPENDS ${QCH_FILE} ${QHCP_FILE}
    COMMENT "Generating Qt Help Collection (.qhc)"
)

add_custom_target(qt_help_collection ALL DEPENDS ${QHC_FILE})

