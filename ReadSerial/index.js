const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');
const axios = require('axios');

// Serial Port setup
const portName = "/dev/cu.usbserial-xxxxx"; // Add serial port number here
const apiKey = 'thingspeak-api-key-xxxxxxxxxx'; // Add Thingspeak api key here
const token = 'blynk-api=key-xxxxxx'; // Add blynk api key here

const port = new SerialPort({path: portName, baudRate: 9600 }); // Change baudRate if different
const parser = port.pipe(new ReadlineParser({ delimiter: '\n' }));

port.on("open", () => {
    console.log(`Serial port ${portName} opened`);
});

// Read the port data
parser.on('data', data => {
    setTimeout( () => {

        try {
            console.log(`Data received: ${data}`);
            var jsonObj = JSON.parse(data);

            //This is only for 3 data fields
            Object.keys(jsonObj).forEach(function(key) {
                var tsField = key === 'co2Value' ? 'field1' : key === 'temperature' ?  'field2' : 'field3';
                var blynkField = key === 'co2Value' ? 'V0' : key === 'temperature' ?  'V1' : 'V2';

                var tsUrl = `https://api.thingspeak.com/update?api_key=${apiKey}&${tsField}=${jsonObj[key]}`;
                var blynkUrl = `https://blynk.cloud/external/api/update?token=${token}&${blynkField}=${jsonObj[key]}`;

                console.log(tsUrl);
                console.log(blynkUrl);

                sendData(tsUrl, jsonObj[key]);
                sendData(blynkUrl, jsonObj[key]);
            });
        } catch (error) {
            console.log(error);
        }
    }, 5000);
});

function sendData(url, data) {
    axios.get(url)
    .then(function (response) {
        console.log(`Successfully sent data: ${data}`);
    })
    .catch(error => {
        console.log(error);
    });
}