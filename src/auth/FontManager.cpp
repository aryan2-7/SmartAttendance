#include "FontManager.h"
#include <QFontDatabase>

static bool fontsLoaded = false;
static void ensureFonts() {
    if (fontsLoaded) return;
    QFontDatabase::addApplicationFont(":/fonts/fonts/Montserrat-VariableFont_wght.ttf");
    QFontDatabase::addApplicationFont(":/fonts/fonts/Playfair-VariableFont_opsz,wdth,wght.ttf");
    QFontDatabase::addApplicationFont(":/fonts/fonts/SortsMillGoudy-Regular.ttf");
    fontsLoaded = true;
}

const QString FONT_FAMILY = "Montserrat";

QFont FontManager::appFont(int size)
{
    ensureFonts();
    return QFont(FONT_FAMILY, size);
}

QFont FontManager::headingFont(int size)
{
    ensureFonts();
    QFont font(FONT_FAMILY, size);
    font.setBold(true);
    return font;
}

QFont FontManager::titleFont(int size)
{
    ensureFonts();
    QFont font(FONT_FAMILY, size);
    font.setBold(true);
    return font;
}

QFont FontManager::buttonFont(int size)
{
    ensureFonts();
    QFont font(FONT_FAMILY, size);
    font.setPointSize(size);
    font.setWeight(QFont::Medium);
    return font;
}