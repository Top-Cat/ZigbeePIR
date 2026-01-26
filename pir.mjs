import {Zcl} from "zigbee-herdsman";
import * as m from 'zigbee-herdsman-converters/lib/modernExtend';

export default {
    zigbeeModel: ['Kitchen PIR Sensor'],
    model: 'Kitchen PIR Sensor',
    vendor: 'TC',
    icon: 'device_icons/8783691.png',
    description: 'Custom PIR sensor light',
    extend: [
        m.deviceAddCustomCluster("tcSpecificLed", {
            manufacturerCode: 0x1234,
            ID: 0xFC10,
            attributes: {
                'amber': { ID: 0x0001, type: Zcl.DataType.UINT8, write: true, max: 0xff },
                'warm_white': { ID: 0x0002, type: Zcl.DataType.UINT8, write: true, max: 0xff },
                'cool_white': { ID: 0x0003, type: Zcl.DataType.UINT8, write: true, max: 0xff },
                'count': { ID: 0x0010, type: Zcl.DataType.UINT16, write: true, max: 0xffff },
                'animation': { ID: 0x0011, type: Zcl.DataType.ENUM8, write: true, max: 0xff },
                'speed': { ID: 0x0012, type: Zcl.DataType.UINT8, write: true, max: 0xff },
            },
            commands: {},
            commandsResponse: {},
        }),
        m.deviceAddCustomCluster("tcSpecificLux", {
            manufacturerCode: 0x1234,
            ID: 0xFC11,
            attributes: {
                'inhibit_threshold': { ID: 0x0001, type: Zcl.DataType.UINT16, write: true, max: 0xffff }
            },
            commands: {},
            commandsResponse: {},
        }),

        m.occupancy({
            pirConfig: ["otu_delay"]
        }),
        m.numeric({
            name: "pir_uto_delay",
            label: "Manual timeout",
            description: "Time in seconds before automatic mode resumes after manual changes.",
            cluster: "msOccupancySensing",
            attribute: "pirUToODelay",
            valueMin: 0,
            valueMax: 65534,
            unit: "s"
        }),
        m.onOff({
            powerOnBehavior: false
        }),
        m.numeric({
            name: 'amber',
            label: 'Amber Brightness',
            cluster: 'tcSpecificLed',
            attribute: 'amber',
            valueMin: 0,
            valueMax: 255,
        }),
        m.numeric({
            name: 'warm_white',
            label: 'Warm White Brightness',
            cluster: 'tcSpecificLed',
            attribute: 'warm_white',
            valueMin: 0,
            valueMax: 255,
        }),
        m.numeric({
            name: 'cool_white',
            label: 'Cold White Brightness',
            cluster: 'tcSpecificLed',
            attribute: 'cool_white',
            valueMin: 0,
            valueMax: 255,
        }),
        m.numeric({
            name: 'count',
            label: 'Led Count',
            cluster: 'tcSpecificLed',
            attribute: 'count',
            valueMin: 0,
            valueMax: 500,
        }),
        m.enumLookup({
            name: 'effect',
            label: 'Light Animation',
            cluster: 'tcSpecificLed',
            attribute: 'animation',
            lookup: {
                'basic': 0x00,
                'rows': 0x01,
                'from_ends': 0x02,
                'from_center': 0x03,
                'sparkle': 0x04,
                'from_left': 0x05,
                'from_right': 0x06
            }
        }),
        m.numeric({
            name: 'speed',
            label: 'Animation Speed',
            cluster: 'tcSpecificLed',
            attribute: 'speed',
            valueMin: 0,
            valueMax: 255
        }),
        m.numeric({
            name: 'inhibit_threshold',
            label: 'Inhibit Threshold',
            cluster: 'tcSpecificLux',
            attribute: 'inhibit_threshold',
            valueMin: 0,
            valueMax: 88000,
            unit: 'lx',
            scale: (value, type) => {
                if (type === "from") {
                    return Math.round(10 ** ((value - 1) / 10000));
                } else {
                    return 10000 * Math.log10(value) + 1;
                }
            }
        }),
        m.illuminance(),
        m.temperature()
    ],
    ota: true
};
