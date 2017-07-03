/*
 * Copyright 2016  Rafał Rzepecki <divided.mind@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "kwhexview.h"

#include <qtextstream.h>

KWHexView::KWHexView(QWidget* parent): QPlainTextEdit(parent)
{
    setReadOnly(true);
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    setWordWrapMode(QTextOption::NoWrap);
}

enum { hexStride = 4 };

template<class It>
static QString toHex(It it, It end)
{
    QString text;
    QTextStream ts(&text);

    ts << hex << qSetFieldWidth(2) << qSetPadChar(QLatin1Char('0'));

    while (it < end) {
        const auto sEnd = qMin(it + hexStride, end);
        while (it < sEnd)
            ts << static_cast<quint8>(*(it++));
        ts << qSetFieldWidth(0) << " " << qSetFieldWidth(2);
    }

    return text;
}

template<class It>
static QString toText(It begin, It end)
{
    QString text = QString::fromLatin1(begin, end - begin);

    for (auto &ch: text)
        if (!ch.isPrint()) ch = QLatin1Char('.');

    return text;
}

int KWHexView::calculateStride()
{
    const auto w = viewport()->width();
    const auto em = fontMetrics().averageCharWidth();
    const auto chars =  w / em - 1;
    auto stride =  chars / 3 / hexStride * hexStride;

    while (stride * 3 + (stride / hexStride) + 1 > chars)
        stride -= hexStride;

    return qMax(static_cast<int>(hexStride), stride);
}

void KWHexView::setData(const QByteArray& ba)
{
    data = ba;
    showData();
}

void KWHexView::showData()
{
    QString text;
    QTextStream ts(&text);
    ts << left;

    const auto stride = calculateStride();
    const auto hexwidth = stride * 2 + (stride / hexStride) + 1;

    for (auto it = data.begin(); it < data.end(); it += stride) {
        auto end = qMin(it + stride, data.end());
        ts << qSetFieldWidth(hexwidth) << toHex(it, end);
        ts << qSetFieldWidth(0) << toText(it, end) << endl;
    }

    setPlainText(text);
}

void KWHexView::resizeEvent(QResizeEvent* e)
{
    QPlainTextEdit::resizeEvent(e);
    if (e->size() != e->oldSize()) showData();
}
