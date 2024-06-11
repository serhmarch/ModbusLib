if (NOT GOOGLETEST_DIR)
    set(GOOGLETEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external/googletest")
endif()

message("MB: GOOGLETEST_DIR: ${GOOGLETEST_DIR}")

if (EXISTS ${GOOGLETEST_DIR})
    set(GTEST_SRCDIR ${GOOGLETEST_DIR}/googletest)
    set(GMOCK_SRCDIR ${GOOGLETEST_DIR}/googlemock)
else()
    message(FATAL_ERROR "Google Tests not found" )
endif()

include_directories("${GTEST_SRCDIR}" 
                    "${GTEST_SRCDIR}/include")

set(GTEST_SOURCES "${GTEST_SRCDIR}/src/gtest-all.cc")

include_directories("${GMOCK_SRCDIR}" 
                    "${GMOCK_SRCDIR}/include")

set(GMOCK_SOURCES "${GMOCK_SRCDIR}/src/gmock-all.cc")

set(GOOGLETEST_SOURCES ${GTEST_SOURCES} ${GMOCK_SOURCES})