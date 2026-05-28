# Smart Attendance System

Facial recognition-based attendance system built in C++ using OpenCV and Qt.  
Submitted for ENGG 102 – Kathmandu University, Dept. of Computer Science.

## Team
| Name | Roll | Module |
|------|------|--------|
| Aryan Khatri | 43 | Face Registration|
| Shashwot Karki | 36 |Attendance Marking |
| Subhechha Ghimire | 26 | Login / Auth |
| Dilasha Dulal | 18 | UI/UX |
| Prachika Dhaubhadel | 14 | Database, Records Viewer |

## Tech Stack
- **Language:** C++17
- **GUI:** Qt6 (Qt5 fallback)
- **Computer Vision:** OpenCV 4.x + opencv_contrib (LBPH face recognition)
- **Database:** SQLite (amalgamation)
- **Build:** CMake 3.16+

## Build & Run

```
git clone https://github.com/aryan2-7/SmartAttendance.git
```

```bash
mkdir build
cd build
cmake ..
cmake --build . -j4
./SmartAttendance
```
## Documentation
To check progress go to [This](https://github.com/aryan2-7/SmartAttendance/blob/main/docs/architecture.md)
Teamates are requested to update this after every major change
