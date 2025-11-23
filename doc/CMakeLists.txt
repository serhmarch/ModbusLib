# doxygen Configuration

cmake_minimum_required(VERSION 3.13) # 2.2 - case insensitive syntax
                                     # 3.13 included policy CMP0077

find_program(MB_DOXYGEN_EXECUTABLE
    NAMES doxygen doxygen.exe
    PATHS
        "$ENV{ProgramFiles}\\Doxygen\\bin"
        "$ENV{ProgramFiles\(x86\)}\\Doxygen\\bin"
    DOC "Doxygen documentation generator"
)

if(MB_DOXYGEN_EXECUTABLE)
    message(STATUS "MB-DOC: Found Doxygen: ${MB_DOXYGEN_EXECUTABLE}")

    project(${MB_LIBRARY_NAME}-doc VERSION ${MB_LIBRARY_VERSION})

    set(MB_DOC_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}")
    set(MB_DOC_DIR "${CMAKE_BINARY_DIR}/doc")
    set(MB_DOC_FILE_NAME "Doxyfile")
    set(MB_DOC_FILE "${MB_DOC_DIR}/${MB_DOC_FILE_NAME}")

    message(STATUS "MB-DOC: Create '${MB_DOC_FILE}' for project '${PROJECT_NAME}'")

    configure_file(${CMAKE_CURRENT_LIST_DIR}/${MB_DOC_FILE_NAME}.in
                   ${MB_DOC_DIR}/${MB_DOC_FILE_NAME})

    add_custom_target(${PROJECT_NAME} ALL
            WORKING_DIRECTORY "${MB_DOC_DIR}"
            COMMAND "${MB_DOXYGEN_EXECUTABLE}" "${MB_DOC_FILE}"
            COMMENT "Build documentation"
            )
    #add_dependencies(${PROJECT_NAME} ${MB_APP_NAME})

else()
    message(WARNING "MB-DOC: Doxygen not found. Documentation will not be generated.")
    message(STATUS "MB-DOC: To install Doxygen, visit: https://www.doxygen.nl/download.html")
endif()

