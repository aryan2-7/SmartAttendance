# Smart Attendance System

Facial recognition-based attendance system built in C++ using OpenCV and Qt.  
Submitted for ENGG 102 – Kathmandu University, Dept. of Computer Science.

## Team
| Name | Roll | Module |
|------|------|--------|
| Aryan Khatri | 43 | Face Registration, Attendance Marking |
| Shashwot Karki | 36 | Face Registration, Attendance Marking |
| Subhechha Ghimire | 26 | Login / Auth |
| Dilasha Dulal | 18 | Database, Records Viewer |
| Prachika Dhaubhadel | 14 | Database, Records Viewer |

## Tech Stack
- **Language:** C++17
- **GUI:** Qt6 (Qt5 fallback)
- **Computer Vision:** OpenCV 4.x + opencv_contrib (LBPH face recognition)
- **Database:** SQLite (amalgamation)
- **Build:** CMake 3.16+

## Build & Run
```bash
mkdir build && cd build
cmake ..
make -j4
./SmartAttendance
```