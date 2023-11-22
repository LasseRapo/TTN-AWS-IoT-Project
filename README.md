This project uses the FIT IoT lab, The Things Network and AWS. The board in the IoT lab testbed reads sensor data, sends it to the TTN by LoRaWAN, and then TTN forwards the data to the AWS DynamoDB by MQTT.

Here is a quick video demonstration of the project: https://youtu.be/l7ZJS32fN-E

How to use the project:

1 - Setup your TTN application. Add an end device with the following parameters:

Frequency plan: Europe 863-870 MHz (SF9 for RX2 - recommended)

LoRaWAN version: 1.0.3

AppEUI: 0000000000000000

DevEUI: Generate (add this to the main.c file)

AppKey: Generate (add this to the main.c file)

2 - In TTN end device "Payload formatters" -> "Uplink" use "Custom Javascript formatter":

	function decodeUplink(input) {
		var jsonString = "";

		for (var i = 0; i < input.bytes.length; i++) {
			jsonString += String.fromCharCode(input.bytes[i]);
		}

		var decodedData;
		try {
			  decodedData = JSON.parse(jsonString);
		} catch (error) {
			return {
				data: {},
				warnings: [],
				errors: ["Failed to parse JSON"],
			};
		}
		
		return {
			data: decodedData,
			warnings: [],
			errors: [],
	};
	}


2 - Setup AWS IoT from the application you created in TTN and follow these instructions "https://www.thethingsindustries.com/docs/integrations/cloud-integrations/aws-iot/deployment-guide/"

3 - Setup DynamoDB following these instructions "https://docs.aws.amazon.com/iot/latest/developerguide/iot-ddrule.html"

For "SQL statement" to get our project work use this 

	"SELECT get((SELECT decoded_payload.Humidity FROM uplink_message), 0).Humidity as Humidity, get((SELECT decoded_payload.Temperature FROM uplink_message), 0).Temperature as Temperature, get((SELECT decoded_payload.Pressure FROM uplink_message), 0).Pressure as Pressure, get((SELECT decoded_payload.Air_quality FROM uplink_message), 0).Air_quality as Air_quality, received_at FROM 'lorawan/+/uplink'"

4 - Login to the FIT IoT-Lab and go to the folder that has the main.c and Makefile

5 - run "make all"

6 - Download the file "lorawan_sensors.elf". It should be under this path -> "/bin/b-l072z-	lrwan1/lorawan_sensors.elf". This is the firmware file.

7 - Go to FIT IoT-Lab and create a "new experiment" with one node (st-lrwan1 (Sx1276) Saclay)

8 - Then "flash firmware" on the node. Use the .elf file downloaded in step 6. After this it should send the message to TTN and TTN should send it to AWS, and then store it to DynamoDB. You can monitor the terminal to see if the messages are sent. If the connection to TTN fails, try resetting the board.
