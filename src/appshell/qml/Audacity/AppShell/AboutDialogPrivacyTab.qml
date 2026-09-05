/*
* Audacity: A Digital Audio Editor
*/
import QtQuick 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents

import Audacity.AppShell

ColumnLayout {
    id: root

    required property AboutModel model

    spacing: prv.contentSpacing

    QtObject {
        id: prv

        readonly property int versionTextSpacing: 12
        readonly property int contentSpacing: 16
        readonly property int contentMargin: 16
        readonly property int contentTextMargin: 12

        readonly property string privacyTitle: qsTrc("appshell/about", "Privacy")
        readonly property string privacySubtitle: qsTrc("appshell/about", "Kdacity works entirely offline. It has no accounts, no cloud storage, no update checks, no crash reporting and no usage tracking.<br>Nothing you record or edit ever leaves this computer.")
    }

    Column {
        Layout.fillWidth: true
        Layout.preferredHeight: implicitHeight

        spacing: prv.versionTextSpacing

        StyledTextLabel {
            width: parent.width

            horizontalAlignment: Text.AlignHCenter

            text: prv.privacyTitle
            font: ui.theme.tabBoldFont
        }

        StyledTextLabel {
            width: parent.width

            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.RichText

            text: prv.privacySubtitle
            font: ui.theme.bodyFont
        }
    }

    StyledFlickable {
        id: legalFlickable

        Layout.fillWidth: true
        Layout.fillHeight: true

        contentHeight: gplContainer.height

        Rectangle {
            id: gplContainer

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: prv.contentMargin
            anchors.rightMargin: prv.contentMargin

            height: gplInner.height + prv.contentTextMargin * 2

            color: ui.theme.backgroundSecondaryColor

            Column {
                id: gplInner

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right

                anchors.margins: prv.contentTextMargin

                StyledTextLabel {
                    width: parent.width

                    horizontalAlignment: Text.AlignLeft
                    textFormat: Text.RichText
                    wrapMode: Text.WordWrap

                    text: root.model.gplText()

                    font: ui.theme.bodyFont
                }
            }
        }
    }
}
