#pragma once
#include <string>

class QWidget;

// Generates a well-formatted, multi-page PDF attendance report: one section
// per course, each listing every enrolled student with their computed
// attendance % and status, sorted by course. Replaces the old flat CSV
// export (see AdminDashboard's "Export Reports" card).
namespace AttendanceReportExporter {

// Builds the PDF at outputPath. Returns true on success. If a QWidget parent
// is supplied it's only used to anchor any Qt error dialogs the caller may
// want to show; this function itself does not show UI.
bool exportPdf(const std::string& outputPath);

} // namespace AttendanceReportExporter
