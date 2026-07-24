cmake_minimum_required(VERSION 3.16)
project(SmartAttendance VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ---- Detect compiler ABI (MSVC vs MinGW) on Windows ----
# Qt/OpenCV Windows binaries are ABI-specific: an MSVC-built Qt cannot link
# against a MinGW-built OpenCV (or vice versa). Detect which toolchain CMake
# picked so we can validate the found packages actually match it.
if (WIN32)
    if (MSVC)
        set(WIN_COMPILER_ABI "msvc")
        message(STATUS "Detected compiler: MSVC (${CMAKE_CXX_COMPILER})")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(WIN_COMPILER_ABI "mingw")
        message(STATUS "Detected compiler: MinGW/GCC (${CMAKE_CXX_COMPILER})")
    else()
        message(WARNING "Unrecognized compiler on Windows: ${CMAKE_CXX_COMPILER_ID}. "
                         "Qt/OpenCV ABI compatibility cannot be verified automatically.")
        set(WIN_COMPILER_ABI "unknown")
    endif()
endif()

# Warns (rather than hard-fails) if a found package's path suggests the wrong ABI.
function(check_lib_abi_matches LIB_NAME LIB_PATH)
    if (NOT WIN32 OR NOT DEFINED WIN_COMPILER_ABI OR WIN_COMPILER_ABI STREQUAL "unknown" OR NOT LIB_PATH)
        return()
    endif()
    string(TOLOWER "${LIB_PATH}" LOWER_PATH)
    if (LOWER_PATH MATCHES "mingw" AND WIN_COMPILER_ABI STREQUAL "msvc")
        message(FATAL_ERROR
            "${LIB_NAME} looks like a MinGW build (path: ${LIB_PATH}) but you are "
            "compiling with MSVC (${CMAKE_CXX_COMPILER}). These ABIs are not link-compatible.\n"
            "Fix: install/point CMAKE_PREFIX_PATH at an MSVC build of ${LIB_NAME}, "
            "or reconfigure with a MinGW generator (e.g. -G \"MinGW Makefiles\").")
    elseif (LOWER_PATH MATCHES "msvc" AND WIN_COMPILER_ABI STREQUAL "mingw")
        message(FATAL_ERROR
            "${LIB_NAME} looks like an MSVC build (path: ${LIB_PATH}) but you are "
            "compiling with MinGW/GCC (${CMAKE_CXX_COMPILER}). These ABIs are not link-compatible.\n"
            "Fix: install/point CMAKE_PREFIX_PATH at a MinGW build of ${LIB_NAME}, "
            "or reconfigure using an MSVC generator (e.g. -G \"Visual Studio 17 2022\").")
    else()
        message(STATUS "${LIB_NAME} ABI check OK for ${WIN_COMPILER_ABI} toolchain")
    endif()
endfunction()

find_package(Qt6 COMPONENTS Core Widgets SvgWidgets Svg QUIET)
if (Qt6_FOUND)
    message(STATUS "Using Qt6")
    set(QT_LIBS Qt6::Core Qt6::Widgets Qt6::Svg Qt6::SvgWidgets)
    check_lib_abi_matches("Qt6" "${Qt6_DIR}")
else()
    find_package(Qt5 COMPONENTS Core Widgets Svg REQUIRED)
    message(STATUS "Qt6 not found, falling back to Qt5")
    set(QT_LIBS Qt5::Core Qt5::Widgets Qt5::Svg)
    check_lib_abi_matches("Qt5" "${Qt5_DIR}")
endif()

find_package(OpenCV REQUIRED COMPONENTS
    core imgproc objdetect videoio highgui dnn
)
check_lib_abi_matches("OpenCV" "${OpenCV_DIR}")
if (WIN32 AND WIN_COMPILER_ABI STREQUAL "msvc" AND NOT DEFINED OpenCV_DIR)
    message(WARNING "OpenCV_DIR not set — cannot verify OpenCV was built for MSVC. "
                     "If linking fails, point OpenCV_DIR at the vc16/vc17 build folder explicitly.")
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/sqlite/sqlite3.c")
    message(STATUS "Using bundled SQLite from sqlite")
    set(SQLITE_SOURCES sqlite/sqlite3.c)
    set(SQLITE_INCLUDE sqlite)
else()
    message(STATUS "Bundled SQLite not found")
    set(SQLITE_SOURCES "")
    set(SQLITE_INCLUDE "")
endif()

set(SOURCES
    src/main.cpp

    # Module 1 – Login
    src/auth/WelcomeWindow.cpp
    src/auth/FontManager.cpp

    # Module 2 – Face Registration
    src/registration/StudentRegistrationWindow.cpp

    # Module 3 – Attendance Marking
    src/attendance/AttendanceWindow.cpp

    # Module 4 – Records Viewer
    #src/records/RecordsViewer.cpp

    # Module 5 – CSV Export
    #src/export/CsvExporter.cpp

    # Shared – Database layer (new DAO architecture)
    src/db/Database.cpp
    src/db/StudentDAO.cpp
    src/db/SubjectDAO.cpp
    src/db/AttendanceDAO.cpp
    src/db/UserDAO.cpp

    # Module 6 – Administrator / Dashboard
    src/admin/AdministratorWindow.cpp
    src/admin/AdminDashboard.cpp
    src/admin/ManageStudentsWindow.cpp
    src/admin/AttendanceRecordsWindow.cpp
    src/admin/CircularProgress.cpp

    ${SQLITE_SOURCES}
)

add_executable(SmartAttendance
    ${SOURCES}
    src/auth/WelcomeWindow.h
    src/auth/FontManager.h
    src/registration/StudentRegistrationWindow.h
    src/attendance/AttendanceWindow.h
    src/admin/AdministratorWindow.h
    src/admin/AdminDashboard.h
    src/admin/ManageStudentsWindow.h
    src/admin/AttendanceRecordsWindow.h
    src/admin/CircularProgress.h
    src/db/Database.h
    src/db/StudentDAO.h
    src/db/SubjectDAO.h
    src/db/AttendanceDAO.h
    src/db/UserDAO.h
    src/db/sha256.h
    src/theme/Theme.h
    fonts.qrc
)

target_include_directories(SmartAttendance PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${SQLITE_INCLUDE}
    src
    src/auth
    src/registration
    src/attendance
    src/db
    src/admin
    src/theme
)

target_link_libraries(SmartAttendance PRIVATE
    ${QT_LIBS}
    ${OpenCV_LIBS}
)

string(REPLACE "\\" "/" NORMALIZED_SOURCE_DIR "${CMAKE_SOURCE_DIR}")
target_compile_definitions(SmartAttendance PRIVATE
    PROJECT_SOURCE_DIR="${NORMALIZED_SOURCE_DIR}"
)
