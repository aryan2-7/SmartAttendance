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

## Module status

### ✅ Module 2 — Face Registration (`src/registration/`) 
- Opens webcam, detects face with Haar cascade
- Captures 30 grayscale 100×100 samples
- Trains LBPHFaceRecognizer, saves to `resources/trained_models/<Name>_<Roll>.yml`
- Qt widget: name input, roll input, progress bar, live preview
- Emits `registrationComplete(int userId, QString name, QString rollNo)` signal
- **Not yet connected to database** — signal is just logged in main.cpp for now

### ⬜ Module 0 — Database layer (`src/db/`)
- Shared by all other modules — we have to build this next
- Needs tables: `students`, `attendance_records`
- SQLite amalgamation (single .c/.h drop-in, no external install needed)
- Can use simple C++ wrapper: addStudent(), markAttendance(), getRecords()

### ⬜ Module 1 — Login / Auth (`src/auth/`)
- Simple hardcoded admin password for now (no user accounts)
- Qt login window shown before anything else
- On success: show main tab widget
- On Failure: Prompt to try again

### ⬜ Module 3 — Live Attendance Marking (`src/attendance/`)
- Opens webcam, runs face detection same as registration
- For each detected face: loads all .yml models, runs predict()
- If confidence abhove threshold → mark attendance in DB
- Anti-duplicate: don't mark twice in same session

### ⬜ Module 4 — Records Viewer (`src/records/`) 
- Qt table widget pulling from SQLite attendance_records
- Filter by date / student
- Basically creates an overview for the Attendance and includes stuff like attendance percentage

### ⬜ Module 5 — CSV Export (`src/export/`)
- Reads from DB, writes comma-separated file
- Triggered by a button in the Records Viewer
- Basically creates an overview for the Attendance and includes stuff like attendance percentage

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
