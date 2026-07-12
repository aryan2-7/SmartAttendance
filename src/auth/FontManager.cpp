#include "FontManager.h"

namespace {
const QString FONT_FAMILY = "";
}

QFont FontManager::appFont(int size)
{
    return QFont(FONT_FAMILY, size);
}

QFont FontManager::headingFont(int size)
{
    QFont font(FONT_FAMILY, size);
    font.setBold(true);
    return font;
}

QFont FontManager::titleFont(int size)
{
    QFont font(FONT_FAMILY, size);
    font.setBold(true);
    return font;
}

QFont FontManager::buttonFont(int size)
{
    QFont font(FONT_FAMILY, size);
    font.setPointSize(size);
    font.setWeight(QFont::Medium);
    return font;
}