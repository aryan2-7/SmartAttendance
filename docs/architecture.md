# Smart Attendance System

---

## Stack

| Layer | Technology |
|-------|-----------|
| Language | C++17 |
| GUI | Qt6 (with Qt5 fallback) |
| Face Detection | YuNet (`cv::FaceDetectorYN` via OpenCV DNN) |
| Face Recognition | SFace (`cv::FaceRecognizerSF` via OpenCV DNN) |
| Database | SQLite amalgamation (planned) |
| Build | CMake |
| Platform | macOS (Aryan), Windows (rest of team) |

---

## Link
https://github.com/aryan2-7/SmartAttendance

## File Structure
```
SmartAttendance/
├── CMakeLists.txt
├── README.md
├── .gitignore
├── .gitattributes                          # Git LFS for ONNX models
├── attendance.csv                          # gitignored, generated at runtime
├── src/
│   ├── main.cpp                            # entry point: shows LoginWindow → MainMenu
│   ├── auth/
│   │   ├── LoginWindow.h                   ✅ done
│   │   └── LoginWindow.cpp                 ✅ done
│   ├── registration/
│   │   ├── FaceRegistration.h              ✅ done
│   │   └── FaceRegistration.cpp            ✅ done
│   ├── attendance/
│   │   ├── AttendanceMarker.h              ✅ done
│   │   └── AttendanceMarker.cpp            ✅ done
│   ├── MainMenu.h                          ✅ done
│   ├── MainMenu.cpp                        ✅ done
│   ├── records/
│   │   ├── RecordsViewer.h                 ⬜ not started
│   │   └── RecordsViewer.cpp               ⬜ not started
│   ├── export/
│   │   ├── CsvExporter.h                   ⬜ not started
│   │   └── CsvExporter.cpp                 ⬜ not started
│   └── db/
│       ├── Database.h                      ⬜ not started
│       └── Database.cpp                    ⬜ not started
├── resources/
│   ├── models/                             # ONNX models (Git LFS)
│   │   ├── face_detection_yunet_2023mar.onnx
│   │   └── face_recognition_sface_2021dec.onnx
│   ├── haarcascades/
│   │   └── haarcascade_eye.xml             # blink liveness
│   ├── trained_models/                     # .bin files per registered user
│   └── icons/
└── docs/
    └── architecture.md
```

---

## Module status

### ✅ Module 1 — Login / Auth (`src/auth/`)
- Single hardcoded admin password
- Qt login window shown before everything else
- On success: shows MainMenu tab widget

### ✅ Module 2 — Face Registration (`src/registration/`)
- Opens webcam, loads **YuNet** immediately (face box visible from window open)
- User enters name + roll, clicks Start → loads **SFace** for embedding extraction
- Captures 50 embeddings (128-d feature vectors) from aligned face crops
- Averages embeddings, saves to `resources/trained_models/<Name>_<Roll>.bin`
- Qt widget: name input, roll input, progress bar, live preview with face box
- Signals: none currently

### ✅ Module 3 — Live Attendance Marking (`src/attendance/`)
- Opens webcam, loads YuNet + SFace + all `.bin` gallery files
- **Frame-skip detection**: YuNet runs every 3rd frame (caches last bounding box)
- **Downscaled detection**: YuNet scans a 320×240 copy → box scaled back to full-res
- SFace embedding extracted from full-res frame (quality unaffected)
- Cosine similarity vs gallery — threshold 0.363
- **Blink liveness**: Haar eye-cascade state machine per detected person
- Anti-duplicate: 300 s cooldown per roll number
- Logs to `attendance.csv` with score + timestamp
- Auto-closes ~1.5 s after a "PRESENT" verdict

### ✅ MainMenu (`src/MainMenu.cpp`)
- Tab widget with "Register Face", "Mark Attendance", "View Records" (stub), "Export" (stub)
- Passes `PROJECT_SOURCE_DIR`-based paths to child modules
- All module paths are absolute at build time — no POST_BUILD copy needed

### ⬜ Module 0 — Database layer (`src/db/`)
- Tables planned: `students`, `attendance_records`
- SQLite amalgamation (single .c/.h drop-in, no external install)
- Not yet implemented

### ⬜ Module 4 — Records Viewer (`src/records/`)
- Qt table widget pulling from SQLite (or CSV)
- Filter by date / student, attendance percentage

### ⬜ Module 5 — CSV Export (`src/export/`)
- Reads from DB, writes CSV
- Triggered by button in Records Viewer

---

## Key decisions made

### Model format
- **YuNet + SFace** (ONNX) instead of Haar Cascade + LBPH
- Better accuracy, no training for detection, single unified pipeline
- ONE `cv::FaceDetectorYN` model + ONE `cv::FaceRecognizerSF` model for everything

### Gallery storage
- One `.bin` file per student: `<Name>_<Roll>.bin`
- Binary format: `[rows(int32), cols(int32), data(float32)…]`
- Each file stores N × 128 averaged embedding matrix (currently 1 row per person)

### Path resolution
- `PROJECT_SOURCE_DIR` compile-define set in `CMakeLists.txt`
- All paths are absolute at compile time — no relative path assumptions
- No POST_BUILD copy step — resources live only in source tree
- `attendance.csv` written to project root (survives `rm -rf build`)

### Blink liveness
- Implemented using `haarcascade_eye.xml` (Haar cascade, same repo)
- Three-state machine: eyes-open → eyes-closed → eyes-open → verified
- Runs per-detected-person state tracked by student index

### Frame-skip + downscaled detection
- YuNet runs on 320×240 internal crop, coordinates scaled back to full-res
- Detection runs every 3rd frame, cached box reused in between
- SFace embedding extracted from full-resolution frame (no quality loss)

### Liveness detection
- Blink state machine implemented (not descoped as earlier planned)
- Could be disabled by removing the `if (!blinked)` branch

### Build
```bash
mkdir build && cd build
cmake ..
cmake --build . -j4
```
No special flags needed — OpenCV from Homebrew includes the DNN models.

---

## What to change in this file (for teammates)
When you finish a module, change ⬜ to ✅ and add a one-line summary.
If any major decision is made, update the relevant section.
