import QtQuick

Window {
    id: root
    width: 1280
    height: 720
    visible: true
    title: qsTr("Hello World")

    Image {
        id: image
        source: "/home/shade/Pictures/Wallpapers/wp15571478-3840x1600-4k-wallpapers.jpg"

        Text {
            text: "Hello World"
            font.pointSize: 40
            anchors.centerIn: image
            color: "#00414A"
        }
    }
}
