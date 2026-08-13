/*
 *  SPDX-FileCopyrightText: 2026 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami

KCM.SimpleKCM {

    header: ColumnLayout {

        Kirigami.InlineMessage {
            id: errorMessage

            Layout.fillWidth: true
            type: Kirigami.MessageType.Error
            position: Kirigami.InlineMessage.Position.Header
            showCloseButton: true

            Connections {
                target: kcm

                function onError(message: string) {
                    errorMessage.visible = true;
                    errorMessage.text = message;
                }
            }
        }

        QQC2.CheckBox {
            id: enabledCheckbox

            Layout.fillWidth: true

            text: i18n("Enable the KDE wallet subsystem")
            leftPadding: Kirigami.Units.smallSpacing

            checked: kcm.settings.kWalletDEnabled
            onToggled: kcm.settings.kWalletDEnabled = checked

            KCM.SettingStateBinding {
                configObject: kcm.settings
                settingName: "KWalletDEnabled"
            }
        }
    }

    NewWalletDialog {
        id: newWalletDialog

        onAccepted: {
            kcm.createWallet(walletName);
        }
    }

    Kirigami.Form {

        enabled: enabledCheckbox.checked

        Layout.fillWidth: true
        Kirigami.FormGroup {
            title: i18nc("@title:group", "Close Behavior")

            Kirigami.FormEntry {
                title: i18n("Close Wallet:")
                contentItem: QQC2.CheckBox {
                    text: i18n("When last application stops using it")
                    checked: !kcm.settings.leaveOpen
                    onCheckedChanged: kcm.settings.leaveOpen = !checked

                    KCM.SettingStateBinding {
                        configObject: kcm.settings
                        settingName: "LeaveOpen"
                    }
                }
            }

            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    id: unusedTimeoutCheckBox
                    text: i18n("When unused for:")
                    checked: kcm.settings.closeWhenIdle
                    onCheckedChanged: kcm.settings.closeWhenIdle = checked

                    KCM.SettingStateBinding {
                        configObject: kcm.settings
                        settingName: "CloseWhenIdle"
                    }
                }
            }

            Kirigami.FormEntry {
                contentItem: QQC2.SpinBox {
                    stepSize: 5
                    from: 1
                    to: 999

                    textFromValue: (value, locale) => {
                        return i18ncp("short for minute(s)", "%1 min", "%1 min", value);
                    }
                    valueFromText: (text, locale) => {
                        return Number.fromLocaleString(locale, text.replace(i18nc("short for minute(s)", "min"), ""));
                    }

                    value: kcm.settings.idleTimeout
                    onValueModified: kcm.settings.idleTimeout = value

                    KCM.SettingStateBinding {
                        configObject: kcm.settings
                        settingName: "IdleTimeout"
                        extraEnabledConditions: unusedTimeoutCheckBox.checked
                    }
                }
            }
        }

        Kirigami.FormGroup {
            title: i18nc("@title:group", "Automatic Wallet Selection")

            Kirigami.FormEntry {
                title: i18n("Default Wallet:")
                contentItem: RowLayout {
                    QQC2.ComboBox {
                        Layout.fillWidth: true

                        model: kcm.walletList

                        currentIndex: kcm.walletList.indexOf(kcm.settings.defaultWallet)

                        onActivated: {
                            kcm.settings.defaultWallet = currentText;
                        }

                        KCM.SettingStateBinding {
                            configObject: kcm.settings
                            settingName: "DefaultWallet"
                        }
                    }
                    QQC2.Button {
                        text: i18n("New…")

                        onClicked: {
                            newWalletDialog.open();
                        }
                    }
                }
            }

            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    id: useLocalWalletCheckBox
                    text: i18n("Use different default wallet for local passwords")
                    checked: !kcm.settings.useOneWallet
                    onCheckedChanged: {
                        kcm.settings.useOneWallet = !checked;

                        if (checked) {
                            kcm.settings.localWallet = localWalletCombo.valueAt(0);
                        } else {
                            kcm.settings.localWallet = kcm.settings.defaultLocalWalletValue;
                        }
                    }

                    KCM.SettingStateBinding {
                        configObject: kcm.settings
                        settingName: "UseOneWallet"
                    }
                }
            }

            Kirigami.FormEntry {
                contentItem: RowLayout {
                    QQC2.ComboBox {
                        id: localWalletCombo

                        Layout.fillWidth: true

                        model: kcm.walletList

                        currentIndex: {
                            return kcm.walletList.indexOf(kcm.settings.localWallet);
                        }

                        onActivated: {
                            kcm.settings.localWallet = currentText;
                        }

                        KCM.SettingStateBinding {
                            configObject: kcm.settings
                            settingName: "LocalWallet"
                            extraEnabledConditions: useLocalWalletCheckBox.checked
                        }
                    }
                    QQC2.Button {
                        text: i18n("New…")

                        onClicked: {
                            newWalletDialog.open();
                        }
                    }
                }
            }
        }

        Kirigami.FormGroup {
            title: i18nc("@title:group", "Wallet Manager")

            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    id: showManagerCheckbox
                    text: i18n("Show manager in system tray")
                    checked: kcm.settings.launchManager
                    onCheckedChanged: kcm.settings.launchManager = checked

                    KCM.SettingStateBinding {
                        configObject: kcm.settings
                        settingName: "LaunchManager"
                    }
                }
            }

            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    text: i18n("Hide system tray icon when last wallet closes")
                    checked: !kcm.settings.leaveManagerOpen
                    onCheckedChanged: kcm.settings.leaveManagerOpen = !checked

                    KCM.SettingStateBinding {
                        configObject: kcm.settings
                        settingName: "LeaveManagerOpen"
                        extraEnabledConditions: showManagerCheckbox.checked
                    }
                }
            }
        }

        Kirigami.FormGroup {
            title: i18nc("@title:group", "Secret Service")

            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    text: i18n("Use KWallet for Secret Service Interface")
                    checked: kcm.settings.fdoSecretsApiEnabled
                    onCheckedChanged: kcm.settings.fdoSecretsApiEnabled = checked

                    KCM.SettingStateBinding {
                        configObject: kcm.settings
                        settingName: "fdoSecretsApiEnabled"
                    }
                }
            }
        }
    }
}
