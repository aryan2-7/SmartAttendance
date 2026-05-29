# Smart Attendance System

---

## Stack

| Layer | Technology |
|-------|-----------|
| Language | Technology |
| GUI | Qt6 with Qt5 fallback |
| Computer Vision | OpenCV and opencv_contrib |
| Database | SQLite amalgamation |
| Build | Cmake |
| Platform | macOS (Aryan), Windows (Rest of the team) |
 
---

## Link
https://github.com/aryan2-7/SmartAttendance

## File Structure
```
SmartAttendance/
├── CMakeLists.txt
├── README.md
├── .gitignore
├── src/
│   ├── main.cpp                        will be used to compile all modules
│   ├── auth/
│   │   ├── LoginWindow.h               ⬜ not started
│   │   └── LoginWindow.cpp             ⬜ not started
│   ├── registration/
│   │   ├── FaceRegistration.h          ✅ done by Aryan
│   │   └── FaceRegistration.cpp        ✅ done by Aryan
│   ├── attendance/
│   │   ├── AttendanceMarker.h          ⬜ not started
│   │   └── AttendanceMarker.cpp        ⬜ not started
│   ├── records/
│   │   ├── RecordsViewer.h             ⬜ not started
│   │   └── RecordsViewer.cpp           ⬜ not started
│   ├── export/
│   │   ├── CsvExporter.h               ⬜ not started
│   │   └── CsvExporter.cpp             ⬜ not started
│   └── db/
│       ├── Database.h                  ⬜ not started
│       └── Database.cpp                ⬜ not started
├── resources/
│   ├── haarcascades/
│   │   └── haarcascade_frontalface_default.xml
│   ├── trained_models/                 # .yml files saved here per registered user
│   └── icons/
├── data/
│   └── attendance.db                   # gitignored, generated at runtime
└── docs/
    └── architecture.md                 #Used to keep track of what is happening
```
 
---


--- 

## Key decisions made
- LBPH algorithm chosen insted of deep learning as it requrires too many initial images, computational power and is less efficient, our skill level was also considered for this decision
- Liveness detection explicitly descoped — for now, will come back to this if the project is finished early, very complex
- One .yml model file per student (name_roll.yml) — no numeric ID needed in filename
- CASCADE_PATH = `../resources/haarcascades/haarcascade_frontalface_default.xml`
- MODELS_DIR   = `../resources/trained_models/`
- Paths are relative to `build/` directory (where the executable runs)
- SQLite will use the amalgamation approach (drop-in .c + .h, no brew install)

---

## Build command (all OS)
```bash
mkdir build
cd build
cmake ..
cmake --build . -j4
```
---

## What to change in this file (for teamates)
When you finish a module, change the status next to the file structure from ⬜ to ✅ and add a one-line summary of what it does
If any major decision is made, update the relevant section.
