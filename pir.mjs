import {Zcl} from "zigbee-herdsman";
import * as m from 'zigbee-herdsman-converters/lib/modernExtend';
import * as exposes from 'zigbee-herdsman-converters/lib/exposes';

export default {
    zigbeeModel: ['Kitchen PIR Sensor'],
    model: 'Kitchen PIR Sensor',
    vendor: 'TC',
    description: 'Automatically generated definition',
    extend: [
        m.occupancy({
            pirConfig: ["otu_delay"]
        }),
        m.onOff({
            powerOnBehavior: false
        }),
        m.temperature(),
        m.deviceAddCustomCluster("tcSpecificLed", {
            manufacturerCode: 0x1234,
            ID: 0xFC10,
            attributes: {
                'amber': { ID: 0x0001, type: Zcl.DataType.UINT8, write: true, max: 0xff },
                'warm_white': { ID: 0x0002, type: Zcl.DataType.UINT8, write: true, max: 0xff },
                'cool_white': { ID: 0x0003, type: Zcl.DataType.UINT8, write: true, max: 0xff },
                'count': { ID: 0x0010, type: Zcl.DataType.UINT16, write: true, max: 0xffff },
            },
            commands: {},
            commandsResponse: {},
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
        })
    ],
    ota: true
};
