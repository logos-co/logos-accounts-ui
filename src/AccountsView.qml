import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: _d.bgColor

    QtObject {
        id: _d

        // Theme colors
        readonly property color bgColor: "#1e1e1e"
        readonly property color panelColor: "#2d2d2d"
        readonly property color inputColor: "#3c3c3c"
        readonly property color borderColor: "#555555"
        readonly property color textColor: "#e0e0e0"
        readonly property color labelColor: "#b0b0b0"
        readonly property color accentColor: "#4fc3f7"
        readonly property color successColor: "#66bb6a"
        readonly property color errorColor: "#ef5350"
        readonly property color buttonColor: "#0d47a1"
        readonly property color buttonHoverColor: "#1565c0"
        readonly property color separatorColor: "#444444"

        // Font sizes
        readonly property int titleSize: 18
        readonly property int sectionSize: 14
        readonly property int textSize: 12

        // Shared result text
        property string resultText: ""
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // Header
        Text {
            text: "Accounts Manager"
            color: _d.textColor
            font.pixelSize: _d.titleSize
            font.bold: true
        }

        // Status bar
        Rectangle {
            Layout.fillWidth: true
            height: 30
            color: _d.panelColor
            radius: 4

            Text {
                anchors.fill: parent
                anchors.leftMargin: 10
                verticalAlignment: Text.AlignVCenter
                text: "Status: " + backend.status
                color: _d.labelColor
                font.pixelSize: _d.textSize
            }
        }

        // Tab bar
        TabBar {
            id: tabBar
            Layout.fillWidth: true
            background: Rectangle { color: _d.panelColor; radius: 4 }

            TabButton {
                text: "Keys"
                font.pixelSize: _d.textSize
                contentItem: Text {
                    text: parent.text
                    color: tabBar.currentIndex === 0 ? _d.accentColor : _d.labelColor
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font: parent.font
                }
                background: Rectangle {
                    color: tabBar.currentIndex === 0 ? _d.inputColor : "transparent"
                    radius: 4
                }
            }
            TabButton {
                text: "Keystore"
                font.pixelSize: _d.textSize
                contentItem: Text {
                    text: parent.text
                    color: tabBar.currentIndex === 1 ? _d.accentColor : _d.labelColor
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font: parent.font
                }
                background: Rectangle {
                    color: tabBar.currentIndex === 1 ? _d.inputColor : "transparent"
                    radius: 4
                }
            }
            TabButton {
                text: "ExtKeystore"
                font.pixelSize: _d.textSize
                contentItem: Text {
                    text: parent.text
                    color: tabBar.currentIndex === 2 ? _d.accentColor : _d.labelColor
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font: parent.font
                }
                background: Rectangle {
                    color: tabBar.currentIndex === 2 ? _d.inputColor : "transparent"
                    radius: 4
                }
            }
        }

        // Tab content
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex

            // ===== Keys Tab =====
            ScrollView {
                id: keysScrollView
                clip: true
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                ScrollBar.vertical.policy: ScrollBar.AsNeeded

                ColumnLayout {
                    width: keysScrollView.availableWidth
                    spacing: 12

                    // -- Mnemonic Operations --
                    SectionHeader { text: "Mnemonic Operations" }

                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: mnemonicLengthInput
                            Layout.fillWidth: true
                            placeholderText: "Length (12, 15, 18, 21, 24) - leave empty for default"
                        }
                        StyledButton {
                            text: "Create Random Mnemonic"
                            onClicked: {
                                var len = mnemonicLengthInput.text
                                var result = len === "" ? backend.createRandomMnemonicDefault() : backend.createRandomMnemonic(parseInt(len))
                                mnemonicResult.text = result || "Failed to create mnemonic"
                            }
                        }
                    }
                    ResultLabel { text: "Mnemonic Result:" }
                    ResultArea { id: mnemonicResult }

                    Separator {}

                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: entropyLengthInput
                            Layout.fillWidth: true
                            placeholderText: "Length (12, 15, 18, 21, 24)..."
                        }
                        StyledButton {
                            text: "Length To Entropy Strength"
                            onClicked: {
                                var result = backend.lengthToEntropyStrength(parseInt(entropyLengthInput.text))
                                entropyResult.text = result.toString()
                            }
                        }
                    }
                    ResultLabel { text: "Entropy Strength Result:" }
                    ResultField { id: entropyResult }

                    // -- Key Operations --
                    SectionHeader { text: "Key Operations" }

                    ResultLabel { text: "Create Extended Key from Mnemonic:" }
                    StyledTextArea {
                        id: extKeyMnemonicInput
                        Layout.fillWidth: true
                        placeholderText: "Mnemonic phrase..."
                    }
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: extKeyPassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledButton {
                            text: "Create Extended Key"
                            onClicked: {
                                var result = backend.createExtKeyFromMnemonic(extKeyMnemonicInput.text, extKeyPassphraseInput.text)
                                extKeyResult.text = result || "Failed to create extended key"
                            }
                        }
                    }
                    ResultLabel { text: "Extended Key Result:" }
                    ResultArea { id: extKeyResult }

                    Separator {}

                    ResultLabel { text: "Derive Extended Key:" }
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: deriveExtKeyInput
                            Layout.fillWidth: true
                            placeholderText: "Extended Key..."
                        }
                        StyledInput {
                            id: deriveExtKeyPathInput
                            Layout.fillWidth: true
                            placeholderText: "Derivation Path (e.g., m/44'/60'/0'/0/0)..."
                        }
                        StyledButton {
                            text: "Derive"
                            onClicked: {
                                var result = backend.deriveExtKey(deriveExtKeyInput.text, deriveExtKeyPathInput.text)
                                deriveExtKeyResult.text = result || "Derivation failed"
                            }
                        }
                    }
                    ResultLabel { text: "Derived Key Result:" }
                    ResultArea { id: deriveExtKeyResult }

                    Separator {}

                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: extKeyToECDSAInput
                            Layout.fillWidth: true
                            placeholderText: "Extended Key..."
                        }
                        StyledButton {
                            text: "Ext Key To ECDSA"
                            onClicked: {
                                var result = backend.extKeyToECDSA(extKeyToECDSAInput.text)
                                extKeyToECDSAResult.text = result || "Conversion failed"
                            }
                        }
                    }
                    ResultLabel { text: "ECDSA Private Key Result:" }
                    ResultField { id: extKeyToECDSAResult }

                    Separator {}

                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: ecdsaToPublicKeyInput
                            Layout.fillWidth: true
                            placeholderText: "ECDSA Private Key (hex)..."
                        }
                        StyledButton {
                            text: "ECDSA To Public Key"
                            onClicked: {
                                var result = backend.ecdsaToPublicKey(ecdsaToPublicKeyInput.text)
                                ecdsaToPublicKeyResult.text = result || "Conversion failed"
                            }
                        }
                    }
                    ResultLabel { text: "Public Key Result:" }
                    ResultField { id: ecdsaToPublicKeyResult }

                    Separator {}

                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: publicKeyToAddressInput
                            Layout.fillWidth: true
                            placeholderText: "Public Key (hex)..."
                        }
                        StyledButton {
                            text: "Public Key To Address"
                            onClicked: {
                                var result = backend.publicKeyToAddress(publicKeyToAddressInput.text)
                                publicKeyToAddressResult.text = result || "Conversion failed"
                            }
                        }
                    }
                    ResultLabel { text: "Address Result:" }
                    ResultField { id: publicKeyToAddressResult }

                    Item { Layout.fillHeight: true }
                }
            }

            // ===== Keystore Tab =====
            ScrollView {
                id: keystoreScrollView
                clip: true
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                ScrollBar.vertical.policy: ScrollBar.AsNeeded

                ColumnLayout {
                    width: keystoreScrollView.availableWidth
                    spacing: 12

                    SectionHeader { text: "Keystore Operations" }

                    // Init Keystore
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: ksDirInput
                            Layout.fillWidth: true
                            placeholderText: "Keystore Directory..."
                            text: "/tmp/logos_keystore"
                        }
                        StyledInput {
                            id: ksScryptNInput
                            Layout.preferredWidth: 120
                            placeholderText: "Scrypt N"
                            text: "262144"
                        }
                        StyledInput {
                            id: ksScryptPInput
                            Layout.preferredWidth: 80
                            placeholderText: "Scrypt P"
                            text: "1"
                        }
                        StyledButton {
                            text: "Init Keystore"
                            onClicked: backend.initKeystore(ksDirInput.text, parseInt(ksScryptNInput.text), parseInt(ksScryptPInput.text))
                        }
                    }

                    Separator {}

                    StyledButton {
                        text: "Close Keystore"
                        onClicked: backend.closeKeystore()
                    }

                    Separator {}

                    // List Accounts
                    RowLayout {
                        spacing: 8
                        StyledButton {
                            text: "List Accounts"
                            onClicked: backend.refreshKeystoreAccounts()
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            height: 80
                            color: _d.inputColor
                            radius: 4
                            border.color: _d.borderColor
                            border.width: 1

                            ListView {
                                anchors.fill: parent
                                anchors.margins: 4
                                clip: true
                                model: backend.keystoreAccounts
                                delegate: TextInput {
                                    text: modelData
                                    color: _d.textColor
                                    font.pixelSize: _d.textSize
                                    width: parent ? parent.width : 0
                                    padding: 2
                                    readOnly: true
                                    selectByMouse: true
                                    selectedTextColor: _d.bgColor
                                    selectionColor: _d.accentColor
                                }
                            }
                        }
                    }

                    Separator {}

                    // New Account
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: ksNewPassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledButton {
                            text: "New Account"
                            onClicked: {
                                var result = backend.keystoreNewAccount(ksNewPassphraseInput.text)
                                if (result) ksNewPassphraseInput.text = ""
                            }
                        }
                    }

                    Separator {}

                    // Import Account
                    ResultLabel { text: "Import Account:" }
                    StyledTextArea {
                        id: ksImportKeyJSONInput
                        Layout.fillWidth: true
                        placeholderText: "Key JSON..."
                    }
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: ksImportPassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "Current Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledInput {
                            id: ksImportNewPassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "New Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledButton {
                            text: "Import"
                            onClicked: {
                                var result = backend.keystoreImport(ksImportKeyJSONInput.text, ksImportPassphraseInput.text, ksImportNewPassphraseInput.text)
                                if (result) { ksImportKeyJSONInput.text = ""; ksImportPassphraseInput.text = ""; ksImportNewPassphraseInput.text = "" }
                            }
                        }
                    }

                    Separator {}

                    // Export Account
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: ksExportAddressInput
                            Layout.fillWidth: true
                            placeholderText: "Address..."
                        }
                        StyledInput {
                            id: ksExportPassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledInput {
                            id: ksExportNewPassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "New Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledButton {
                            text: "Export"
                            onClicked: {
                                var result = backend.keystoreExport(ksExportAddressInput.text, ksExportPassphraseInput.text, ksExportNewPassphraseInput.text)
                                ksExportResult.text = result || "Export failed"
                            }
                        }
                    }
                    ResultLabel { text: "Export Result:" }
                    ResultArea { id: ksExportResult }

                    Separator {}

                    // Delete Account
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: ksDeleteAddressInput
                            Layout.fillWidth: true
                            placeholderText: "Address..."
                        }
                        StyledInput {
                            id: ksDeletePassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledButton {
                            text: "Delete Account"
                            onClicked: backend.keystoreDelete(ksDeleteAddressInput.text, ksDeletePassphraseInput.text)
                        }
                    }

                    Separator {}

                    // Has Address
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: ksHasAddressInput
                            Layout.fillWidth: true
                            placeholderText: "Address..."
                        }
                        StyledButton {
                            text: "Has Address"
                            onClicked: {
                                var result = backend.keystoreHasAddress(ksHasAddressInput.text)
                                ksHasAddressResult.text = result ? "true" : "false"
                            }
                        }
                    }
                    ResultLabel { text: "Has Address Result:" }
                    ResultField { id: ksHasAddressResult }

                    Separator {}

                    // Unlock / Lock
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: ksUnlockAddressInput
                            Layout.fillWidth: true
                            placeholderText: "Address..."
                        }
                        StyledInput {
                            id: ksUnlockPassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledButton {
                            text: "Unlock"
                            onClicked: backend.keystoreUnlock(ksUnlockAddressInput.text, ksUnlockPassphraseInput.text)
                        }
                    }
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: ksLockAddressInput
                            Layout.fillWidth: true
                            placeholderText: "Address..."
                        }
                        StyledButton {
                            text: "Lock"
                            onClicked: backend.keystoreLock(ksLockAddressInput.text)
                        }
                    }

                    Separator {}

                    // Timed Unlock
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: ksTimedUnlockAddressInput
                            Layout.fillWidth: true
                            placeholderText: "Address..."
                        }
                        StyledInput {
                            id: ksTimedUnlockPassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledInput {
                            id: ksTimedUnlockTimeoutInput
                            Layout.preferredWidth: 120
                            placeholderText: "Timeout (s)..."
                        }
                        StyledButton {
                            text: "Timed Unlock"
                            onClicked: backend.keystoreTimedUnlock(ksTimedUnlockAddressInput.text, ksTimedUnlockPassphraseInput.text, parseInt(ksTimedUnlockTimeoutInput.text))
                        }
                    }

                    Separator {}

                    // Update Passphrase
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: ksUpdateAddressInput
                            Layout.fillWidth: true
                            placeholderText: "Address..."
                        }
                        StyledInput {
                            id: ksUpdatePassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "Current Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledInput {
                            id: ksUpdateNewPassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "New Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledButton {
                            text: "Update Passphrase"
                            onClicked: backend.keystoreUpdate(ksUpdateAddressInput.text, ksUpdatePassphraseInput.text, ksUpdateNewPassphraseInput.text)
                        }
                    }

                    Separator {}

                    // Sign Hash
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: ksSignHashAddressInput
                            Layout.fillWidth: true
                            placeholderText: "Address..."
                        }
                        StyledInput {
                            id: ksSignHashHashInput
                            Layout.fillWidth: true
                            placeholderText: "Hash (hex)..."
                        }
                        StyledButton {
                            text: "Sign Hash"
                            onClicked: {
                                var result = backend.keystoreSignHash(ksSignHashAddressInput.text, ksSignHashHashInput.text)
                                ksSignHashResult.text = result || "Signing failed"
                            }
                        }
                    }
                    ResultLabel { text: "Signature Result:" }
                    ResultField { id: ksSignHashResult }

                    Separator {}

                    // Sign Hash With Passphrase
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: ksSignHashWPAddressInput
                            Layout.fillWidth: true
                            placeholderText: "Address..."
                        }
                        StyledInput {
                            id: ksSignHashWPPassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledInput {
                            id: ksSignHashWPHashInput
                            Layout.fillWidth: true
                            placeholderText: "Hash (hex)..."
                        }
                        StyledButton {
                            text: "Sign Hash With Passphrase"
                            onClicked: {
                                var result = backend.keystoreSignHashWithPassphrase(ksSignHashWPAddressInput.text, ksSignHashWPPassphraseInput.text, ksSignHashWPHashInput.text)
                                ksSignHashWPResult.text = result || "Signing failed"
                            }
                        }
                    }
                    ResultLabel { text: "Signature Result:" }
                    ResultField { id: ksSignHashWPResult }

                    Separator {}

                    // Import ECDSA
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: ksImportECDSAKeyInput
                            Layout.fillWidth: true
                            placeholderText: "Private Key (hex)..."
                        }
                        StyledInput {
                            id: ksImportECDSAPassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledButton {
                            text: "Import ECDSA"
                            onClicked: {
                                var result = backend.keystoreImportECDSA(ksImportECDSAKeyInput.text, ksImportECDSAPassphraseInput.text)
                                ksImportECDSAResult.text = result || "Import failed"
                            }
                        }
                    }
                    ResultLabel { text: "Import Result:" }
                    ResultField { id: ksImportECDSAResult }

                    Separator {}

                    // Sign Transaction
                    ResultLabel { text: "Sign Transaction:" }
                    StyledInput {
                        id: ksSignTxAddressInput
                        Layout.fillWidth: true
                        placeholderText: "Address..."
                    }
                    StyledTextArea {
                        id: ksSignTxJSONInput
                        Layout.fillWidth: true
                        placeholderText: "Transaction JSON..."
                    }
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: ksSignTxChainIDInput
                            Layout.fillWidth: true
                            placeholderText: "Chain ID (hex)..."
                        }
                        StyledButton {
                            text: "Sign Transaction"
                            onClicked: {
                                var result = backend.keystoreSignTx(ksSignTxAddressInput.text, ksSignTxJSONInput.text, ksSignTxChainIDInput.text)
                                ksSignTxResult.text = result || "Signing failed"
                            }
                        }
                    }
                    ResultLabel { text: "Signed Transaction Result:" }
                    ResultArea { id: ksSignTxResult }

                    Separator {}

                    // Sign Transaction With Passphrase
                    ResultLabel { text: "Sign Transaction With Passphrase:" }
                    StyledInput {
                        id: ksSignTxWPAddressInput
                        Layout.fillWidth: true
                        placeholderText: "Address..."
                    }
                    StyledInput {
                        id: ksSignTxWPPassphraseInput
                        Layout.fillWidth: true
                        placeholderText: "Passphrase..."
                        echoMode: TextInput.Password
                    }
                    StyledTextArea {
                        id: ksSignTxWPJSONInput
                        Layout.fillWidth: true
                        placeholderText: "Transaction JSON..."
                    }
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: ksSignTxWPChainIDInput
                            Layout.fillWidth: true
                            placeholderText: "Chain ID (hex)..."
                        }
                        StyledButton {
                            text: "Sign Transaction With Passphrase"
                            onClicked: {
                                var result = backend.keystoreSignTxWithPassphrase(ksSignTxWPAddressInput.text, ksSignTxWPPassphraseInput.text, ksSignTxWPJSONInput.text, ksSignTxWPChainIDInput.text)
                                ksSignTxWPResult.text = result || "Signing failed"
                            }
                        }
                    }
                    ResultLabel { text: "Signed Transaction Result:" }
                    ResultArea { id: ksSignTxWPResult }

                    Separator {}

                    // Find
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: ksFindAddressInput
                            Layout.fillWidth: true
                            placeholderText: "Address..."
                        }
                        StyledInput {
                            id: ksFindUrlInput
                            Layout.fillWidth: true
                            placeholderText: "URL..."
                        }
                        StyledButton {
                            text: "Find"
                            onClicked: {
                                var result = backend.keystoreFind(ksFindAddressInput.text, ksFindUrlInput.text)
                                ksFindResult.text = result || "Not found"
                            }
                        }
                    }
                    ResultLabel { text: "Find Result:" }
                    ResultField { id: ksFindResult }

                    Item { Layout.fillHeight: true }
                }
            }

            // ===== ExtKeystore Tab =====
            ScrollView {
                id: extKeystoreScrollView
                clip: true
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                ScrollBar.vertical.policy: ScrollBar.AsNeeded

                ColumnLayout {
                    width: extKeystoreScrollView.availableWidth
                    spacing: 12

                    SectionHeader { text: "Extended Keystore Operations" }

                    // Init Extended Keystore
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: eksDirInput
                            Layout.fillWidth: true
                            placeholderText: "Keystore Directory..."
                            text: "/tmp/logos_keystore"
                        }
                        StyledInput {
                            id: eksScryptNInput
                            Layout.preferredWidth: 120
                            placeholderText: "Scrypt N"
                            text: "262144"
                        }
                        StyledInput {
                            id: eksScryptPInput
                            Layout.preferredWidth: 80
                            placeholderText: "Scrypt P"
                            text: "1"
                        }
                        StyledButton {
                            text: "Init Extended Keystore"
                            onClicked: backend.initExtKeystore(eksDirInput.text, parseInt(eksScryptNInput.text), parseInt(eksScryptPInput.text))
                        }
                    }

                    Separator {}

                    StyledButton {
                        text: "Close Extended Keystore"
                        onClicked: backend.closeExtKeystore()
                    }

                    Separator {}

                    // List Accounts
                    RowLayout {
                        spacing: 8
                        StyledButton {
                            text: "List Accounts"
                            onClicked: backend.refreshExtKeystoreAccounts()
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            height: 80
                            color: _d.inputColor
                            radius: 4
                            border.color: _d.borderColor
                            border.width: 1

                            ListView {
                                anchors.fill: parent
                                anchors.margins: 4
                                clip: true
                                model: backend.extKeystoreAccounts
                                delegate: TextInput {
                                    text: modelData
                                    color: _d.textColor
                                    font.pixelSize: _d.textSize
                                    width: parent ? parent.width : 0
                                    padding: 2
                                    readOnly: true
                                    selectByMouse: true
                                    selectedTextColor: _d.bgColor
                                    selectionColor: _d.accentColor
                                }
                            }
                        }
                    }

                    Separator {}

                    // New Account
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: eksNewPassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledButton {
                            text: "New Account"
                            onClicked: {
                                var result = backend.extKeystoreNewAccount(eksNewPassphraseInput.text)
                                if (result) eksNewPassphraseInput.text = ""
                            }
                        }
                    }

                    Separator {}

                    // Import Account
                    ResultLabel { text: "Import Account:" }
                    StyledTextArea {
                        id: eksImportKeyJSONInput
                        Layout.fillWidth: true
                        placeholderText: "Key JSON..."
                    }
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: eksImportPassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "Current Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledInput {
                            id: eksImportNewPassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "New Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledButton {
                            text: "Import"
                            onClicked: {
                                var result = backend.extKeystoreImport(eksImportKeyJSONInput.text, eksImportPassphraseInput.text, eksImportNewPassphraseInput.text)
                                if (result) { eksImportKeyJSONInput.text = ""; eksImportPassphraseInput.text = ""; eksImportNewPassphraseInput.text = "" }
                            }
                        }
                    }

                    Separator {}

                    // Export Extended Key
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: eksExportExtAddressInput
                            Layout.fillWidth: true
                            placeholderText: "Address..."
                        }
                        StyledInput {
                            id: eksExportExtPassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledInput {
                            id: eksExportExtNewPassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "New Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledButton {
                            text: "Export"
                            onClicked: {
                                var result = backend.extKeystoreExportExt(eksExportExtAddressInput.text, eksExportExtPassphraseInput.text, eksExportExtNewPassphraseInput.text)
                                eksExportExtResult.text = result || "Export failed"
                            }
                        }
                    }
                    ResultLabel { text: "Export Result:" }
                    ResultArea { id: eksExportExtResult }

                    Separator {}

                    // Export Private Key
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: eksExportPrivAddressInput
                            Layout.fillWidth: true
                            placeholderText: "Address..."
                        }
                        StyledInput {
                            id: eksExportPrivPassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledInput {
                            id: eksExportPrivNewPassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "New Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledButton {
                            text: "Export Private Key"
                            onClicked: {
                                var result = backend.extKeystoreExportPriv(eksExportPrivAddressInput.text, eksExportPrivPassphraseInput.text, eksExportPrivNewPassphraseInput.text)
                                eksExportPrivResult.text = result || "Export failed"
                            }
                        }
                    }
                    ResultLabel { text: "Export Result:" }
                    ResultArea { id: eksExportPrivResult }

                    Separator {}

                    // Delete Account
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: eksDeleteAddressInput
                            Layout.fillWidth: true
                            placeholderText: "Address..."
                        }
                        StyledInput {
                            id: eksDeletePassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledButton {
                            text: "Delete Account"
                            onClicked: backend.extKeystoreDelete(eksDeleteAddressInput.text, eksDeletePassphraseInput.text)
                        }
                    }

                    Separator {}

                    // Has Address
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: eksHasAddressInput
                            Layout.fillWidth: true
                            placeholderText: "Address..."
                        }
                        StyledButton {
                            text: "Has Address"
                            onClicked: {
                                var result = backend.extKeystoreHasAddress(eksHasAddressInput.text)
                                eksHasAddressResult.text = result ? "true" : "false"
                            }
                        }
                    }
                    ResultLabel { text: "Has Address Result:" }
                    ResultField { id: eksHasAddressResult }

                    Separator {}

                    // Unlock / Lock
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: eksUnlockAddressInput
                            Layout.fillWidth: true
                            placeholderText: "Address..."
                        }
                        StyledInput {
                            id: eksUnlockPassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledButton {
                            text: "Unlock"
                            onClicked: backend.extKeystoreUnlock(eksUnlockAddressInput.text, eksUnlockPassphraseInput.text)
                        }
                    }
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: eksLockAddressInput
                            Layout.fillWidth: true
                            placeholderText: "Address..."
                        }
                        StyledButton {
                            text: "Lock"
                            onClicked: backend.extKeystoreLock(eksLockAddressInput.text)
                        }
                    }

                    Separator {}

                    // Timed Unlock
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: eksTimedUnlockAddressInput
                            Layout.fillWidth: true
                            placeholderText: "Address..."
                        }
                        StyledInput {
                            id: eksTimedUnlockPassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledInput {
                            id: eksTimedUnlockTimeoutInput
                            Layout.preferredWidth: 120
                            placeholderText: "Timeout (s)..."
                        }
                        StyledButton {
                            text: "Timed Unlock"
                            onClicked: backend.extKeystoreTimedUnlock(eksTimedUnlockAddressInput.text, eksTimedUnlockPassphraseInput.text, parseInt(eksTimedUnlockTimeoutInput.text))
                        }
                    }

                    Separator {}

                    // Update Passphrase
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: eksUpdateAddressInput
                            Layout.fillWidth: true
                            placeholderText: "Address..."
                        }
                        StyledInput {
                            id: eksUpdatePassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "Current Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledInput {
                            id: eksUpdateNewPassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "New Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledButton {
                            text: "Update Passphrase"
                            onClicked: backend.extKeystoreUpdate(eksUpdateAddressInput.text, eksUpdatePassphraseInput.text, eksUpdateNewPassphraseInput.text)
                        }
                    }

                    Separator {}

                    // Sign Hash
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: eksSignHashAddressInput
                            Layout.fillWidth: true
                            placeholderText: "Address..."
                        }
                        StyledInput {
                            id: eksSignHashHashInput
                            Layout.fillWidth: true
                            placeholderText: "Hash (hex)..."
                        }
                        StyledButton {
                            text: "Sign Hash"
                            onClicked: {
                                var result = backend.extKeystoreSignHash(eksSignHashAddressInput.text, eksSignHashHashInput.text)
                                eksSignHashResult.text = result || "Signing failed"
                            }
                        }
                    }
                    ResultLabel { text: "Signature Result:" }
                    ResultField { id: eksSignHashResult }

                    Separator {}

                    // Sign Hash With Passphrase
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: eksSignHashWPAddressInput
                            Layout.fillWidth: true
                            placeholderText: "Address..."
                        }
                        StyledInput {
                            id: eksSignHashWPPassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledInput {
                            id: eksSignHashWPHashInput
                            Layout.fillWidth: true
                            placeholderText: "Hash (hex)..."
                        }
                        StyledButton {
                            text: "Sign Hash With Passphrase"
                            onClicked: {
                                var result = backend.extKeystoreSignHashWithPassphrase(eksSignHashWPAddressInput.text, eksSignHashWPPassphraseInput.text, eksSignHashWPHashInput.text)
                                eksSignHashWPResult.text = result || "Signing failed"
                            }
                        }
                    }
                    ResultLabel { text: "Signature Result:" }
                    ResultField { id: eksSignHashWPResult }

                    Separator {}

                    // Import Extended Key
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: eksImportExtKeyInput
                            Layout.fillWidth: true
                            placeholderText: "Extended Key..."
                        }
                        StyledInput {
                            id: eksImportExtKeyPassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledButton {
                            text: "Import Extended Key"
                            onClicked: {
                                var result = backend.extKeystoreImportExtendedKey(eksImportExtKeyInput.text, eksImportExtKeyPassphraseInput.text)
                                eksImportExtKeyResult.text = result || "Import failed"
                            }
                        }
                    }
                    ResultLabel { text: "Import Result:" }
                    ResultField { id: eksImportExtKeyResult }

                    Separator {}

                    // Sign Transaction
                    ResultLabel { text: "Sign Transaction:" }
                    StyledInput {
                        id: eksSignTxAddressInput
                        Layout.fillWidth: true
                        placeholderText: "Address..."
                    }
                    StyledTextArea {
                        id: eksSignTxJSONInput
                        Layout.fillWidth: true
                        placeholderText: "Transaction JSON..."
                    }
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: eksSignTxChainIDInput
                            Layout.fillWidth: true
                            placeholderText: "Chain ID (hex)..."
                        }
                        StyledButton {
                            text: "Sign Transaction"
                            onClicked: {
                                var result = backend.extKeystoreSignTx(eksSignTxAddressInput.text, eksSignTxJSONInput.text, eksSignTxChainIDInput.text)
                                eksSignTxResult.text = result || "Signing failed"
                            }
                        }
                    }
                    ResultLabel { text: "Signed Transaction Result:" }
                    ResultArea { id: eksSignTxResult }

                    Separator {}

                    // Sign Transaction With Passphrase
                    ResultLabel { text: "Sign Transaction With Passphrase:" }
                    StyledInput {
                        id: eksSignTxWPAddressInput
                        Layout.fillWidth: true
                        placeholderText: "Address..."
                    }
                    StyledInput {
                        id: eksSignTxWPPassphraseInput
                        Layout.fillWidth: true
                        placeholderText: "Passphrase..."
                        echoMode: TextInput.Password
                    }
                    StyledTextArea {
                        id: eksSignTxWPJSONInput
                        Layout.fillWidth: true
                        placeholderText: "Transaction JSON..."
                    }
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: eksSignTxWPChainIDInput
                            Layout.fillWidth: true
                            placeholderText: "Chain ID (hex)..."
                        }
                        StyledButton {
                            text: "Sign Transaction With Passphrase"
                            onClicked: {
                                var result = backend.extKeystoreSignTxWithPassphrase(eksSignTxWPAddressInput.text, eksSignTxWPPassphraseInput.text, eksSignTxWPJSONInput.text, eksSignTxWPChainIDInput.text)
                                eksSignTxWPResult.text = result || "Signing failed"
                            }
                        }
                    }
                    ResultLabel { text: "Signed Transaction Result:" }
                    ResultArea { id: eksSignTxWPResult }

                    Separator {}

                    // Derive
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: eksDeriveAddressInput
                            Layout.fillWidth: true
                            placeholderText: "Address..."
                        }
                        StyledInput {
                            id: eksDerivePathInput
                            Layout.fillWidth: true
                            placeholderText: "Derivation Path (e.g., m/44'/60'/0'/0/0)..."
                        }
                        StyledInput {
                            id: eksDerivePinInput
                            Layout.preferredWidth: 80
                            placeholderText: "PIN..."
                        }
                        StyledButton {
                            text: "Derive"
                            onClicked: {
                                var result = backend.extKeystoreDerive(eksDeriveAddressInput.text, eksDerivePathInput.text, parseInt(eksDerivePinInput.text))
                                eksDeriveResult.text = result || "Derivation failed"
                            }
                        }
                    }
                    ResultLabel { text: "Derived Address Result:" }
                    ResultField { id: eksDeriveResult }

                    Separator {}

                    // Derive With Passphrase
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: eksDeriveWPAddressInput
                            Layout.fillWidth: true
                            placeholderText: "Address..."
                        }
                        StyledInput {
                            id: eksDeriveWPPathInput
                            Layout.fillWidth: true
                            placeholderText: "Derivation Path..."
                        }
                        StyledInput {
                            id: eksDeriveWPPinInput
                            Layout.preferredWidth: 80
                            placeholderText: "PIN..."
                        }
                    }
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: eksDeriveWPPassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledInput {
                            id: eksDeriveWPNewPassphraseInput
                            Layout.fillWidth: true
                            placeholderText: "New Passphrase..."
                            echoMode: TextInput.Password
                        }
                        StyledButton {
                            text: "Derive With Passphrase"
                            onClicked: {
                                var result = backend.extKeystoreDeriveWithPassphrase(eksDeriveWPAddressInput.text, eksDeriveWPPathInput.text, parseInt(eksDeriveWPPinInput.text), eksDeriveWPPassphraseInput.text, eksDeriveWPNewPassphraseInput.text)
                                eksDeriveWPResult.text = result || "Derivation failed"
                            }
                        }
                    }
                    ResultLabel { text: "Derived Address Result:" }
                    ResultField { id: eksDeriveWPResult }

                    Separator {}

                    // Find
                    RowLayout {
                        spacing: 8
                        StyledInput {
                            id: eksFindAddressInput
                            Layout.fillWidth: true
                            placeholderText: "Address..."
                        }
                        StyledInput {
                            id: eksFindUrlInput
                            Layout.fillWidth: true
                            placeholderText: "URL..."
                        }
                        StyledButton {
                            text: "Find"
                            onClicked: {
                                var result = backend.extKeystoreFind(eksFindAddressInput.text, eksFindUrlInput.text)
                                eksFindResult.text = result || "Not found"
                            }
                        }
                    }
                    ResultLabel { text: "Find Result:" }
                    ResultField { id: eksFindResult }

                    Item { Layout.fillHeight: true }
                }
            }
        }
    }

    // ===== Reusable inline components =====

    component SectionHeader: Text {
        Layout.fillWidth: true
        color: _d.accentColor
        font.pixelSize: _d.sectionSize
        font.bold: true
        topPadding: 4
        bottomPadding: 4
    }

    component ResultLabel: Text {
        Layout.fillWidth: true
        color: _d.labelColor
        font.pixelSize: _d.textSize
    }

    component StyledInput: TextField {
        Layout.fillWidth: true
        color: _d.textColor
        font.pixelSize: _d.textSize
        leftPadding: 8
        rightPadding: 8
        topPadding: 6
        bottomPadding: 6
        background: Rectangle {
            color: _d.inputColor
            radius: 4
            border.color: parent.activeFocus ? _d.accentColor : _d.borderColor
            border.width: 1
        }
        placeholderTextColor: "#888888"
    }

    component StyledTextArea: ScrollView {
        property alias text: textAreaInner.text
        property alias placeholderText: textAreaInner.placeholderText
        Layout.fillWidth: true
        Layout.preferredHeight: 60
        background: Rectangle {
            color: _d.inputColor
            radius: 4
            border.color: _d.borderColor
            border.width: 1
        }
        TextArea {
            id: textAreaInner
            color: _d.textColor
            font.pixelSize: _d.textSize
            placeholderTextColor: "#888888"
            wrapMode: TextArea.Wrap
        }
    }

    component ResultField: TextField {
        Layout.fillWidth: true
        readOnly: true
        color: _d.textColor
        font.pixelSize: _d.textSize
        leftPadding: 8
        rightPadding: 8
        topPadding: 6
        bottomPadding: 6
        placeholderText: "Result will appear here..."
        placeholderTextColor: "#888888"
        background: Rectangle {
            color: _d.panelColor
            radius: 4
            border.color: _d.borderColor
            border.width: 1
        }
    }

    component ResultArea: ScrollView {
        property alias text: resultTextInner.text
        Layout.fillWidth: true
        Layout.preferredHeight: 60
        background: Rectangle {
            color: _d.panelColor
            radius: 4
            border.color: _d.borderColor
            border.width: 1
        }
        TextArea {
            id: resultTextInner
            readOnly: true
            color: _d.textColor
            font.pixelSize: _d.textSize
            placeholderText: "Result will appear here..."
            placeholderTextColor: "#888888"
            wrapMode: TextArea.Wrap
        }
    }

    component StyledButton: Button {
        contentItem: Text {
            text: parent.text
            color: _d.textColor
            font.pixelSize: _d.textSize
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        background: Rectangle {
            color: parent.hovered ? _d.buttonHoverColor : _d.buttonColor
            radius: 4
        }
        leftPadding: 16
        rightPadding: 16
        topPadding: 6
        bottomPadding: 6
    }

    component Separator: Rectangle {
        Layout.fillWidth: true
        height: 1
        color: _d.separatorColor
    }
}
