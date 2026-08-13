/*
 *  SPDX-FileCopyrightText: 2026 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

QQC2.Dialog {
    title: i18n("New Wallet")

    property alias walletName: textField.text

    ColumnLayout {

        QQC2.Label {
            id: textFieldLabel

            Layout.fillWidth: true

            text: i18n("Choose a name for the new wallet:")
        }

        QQC2.TextField {
            id: textField

            Layout.fillWidth: true

            Accessible.labelledBy: textFieldLabel
        }
    }

    standardButtons: QQC2.Dialog.Ok | QQC2.Dialog.Cancel
}
