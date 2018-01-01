import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtPositioning 5.6
import QtLocation 5.6
import example.com.networklocation 1.0;

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("GSM Locator")

    ScrollView {
        anchors.fill: parent
        anchors.margins: 10

        ColumnLayout {
            id: mainColLayout
            width: parent.width
            spacing: 15

            GridLayout {
                rows: 5
                columns: 2

                Text {
                    text: "Location API"
                    font.bold: true
                    font.pointSize: 20
                    Layout.minimumWidth: 150
                    color: "#FFFFFF"
                }

                Text { text: "" }

                Text {
                    text: "Longitude:"
                    font.bold: true
                    font.pointSize: 15
                    color: "#FFFFFF"
                }

                Text {
                    id: locApiLongitude
                    text: "N/A"
                    color: "#FFFFFF"
                }

                Text {
                    text: "Latitude:"
                    font.bold: true
                    font.pointSize: 15
                    color: "#FFFFFF"
                }

                Text {
                    id: locApiLatitude
                    text: "N/A"
                    color: "#FFFFFF"
                }

                Text {
                    text: "Source:"
                    font.bold: true
                    font.pointSize: 15
                    color: "#FFFFFF"
                }

                Text {
                    id: locApiSource
                    text: "N/A"
                    color: "#FFFFFF"
                }
            }

            GridLayout {
                rows: 5
                columns: 2

                Text {
                    text: "GSM network"
                    font.bold: true
                    font.pointSize: 20
                    Layout.minimumWidth: 150
                    color: "#FFFFFF"
                }

                Text { text: "" }

                Text {
                    text: "Longitude:"
                    font.bold: true
                    font.pointSize: 15
                    color: "#FFFFFF"
                }

                Text {
                    id: networkApiLongitude
                    text: netlocation.longitude.toFixed(5)
                    color: "#FFFFFF"
                }

                Text {
                    text: "Latitude:"
                    font.bold: true
                    font.pointSize: 15
                    color: "#FFFFFF"
                }

                Text {
                    id: networkApiLatitude
                    text: netlocation.latitude.toFixed(5)
                    color: "#FFFFFF"
                }

                Text {
                    text: "GSM cells #: "
                    font.bold: true
                    font.pointSize: 15
                    color: "#FFFFFF"
                }

                Text {
                    id: networkApiCellCount
                    text: netlocation.cellcount
                    color: "#FFFFFF"
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                GridLayout {
                    Layout.fillWidth: true

                    Text {
                        Layout.minimumWidth: 30;
                        color: "#FFFFFF"
                        text: "MCC"
                        font.bold: true
                    }

                    Text {
                        Layout.minimumWidth: 30;
                        color: "#FFFFFF"
                        text: "MNC"
                        font.bold: true
                    }

                    Text {
                        Layout.minimumWidth: 45;
                        color: "#FFFFFF"
                        text: "LAC"
                        font.bold: true
                    }

                    Text {
                        Layout.minimumWidth: 45;
                        color: "#FFFFFF"
                        text: "CID"
                        font.bold: true
                    }

                    Text {
                        Layout.minimumWidth: 55;
                        color: "#FFFFFF"
                        text: "LON"
                        font.bold: true
                    }

                    Text {
                        Layout.minimumWidth: 55;
                        color: "#FFFFFF"
                        text: "LAT"
                        font.bold: true
                    }

                    Text {
                        color: "#FFFFFF"
                        text: "SIG"
                        font.bold: true
                    }
                }

                Repeater {
                    model: netlocation.cells
                    GridLayout {
                        Layout.fillWidth: true

                        Text {
                            Layout.minimumWidth: 30;
                            color: "#FFFFFF"
                            text: modelData.mcc
                        }

                        Text {
                            Layout.minimumWidth: 30;
                            color: "#FFFFFF"
                            text: modelData.mnc
                        }

                        Text {
                            Layout.minimumWidth: 45;
                            color: "#FFFFFF"
                            text: modelData.lac
                        }

                        Text {
                            Layout.minimumWidth: 45;
                            color: "#FFFFFF"
                            text: modelData.cid
                        }

                        Text {
                            Layout.minimumWidth: 55;
                            color: "#FFFFFF"
                            text: modelData.lon.toFixed(5)
                        }

                        Text {
                            Layout.minimumWidth: 55;
                            color: "#FFFFFF"
                            text: modelData.lat.toFixed(5)
                        }

                        Text {
                            color: "#FFFFFF"
                            text: modelData.ss;
                        }
                    }
                }
            }
        }
    }

    NetworkLocation {
        id: netlocation
    }

    Timer {
        interval: 5000
        running: true
        repeat: true

        onTriggered: {
            netlocation.updateLocation();
        }
    }

    PositionSource {
        id: gsmloc
        updateInterval: 100
        //preferredPositioningMethods: PositionSource.NonSatellitePositioningMethods
        active: true

        onPositionChanged: {
            var coords = gsmloc.position.coordinate;
            if(!gsmloc.valid || isNaN(coords.longitude) || isNaN(coords.latitude)) {
                console.log("Received invalid location data.");
                return;
            }

            locApiLongitude.text = coords.longitude.toFixed(5);
            locApiLatitude.text = coords.latitude.toFixed(5);
            switch(gsmloc.supportedPositioningMethods) {
            case PositionSource.NoPositioningMethods:
                locApiSource.text = "None";
                break;
            case PositionSource.SatellitePositioningMethods:
                locApiSource.text = "Satellite-based (GPS)";
                break;
            case PositionSource.NonSatellitePositioningMethods:
                locApiSource.text = "Non-satellite-based (GSM)";
                break;
            case PositionSource.AllPositioningMethods:
                locApiSource.text = "GPS/GSM";
                break;
            }

            console.log("Coords: ", coords.longitude, coords.latitude);
        }
    }
}
