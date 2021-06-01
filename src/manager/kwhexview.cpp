/*
    SPDX-FileCopyrightText: 2016 Rafał Rzepecki <divided.mind@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

*/

#include "kwhexview.h"

#include <QTextStream>

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

    ts <<
          Qt::hex
       << qSetFieldWidth(2) << qSetPadChar(QLatin1Char('0'));

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

    ts << Qt::left;

    const auto stride = calculateStride();
    const auto hexwidth = stride * 2 + (stride / hexStride) + 1;

    for (auto it = data.begin(); it < data.end(); it += stride) {
        auto end = qMin(it + stride, data.end());
        ts << qSetFieldWidth(hexwidth) << toHex(it, end);
        ts << qSetFieldWidth(0) << toText(it, end) << QLatin1Char('\n');
    }
    ts.flush();
    setPlainText(text);
}

void KWHexView::resizeEvent(QResizeEvent* e)
{
    QPlainTextEdit::resizeEvent(e);
    if (e->size() != e->oldSize()) showData();
}
