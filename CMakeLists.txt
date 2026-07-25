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
    find_package(Qt5 COMPONENTS Core Widgets Svg REQUIRED)
    message(STATUS "Qt6 not found, falling back to Qt5")
    set(QT_LIBS Qt5::Core Qt5::Widgets Qt5::Svg)
endif()

# ---- OpenCV ----
find_package(OpenCV REQUIRED COMPONENTS core imgproc objdetect videoio highgui dnn)
message(STATUS "Found OpenCV ${OpenCV_VERSION} at: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")

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
    src/admin/SubjectManagementWindow.cpp
    src/admin/ScheduleEditor.cpp



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
    src/admin/ScheduleEditor.h
    src/admin/SubjectManagementWindow.h
    src/db/Database.h
    src/db/StudentDAO.h
    src/db/SubjectDAO.h
    src/db/AttendanceDAO.h
    src/db/UserDAO.h
    src/db/sha256.h
    src/theme/Theme.h
    fonts.qrc
    src/admin/SubjectManagementWindow.h
    src/admin/ScheduleEditor.h
    src/admin/ScheduleEditor.cpp
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
