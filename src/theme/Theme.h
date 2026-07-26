#pragma once
#include <QString>

namespace Theme
{
// Backgrounds
inline const QString Card    = "#0E1016";   // outer window / page background
inline const QString Surface = "#171A22";   // buttons, header bars, panel backgrounds
inline const QString Input   = "#12151C";   // search box / table background
inline const QString Hover   = "#1C2029";   // hover state for buttons/rows

// Borders
inline const QString Border  = "#262A35";   // subtle dividing lines / card borders

// Text
inline const QString Primary   = "#ECE6D6"; // main text, headings, student names (warm cream)
inline const QString Secondary = "#9A9DA8"; // subtitles, dates, "Total Registered Students"
inline const QString Muted     = "#5C606B"; // placeholder text

// Accents / status colors
inline const QString Gold   = "#C9A227"; // accent: titles, focus ring, "Edit", left highlight bar
inline const QString Danger = "#B85C5C"; // "Delete", absent status (muted terracotta red)
inline const QString Success = "#7C9473"; // present status (sage green, if you need it elsewhere)
inline const QString Warning = "#D4A94C"; // late status (warm amber)
}