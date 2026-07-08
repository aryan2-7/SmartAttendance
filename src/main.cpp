#include <QApplication>

#include "auth/FontManager.h"
#include "admin/AdminDashboard.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setFont(FontManager::appFont(12));

    AdminDashboard w;
    w.show();

    return a.exec();
}