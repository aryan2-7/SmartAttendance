cmake_minimum_required(VERSION 3.16)
project(SmartAttendance VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Qt setup ──────────────────────────────────────────────────────────────────
# Try Qt6 first; fall back to Qt5 so all team members are covered regardless
# of what they installed.
find_package(Qt6 COMPONENTS Widgets QUIET)
if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
    message(STATUS "Using Qt6")
else()
    find_package(Qt5 COMPONENTS Widgets REQUIRED)
    set(QT_VERSION_MAJOR 5)
    message(STATUS "Using Qt5")
endif()

set(CMAKE_AUTOMOC ON)   # auto-run moc for Q_OBJECT classes
set(CMAKE_AUTOUIC ON)   # auto-run uic for .ui files (if you add any)
set(CMAKE_AUTORCC ON)   # auto-run rcc for .qrc files (if you add any)


find_package(OpenCV REQUIRED COMPONENTS
    core
    imgproc
    highgui     # imshow, waitKey, destroyAllWindows
    videoio
    objdetect
    face        # LBPHFaceRecognizer – requires opencv_contrib
)
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV libs:    ${OpenCV_LIBS}")

# ── Main application ──────────────────────────────────────────────────────────
add_executable(SmartAttendance
    src/main.cpp
    src/FaceRegistration.cpp
    # Add future modules here, e.g.:
    # src/DatabaseManager.cpp
    # src/AttendanceMarking.cpp
    # src/RecordsViewer.cpp
)

target_include_directories(SmartAttendance PRIVATE
    src/
    ${OpenCV_INCLUDE_DIRS}
)

if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(SmartAttendance PRIVATE
        Qt6::Widgets
        ${OpenCV_LIBS}
    )
else()
    target_link_libraries(SmartAttendance PRIVATE
        Qt5::Widgets
        ${OpenCV_LIBS}
    )
endif()

# ── Webcam test (standalone, not part of the main app) ────────────────────────
add_executable(webcam_test
    tests/webcam_test.cpp
)
target_include_directories(webcam_test PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(webcam_test PRIVATE ${OpenCV_LIBS})

message(STATUS
    "⚠️  Make sure data/haarcascade_frontalface_default.xml exists. "
    "Copy it from: "
    "/opt/homebrew/share/opencv4/haarcascades/ "
    "(Apple Silicon) or "
    "/usr/local/share/opencv4/haarcascades/ "
    "(Intel Mac)"
)