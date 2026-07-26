cmake_minimum_required(VERSION 3.16)
project(SmartAttendance VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default to an optimized build unless the caller asked for something else.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Core Widgets SvgWidgets Svg PrintSupport QUIET)
if (Qt6_FOUND)
    message(STATUS "Using Qt6")
    set(QT_LIBS Qt6::Core Qt6::Widgets Qt6::Svg Qt6::SvgWidgets Qt6::PrintSupport)
else()
    find_package(Qt5 COMPONENTS Core Widgets Svg PrintSupport REQUIRED)
    message(STATUS "Qt6 not found, falling back to Qt5")
    set(QT_LIBS Qt5::Core Qt5::Widgets Qt5::Svg Qt5::PrintSupport)
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
    # Module 5 – Attendance Report Export (PDF)
    src/attendance/export/AttendanceReportExporter.cpp
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
    # Module 7 – Liveness Detection
    src/liveness/LivenessDetector.cpp
    ${SQLITE_SOURCES}
)

add_executable(SmartAttendance
    ${SOURCES}
    src/auth/WelcomeWindow.h
    src/auth/FontManager.h
    src/registration/StudentRegistrationWindow.h
    src/attendance/AttendanceWindow.h
    src/attendance/export/AttendanceReportExporter.h
    src/admin/AdministratorWindow.h
    src/admin/AdminDashboard.h
    src/admin/ManageStudentsWindow.h
    src/admin/AttendanceRecordsWindow.h
    src/admin/CircularProgress.h
    src/liveness/LivenessDetector.h
    src/admin/ScheduleEditor.h
    src/admin/SubjectManagementWindow.h
    src/db/Database.h
    src/db/StudentDAO.h
    src/db/SubjectDAO.h
    src/db/AttendanceDAO.h
    src/db/UserDAO.h
    src/db/sha256.h
    src/db/DbPath.h
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
    src/liveness
    src/theme
)

target_link_libraries(SmartAttendance PRIVATE
    ${QT_LIBS}
    ${OpenCV_LIBS}
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(SmartAttendance PRIVATE -Wall -Wextra)
endif()

string(REPLACE "\\" "/" NORMALIZED_SOURCE_DIR "${CMAKE_SOURCE_DIR}")
target_compile_definitions(SmartAttendance PRIVATE
    PROJECT_SOURCE_DIR="${NORMALIZED_SOURCE_DIR}"
)

option(PACKAGING "Building for installer packaging" OFF)
if(PACKAGING)
    target_compile_definitions(SmartAttendance PRIVATE INSTALLED_BUILD)
endif()

if(APPLE)
    set(BUNDLE_IDENTIFIER "edu.ku.smartattendance")
    configure_file(
        resources/Info.plist.in
        "${CMAKE_CURRENT_BINARY_DIR}/Info.plist"
        @ONLY
    )
    set_source_files_properties(
        resources/SmartAttendance.icns
        PROPERTIES MACOSX_PACKAGE_LOCATION "Resources"
    )
    target_sources(SmartAttendance PRIVATE resources/SmartAttendance.icns)
    set_target_properties(SmartAttendance PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE SmartAttendance.icns
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist"
    )
endif()

if(PACKAGING)
    if(APPLE)
        set_source_files_properties(
            resources/models/face_detection_yunet_2023mar.onnx
            resources/models/face_recognition_sface_2021dec.onnx
            PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/models"
        )
        target_sources(SmartAttendance PRIVATE
            resources/models/face_detection_yunet_2023mar.onnx
            resources/models/face_recognition_sface_2021dec.onnx
        )
    elseif(WIN32)
        install(FILES
            resources/models/face_detection_yunet_2023mar.onnx
            resources/models/face_recognition_sface_2021dec.onnx
            DESTINATION resources/models
        )
    endif()
endif()

set(CPACK_PACKAGE_NAME "SmartAttendance")
set(CPACK_PACKAGE_VENDOR "Kathmandu University - ENGG 102")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Face Recognition Attendance System")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "SmartAttendance")

if(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "SmartAttendance")
elseif(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_PACKAGE_NAME "SmartAttendance")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
endif()

include(CPack)