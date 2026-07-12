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
├── fonts.qrc
├── src/
│   ├── main.cpp                            # entry point: shows WelcomeWindow
│   ├── auth/
│   │   ├── WelcomeWindow.h                 ✅ done
│   │   ├── WelcomeWindow.cpp               ✅ done
│   │   ├── FontManager.h                   ✅ done
│   │   └── FontManager.cpp                 ✅ done
│   ├── registration/
│   │   ├── StudentRegistrationWindow.h     ✅ done
│   │   └── StudentRegistrationWindow.cpp   ✅ done
│   ├── attendance/
│   │   ├── AttendanceWindow.h              ✅ done
│   │   └── AttendanceWindow.cpp            ✅ done
│   ├── admin/
│   │   ├── AdministratorWindow.h           ✅ done
│   │   ├── AdministratorWindow.cpp         ✅ done
│   │   ├── AdminDashboard.h                ✅ done
│   │   ├── AdminDashboard.cpp              ✅ done
│   │   ├── ManageStudentsWindow.h          ✅ done
│   │   ├── ManageStudentsWindow.cpp        ✅ done
│   │   ├── AttendanceRecordsWindow.h       ✅ done
│   │   ├── AttendanceRecordsWindow.cpp     ✅ done
│   │   ├── CircularProgress.h              ✅ done
│   │   └── CircularProgress.cpp            ✅ done
│   └── db/
│       ├── db.h                            ✅ implemented
│       └── db.cpp                          ✅ implemented
├── resources/
│   ├── models/                             # ONNX models (Git LFS)
│   │   ├── face_detection_yunet_2023mar.onnx
│   │   └── face_recognition_sface_2021dec.onnx
│   ├── trained_models/                     # .bin files per registered user
│   └── icons/
├── docs/
│   └── architecture.md
└── fonts/
```

---

## Module status

### ✅ Module 1 — Welcome / Auth (`src/auth/`)
- WelcomeWindow with three action cards: Student Registration, Mark Attendance, Administrator Login
- FontManager loads embedded Montserrat/Playfair fonts from Qt resources
- Administrator login via SQLite with prepared statements

### ✅ Module 2 — Face Registration (`src/registration/`)
- Opens webcam, loads **YuNet** immediately (face box visible from window open)
- User enters name + roll, clicks Start → loads **SFace** for embedding extraction
- Captures 50 embeddings (128-d feature vectors) from aligned face crops
- Averages embeddings, saves to `resources/trained_models/<Name>_<Roll>.bin`
- Qt widget: name input, roll input, progress bar, live preview with face box

### ✅ Module 3 — Live Attendance Marking (`src/attendance/`)
- Opens webcam, loads YuNet + SFace + all `.bin` gallery files
- Cosine similarity vs gallery — threshold 0.363
- Anti-duplicate: 300 s cooldown per roll number
- Logs to SQLite database

### ✅ Module 4 — Admin Dashboard (`src/admin/`)
- Administrator login with username/password
- Dashboard with attendance circle, weekly bar chart, recent check-ins
- Student management: add, edit, delete students
- Attendance records: daily roster with Present/Late/Absent status
- CSV export of attendance data

### ✅ Module 5 — Database layer (`src/db/`)
- Tables: `users`, `students`, `attendance`
- SQLite with parameterized queries (no SQL injection)
- Foreign key cascading on rollNumber changes

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
