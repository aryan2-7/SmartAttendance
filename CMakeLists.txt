cmake_minimum_required(VERSION 3.16)
project(SmartAttendance VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Core Widgets SvgWidgets Svg QUIET)
if (Qt6_FOUND)
    message(STATUS "Using Qt6")
    set(QT_LIBS Qt6::Core Qt6::Widgets Qt6::Svg Qt6::SvgWidgets)
else()
    find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
    message(STATUS "Qt6 not found, falling back to Qt5")
    set(QT_LIBS Qt5::Core Qt5::Widgets)
endif()

find_package(OpenCV REQUIRED COMPONENTS
    core imgproc objdetect videoio highgui dnn
)

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

    # Shared – Database layer
    src/db/db.cpp

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
    src/main.cpp
    src/auth/WelcomeWindow.h
    src/auth/FontManager.h
    src/registration/StudentRegistrationWindow.h
    src/attendance/AttendanceWindow.h
    src/admin/AdministratorWindow.h
    src/admin/AdminDashboard.h
    src/admin/ManageStudentsWindow.h
    src/admin/AttendanceRecordsWindow.h
    src/admin/CircularProgress.h
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
    src/records
    src/export
    src/db
    src/admin
    src/theme
)

target_link_libraries(SmartAttendance PRIVATE
    ${QT_LIBS}
    ${OpenCV_LIBS}
)

target_compile_definitions(SmartAttendance PRIVATE
    PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)
