/*
    SPDX-FileCopyrightText: 2016 Rafał Rzepecki <divided.mind@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

*/

#ifndef KWHEXVIEW_H
#define KWHEXVIEW_H

#include <QPlainTextEdit>

class KWHexView : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit KWHexView(QWidget* parent = nullptr);
    void setData(const QByteArray &ba);

protected:
    void resizeEvent(QResizeEvent* e) override;

private:
    QByteArray data;

    int calculateStride();
    void showData();
};

#endif // KWHEXVIEW_H
