#include "AttendanceReportExporter.h"
#include "../../db/Database.h"
#include "../../db/DbPath.h"
#include "../../db/SubjectDAO.h"
#include "../../db/AttendanceDAO.h"

#include <QPdfWriter>
#include <QPainter>
#include <QPageSize>
#include <QPageLayout>
#include <QFont>
#include <QString>
#include <QDateTime>
#include <QColor>

namespace {

// Layout constants, in points (QPdfWriter's default unit).
constexpr int kMargin = 40;
constexpr int kRowHeight = 24;
constexpr int kHeaderRowHeight = 28;
constexpr int kTitleGap = 20;
constexpr int kSectionGap = 30;

// Column layout for the student table, as fractions of the usable width.
struct ColumnLayout {
    int rollX, nameX, pctX, statusX, endX;
};

ColumnLayout computeColumns(int usableLeft, int usableWidth) {
    ColumnLayout c;
    c.rollX = usableLeft;
    c.nameX = usableLeft + static_cast<int>(usableWidth * 0.15);
    c.pctX = usableLeft + static_cast<int>(usableWidth * 0.65);
    c.statusX = usableLeft + static_cast<int>(usableWidth * 0.80);
    c.endX = usableLeft + usableWidth;
    return c;
}

void drawTableHeader(QPainter &painter, int &y, const ColumnLayout &cols, int usableLeft, int usableWidth) {
    QFont headerFont = painter.font();
    headerFont.setBold(true);
    headerFont.setPointSize(10);
    painter.setFont(headerFont);
    painter.fillRect(usableLeft, y, usableWidth, kHeaderRowHeight, QColor("#EEEEEE"));
    painter.drawText(cols.rollX + 4, y + kHeaderRowHeight - 8, "Roll No");
    painter.drawText(cols.nameX + 4, y + kHeaderRowHeight - 8, "Student Name");
    painter.drawText(cols.pctX + 4, y + kHeaderRowHeight - 8, "Attendance %");
    painter.drawText(cols.statusX + 4, y + kHeaderRowHeight - 8, "Status");
    y += kHeaderRowHeight;
    QFont bodyFont = painter.font();
    bodyFont.setBold(false);
    bodyFont.setPointSize(10);
    painter.setFont(bodyFont);
}

} // namespace

bool AttendanceReportExporter::exportPdf(const std::string& outputPath) {
    Database db(appDbPath());
    db.initializeTables();
    SubjectDAO subjectDAO(db.getConnection());
    AttendanceDAO attendanceDAO(db.getConnection());

    QPdfWriter writer(QString::fromStdString(outputPath));
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageMargins(QMarginsF(kMargin, kMargin, kMargin, kMargin));
    writer.setResolution(96);

    QPainter painter(&writer);
    if (!painter.isActive()) {
        return false;
    }

    QRect pageRect = painter.viewport();
    int usableLeft = 0;
    int usableWidth = pageRect.width();
    int pageBottom = pageRect.height();

    ColumnLayout cols = computeColumns(usableLeft, usableWidth);

    // Report title / generated-on timestamp.
    QFont titleFont = painter.font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    int y = 0;
    painter.drawText(usableLeft, y + 24, "Attendance Report — by Course");
    y += 24 + 8;

    QFont subFont = painter.font();
    subFont.setPointSize(9);
    subFont.setBold(false);
    painter.setFont(subFont);
    painter.setPen(QColor("#666666"));
    painter.drawText(usableLeft, y + 14,
        "Generated " + QDateTime::currentDateTime().toString("MMM d, yyyy 'at' h:mm AP"));
    painter.setPen(QColor("#000000"));
    y += 14 + kTitleGap;

    auto subjects = subjectDAO.getAllSubjects();
    if (subjects.empty()) {
        QFont noneFont = painter.font();
        noneFont.setPointSize(11);
        painter.setFont(noneFont);
        painter.drawText(usableLeft, y + 20, "No courses found.");
        painter.end();
        return true;
    }

    for (size_t si = 0; si < subjects.size(); ++si) {
        const auto &subject = subjects[si];

        // Start a new page for each course after the first, so each course's
        // list is self-contained and easy to hand out separately if needed.
        if (si > 0) {
            writer.newPage();
            y = 0;
        }

        QFont courseFont = painter.font();
        courseFont.setPointSize(14);
        courseFont.setBold(true);
        painter.setFont(courseFont);
        QString courseHeading = QString::fromStdString(subject.subjectCode + " — " + subject.subjectName);
        painter.drawText(usableLeft, y + 20, courseHeading);
        y += 20 + 4;

        QFont metaFont = painter.font();
        metaFont.setPointSize(9);
        metaFont.setBold(false);
        painter.setFont(metaFont);
        painter.setPen(QColor("#666666"));
        painter.drawText(usableLeft, y + 14,
            QString("Semester %1 • %2 • Minimum attendance: %3%")
                .arg(subject.subjectSemester)
                .arg(QString::fromStdString(subject.subjectDepartment))
                .arg(subject.subjectMinAttendance));
        painter.setPen(QColor("#000000"));
        y += 14 + kTitleGap;

        auto percentages = attendanceDAO.getSubjectAttendancePercentage(subject.subjectId);

        if (percentages.empty()) {
            painter.setFont(metaFont);
            painter.drawText(usableLeft, y + 16, "No enrolled students or attendance data for this course.");
            y += 16 + kSectionGap;
            continue;
        }

        drawTableHeader(painter, y, cols, usableLeft, usableWidth);

        for (const auto &p : percentages) {
            // Paginate within a course if the table runs past the page.
            if (y + kRowHeight > pageBottom - kMargin) {
                writer.newPage();
                y = 0;
                drawTableHeader(painter, y, cols, usableLeft, usableWidth);
            }

            bool belowThreshold = p.calculatedPercentage < subject.subjectMinAttendance;

            painter.drawText(cols.rollX + 4, y + kRowHeight - 7, QString::number(p.percentageRollNumber));
            painter.drawText(cols.nameX + 4, y + kRowHeight - 7, QString::fromStdString(p.percentageStudentName));
            painter.drawText(cols.pctX + 4, y + kRowHeight - 7, QString::number(p.calculatedPercentage, 'f', 1) + "%");

            painter.setPen(belowThreshold ? QColor("#B85C5C") : QColor("#5F8F55"));
            painter.drawText(cols.statusX + 4, y + kRowHeight - 7,
                belowThreshold ? "Below Min." : "OK");
            painter.setPen(QColor("#000000"));

            painter.setPen(QColor("#DDDDDD"));
            painter.drawLine(usableLeft, y + kRowHeight, cols.endX, y + kRowHeight);
            painter.setPen(QColor("#000000"));

            y += kRowHeight;
        }

        y += kSectionGap;
    }

    painter.end();
    return true;
}
