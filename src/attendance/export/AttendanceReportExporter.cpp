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
#include <QFontMetrics>
#include <QString>
#include <QDateTime>
#include <QColor>
#include <QMarginsF>
#include <QRect>

namespace {

// Layout constants, in points (QPdfWriter's default unit).
constexpr int kMargin = 40;
constexpr int kRowHeight = 24;
constexpr int kHeaderRowHeight = 28;
constexpr int kTitleGap = 20;
constexpr int kSectionGap = 30;

// Column layout for the student table.
struct ColumnLayout {
    QRect rollCol, nameCol, pctCol, statusCol;
    int endX;
};

ColumnLayout computeColumns(int usableLeft, int usableWidth, int rowHeight) {
    ColumnLayout c;

    // Give Attendance % enough room so the header is not clipped.
    // The exact ratios are a bit more balanced than before.
    int rollW   = static_cast<int>(usableWidth * 0.12);
    int nameW   = static_cast<int>(usableWidth * 0.42);
    int pctW    = static_cast<int>(usableWidth * 0.24);
    int statusW = usableWidth - rollW - nameW - pctW;

    int rollX = usableLeft;
    int nameX = rollX + rollW;
    int pctX = nameX + nameW;
    int statusX = pctX + pctW;
    int endX = usableLeft + usableWidth;

    c.rollCol = QRect(rollX, 0, rollW, rowHeight);
    c.nameCol = QRect(nameX, 0, nameW, rowHeight);
    c.pctCol = QRect(pctX, 0, pctW, rowHeight);
    c.statusCol = QRect(statusX, 0, statusW, rowHeight);
    c.endX = endX;
    return c;
}

// Draws text within a column rect at the given y, with a small inset and
// safe eliding so content never bleeds into the next column.
void drawInColumn(QPainter &painter, const QRect &col, int y, int rowHeight,
                  const QString &text, Qt::Alignment align) {
    QRect rect(col.x() + 4, y, col.width() - 8, rowHeight);

    QFontMetrics fm(painter.font());
    QString clipped = fm.elidedText(text, Qt::ElideRight, rect.width());

    painter.drawText(rect,
                     static_cast<int>(align) | Qt::AlignVCenter | Qt::TextSingleLine,
                     clipped);
}

void drawTableHeader(QPainter &painter, int &y, const ColumnLayout &cols, int usableLeft, int usableWidth) {
    QFont headerFont = painter.font();
    headerFont.setBold(true);
    headerFont.setPointSize(10);
    painter.setFont(headerFont);
    painter.fillRect(usableLeft, y, usableWidth, kHeaderRowHeight, QColor("#EEEEEE"));
    drawInColumn(painter, cols.rollCol, y, kHeaderRowHeight, "Roll No", Qt::AlignLeft);
    drawInColumn(painter, cols.nameCol, y, kHeaderRowHeight, "Student Name", Qt::AlignLeft);
    drawInColumn(painter, cols.pctCol, y, kHeaderRowHeight, "Attendance %", Qt::AlignHCenter);
    drawInColumn(painter, cols.statusCol, y, kHeaderRowHeight, "Status", Qt::AlignLeft);
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

    ColumnLayout cols = computeColumns(usableLeft, usableWidth, kRowHeight);

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

        // Start a new page for each course after the first.
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

            drawInColumn(painter, cols.rollCol, y, kRowHeight,
                QString::number(p.percentageRollNumber), Qt::AlignLeft);
            drawInColumn(painter, cols.nameCol, y, kRowHeight,
                QString::fromStdString(p.percentageStudentName), Qt::AlignLeft);
            drawInColumn(painter, cols.pctCol, y, kRowHeight,
                QString::number(p.calculatedPercentage, 'f', 1) + "%", Qt::AlignRight);

            painter.setPen(belowThreshold ? QColor("#B85C5C") : QColor("#5F8F55"));
            drawInColumn(painter, cols.statusCol, y, kRowHeight,
                belowThreshold ? "Below Min." : "OK", Qt::AlignLeft);
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