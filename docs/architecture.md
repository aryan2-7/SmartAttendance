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
│   ├── common/
│   │   ├── FacePreprocessor.h          ✅ shared face alignment + CLAHE
│   │   └── FacePreprocessor.cpp        ✅ LBF facemark alignment + preprocessing
│   ├── auth/
│   │   ├── LoginWindow.h               ⬜ not started
│   │   └── LoginWindow.cpp             ⬜ not started
│   ├── registration/
│   │   ├── FaceRegistration.h          ✅ done by Aryan
│   │   └── FaceRegistration.cpp        ✅ done by Aryan
│   ├── attendance/
│   │   ├── AttendanceMarker.h          ✅ temporal smoothing + per-person thresholds
│   │   └── AttendanceMarker.cpp        ✅ temporal smoothing + per-person thresholds
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
│   │   ├── haarcascade_frontalface_default.xml
│   │   └── haarcascade_eye.xml
│   ├── models/
│   │   └── lbfmodel.yaml               # LBF facemark model for face alignment
│   ├── trained_models/                 # .yml files saved here per registered user
│   └── icons/
├── data/
│   └── attendance.db                   # gitignored, generated at runtime
└── docs/
    ├── architecture.md
    └── AI_AGENT_GUIDE.md
```
 
---

## Module status

### ✅ Shared — FacePreprocessor (`src/common/`)
- Loads LBF facemark model (`resources/models/lbfmodel.yaml`) at startup
- `alignFace(grayFrame, faceRect)` — detects 68 landmarks, computes eye-line angle, rotates + crops
- Falls back to unaligned crop if facemark fails (robust on low-quality frames)
- `preprocess(alignedFace)` — CLAHE (clipLimit=2.0, tileGrid=8x8) + resize to 200x200
- `kFaceCropSize = cv::Size(200, 200)` defined once, shared across modules
- **BREAKING**: 200x200 replaces old 100x100 — all registered models must be regenerated

### ✅ Module 2 — Face Registration (`src/registration/`) 
- Opens webcam, detects face with Haar cascade
- Uses shared FacePreprocessor for LBF alignment + CLAHE preprocessing
- Captures 30 aligned grayscale 200x200 samples
- Session/angle prompts force variation (straight → yaw ±15° → pitch up/down)
- Trains LBPHFaceRecognizer (radius=2, neighbors=8, grid=8x8), saves to `resources/trained_models/`
- Qt widget: name input, roll input, progress bar, live preview
- Emits `registrationComplete(int userId, QString name, QString rollNo)` signal

### ✅ Module 3 — Live Attendance Marking (`src/attendance/`)
- Uses shared FacePreprocessor for alignment + CLAHE
- Temporal smoothing: sliding window vote (N=12 frames) eliminates flicker
- Per-person threshold calibration (falls back to global 130.0 cutoff)
- LBPH params: radius=2, neighbors=8, grid=8x8 (exposed as constexpr constants)
- Haar params: scaleFactor=1.05, minNeighbors=6 (tuned to cut false positives)
- Blink liveness state machine preserved

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
