cmake_minimum_required(VERSION 3.16)
project(SmartAttendance VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt needs this to find its generated files (MOC, UIC, RCC) — don't remove
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)


# ─── Qt ───────────────────────────────────────────────────────────────────────
# Try Qt6 first, fall back to Qt5 if not found (for teammates on older setups)
find_package(Qt6 COMPONENTS Core Widgets QUIET)
if (Qt6_FOUND)
    message(STATUS "Using Qt6")
    set(QT_LIBS Qt6::Core Qt6::Widgets)
else()
    find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
    message(STATUS "Qt6 not found, falling back to Qt5")
    set(QT_LIBS Qt5::Core Qt5::Widgets)
endif()


# ─── OpenCV ───────────────────────────────────────────────────────────────────
# We need opencv_contrib for the face module (LBPHFaceRecognizer lives there)
# If you installed via homebrew: brew install opencv
# Make sure you have the contrib modules — check with: brew info opencv
find_package(OpenCV REQUIRED COMPONENTS
    core
    imgproc
    objdetect   # Haar cascade
    videoio     # webcam / VideoCapture
    highgui     # imshow etc (handy for debugging)
    face        # LBPHFaceRecognizer — needs opencv_contrib
)

if (NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found. On macOS run: brew install opencv")
endif()

message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV include: ${OpenCV_INCLUDE_DIRS}")


# ─── SQLite ───────────────────────────────────────────────────────────────────
# We bundle the sqlite amalgamation in third_party/sqlite so every teammate
# gets the same version without needing a system install.
# If that folder doesn't exist yet, just comment out this block for now —
# it will be wired up properly in Phase 2 (DatabaseManager module).
if (EXISTS "${CMAKE_SOURCE_DIR}/third_party/sqlite/sqlite3.c")
    message(STATUS "Using bundled SQLite from third_party/sqlite")
    set(SQLITE_SOURCES third_party/sqlite/sqlite3.c)
    set(SQLITE_INCLUDE third_party/sqlite)
else()
    message(STATUS "Bundled SQLite not found — skipping for now (needed for Phase 2)")
    set(SQLITE_SOURCES "")
    set(SQLITE_INCLUDE "")
endif()


# ─── Source files ─────────────────────────────────────────────────────────────
# Add each module's .cpp here as you finish them.
# Commented-out lines = not implemented yet, uncomment when ready.
set(SOURCES
    src/main.cpp

    # Module 1 – Login (Subhechha)
    # src/auth/LoginWindow.cpp

    # Module 2 – Face Registration (Shashwot + Aryan)
    src/registration/FaceRegistration.cpp

    # Module 3 – Attendance Marking (Shashwot + Aryan)
    # src/attendance/AttendanceMarker.cpp

    # Module 4 – Records Viewer (Prachika)
    # src/records/RecordsViewer.cpp

    # Module 5 – CSV Export
    # src/export/CsvExporter.cpp

    # Shared – Database layer (Dilasha)
    # src/db/Database.cpp

    ${SQLITE_SOURCES}
)


# ─── Executable ───────────────────────────────────────────────────────────────
add_executable(SmartAttendance ${SOURCES})

target_include_directories(SmartAttendance PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${SQLITE_INCLUDE}
    src                     # so modules can include each other without relative path gymnastics
    src/auth
    src/registration
    src/attendance
    src/records
    src/export
    src/db
)

target_link_libraries(SmartAttendance PRIVATE
    ${QT_LIBS}
    ${OpenCV_LIBS}
)


# ─── webcam_test (standalone build sanity check) ──────────────────────────────
# This is just a quick binary to confirm your webcam + OpenCV setup is working
# before touching the main app. Build it with: cmake --build . --target webcam_test
if (EXISTS "${CMAKE_SOURCE_DIR}/tests/webcam_test.cpp")
    add_executable(webcam_test tests/webcam_test.cpp)
    target_include_directories(webcam_test PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(webcam_test PRIVATE ${OpenCV_LIBS})
endif()


# ─── How to build (macOS) ─────────────────────────────────────────────────────
#
#   cd ~/Desktop/SmartAttendence   ← yes the folder name has a typo, keep it
#   mkdir -p build && cd build
#   cmake ..
#   make -j$(sysctl -n hw.logicalcpu)   ← uses all your cores, way faster
#
# If cmake can't find Qt or OpenCV, pass the paths manually e.g.:
#   cmake .. -DCMAKE_PREFIX_PATH=$(brew --prefix qt)
#
# If you get a camera permission error on macOS, you need to run the app from
# Finder or give terminal camera access in System Settings > Privacy & Security
# ─────────────────────────────────────────────────────────────────────────────
