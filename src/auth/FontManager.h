#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include <QFont>
class FontManager {
public:
    static QFont appFont(int size = 12);
    static QFont headingFont(int size = 16);
    static QFont titleFont(int size = 24);
    static QFont buttonFont(int size = 12);
};

#endif