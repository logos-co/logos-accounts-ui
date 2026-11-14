import QtQuick
import QtQuick.Layouts

import Logos.Theme
import Logos.Controls

// Set, change or remove the account relay publishes go through. The choice is
// kept beside the vault, so it outlives the app.
Sheet {
    property var store: null

    id: root
    objectName: "relaySheet"
    title: root.store.backend.relayAvailable ? qsTr("Change account relay")
                                             : qsTr("Set account relay")
    confirmText: qsTr("Save")
    confirmEnabled: (root.peer !== "" || root.address !== "") && !root.unchanged
    backend: root.store.backend
    input: peerField.field

    readonly property string peer: peerField.text.trim()
    readonly property string address: addressField.text.trim()
    readonly property bool unchanged: root.peer === root.store.backend.relayPeer
                                      && root.address === root.store.backend.relayAddress

    function reset() {
        peerField.text = root.store.backend.relayPeer
        addressField.text = root.store.backend.relayAddress
    }

    onConfirmed: {
        root.send("setRelay")
        root.store.backend.setRelay(root.peer, root.address)
    }
    onLanded: root.close()
    onCancelled: root.close()

    LogosText {
        Layout.fillWidth: true
        text: qsTr("An account relay publishes updates to the registry and pays the fee, so an account needs no funds. Anyone can run one: it can decline an update, never change it.")
        textFormat: Text.PlainText
        wrapMode: Text.Wrap
        color: Theme.palette.textSecondary
    }

    Field {
        id: peerField
        objectName: "relayPeerField"
        Layout.fillWidth: true
        label: qsTr("Peer id")
        mono: true
        placeholderText: "16Uiu2HAm…"
        help: qsTr("The relay's libp2p identity, from whoever runs it.")
        onAccepted: addressField.field.forceActiveFocus()
        onTextChanged: root.problem = ""
    }

    Field {
        id: addressField
        objectName: "relayAddressField"
        Layout.fillWidth: true
        label: qsTr("Address")
        mono: true
        placeholderText: "/ip4/203.0.113.7/tcp/9010"
        help: qsTr("Where it listens, as a libp2p TCP address. One that ends in /p2p/ and the peer id fills in both.")
        onAccepted: root.submit()
        onTextChanged: root.problem = ""
    }

    RowLayout {
        Layout.fillWidth: true
        visible: root.store.backend.relayAvailable
        spacing: Theme.spacing.medium

        HelpText {
            Layout.fillWidth: true
            body: qsTr("Without a relay, every publish is paid with your own funds.")
        }
        LogosButton {
            objectName: "relayRemoveButton"
            text: qsTr("Remove relay")
            enabled: root.awaiting === ""
            onClicked: {
                root.send("setRelay")
                root.store.backend.setRelay("", "")
            }
        }
    }
}
