This project demonstrates an end-to-end IoT project, starting from reading sensor data to storing the data securely in the cloud. The project uses the FIT IoT-LAB, The Things Network and AWS. The board in the IoT lab testbed reads sensor data, sends it to the TTN by LoRaWAN, and then TTN forwards the data to the AWS DynamoDB by MQTT.

Here is a quick video demo of the project: https://youtu.be/l7ZJS32fN-E

Here is a quick guide on how to use this project. This guide assumes that you already have a working RIOT environment for building the firmware:

1 - Login to The Things Network, signing up is free: https://www.thethingsnetwork.org/

Once you have logged in, click on the top right on your username and select console. From here you will choose a network cluster, for example Europe 1.

From the console create a new application and give the application an ID. The name and description of the application are optional information.

Register an end device to your application manually with the following parameters:

Frequency plan: Europe 863-870 MHz (SF9 for RX2 - recommended)

LoRaWAN version: 1.0.3

JoinEUI (appeui): 0000000000000000 (this is already in the main.c file, line 29, in C byte array format)

DevEUI: Generate

AppKey: Generate

End device ID: automatically generated when you generate the DevEUI key.

Your main.c should have your DevEUI and AppKey added. You can copy them from the TTN end device information panel. Click on the <> button next to the keys to get the required C byte array format:

![kuva](https://github.com/LasseRapo/TTN-AWS-IoT-Project/assets/71126392/0a6f84dd-3d71-48da-9d49-f42e507ba4b1)

![kuva](https://github.com/LasseRapo/TTN-AWS-IoT-Project/assets/71126392/5f4a0bb2-fca9-4929-b6d4-df677dce048b)




2 - In the TTN end device select "Payload formatters" -> "Uplink" use "Custom Javascript formatter". Paste the following code to the Javascript formatter:

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


2 - In the TTN application select Integrations from the left hand side, select AWS IoT and follow the deployment guide provided by TTN: https://www.thethingsindustries.com/docs/integrations/cloud-integrations/aws-iot/deployment-guide/

Using the AWS CloudFormation Template is not well documented in the guide, so make sure you choose the correct AWS region and The Things Stack subscription you are using.

For example in my case my AWS region is Stockholm and I am using the community edition:

![kuva](https://github.com/LasseRapo/TTN-AWS-IoT-Project/assets/71126392/6c71b5e6-18bb-4573-b76d-8e772760bc18)

After that click Deploy for The Things Network, this will take you to AWS and then you can follow the rest of the deployment guide.

3 - Setup DynamoDB following these instructions "https://docs.aws.amazon.com/iot/latest/developerguide/iot-ddb-rule.html"

Here are some additional information on how I setup the database:

Step 1, creating the table:
In Partition key, choose Number.
In Sort key, use String instead of Number.

Step 2, creating AWS IoT rule:
For "SQL statement" use this: 

	"SELECT get((SELECT decoded_payload.Humidity FROM uplink_message), 0).Humidity as Humidity, get((SELECT decoded_payload.Temperature FROM uplink_message), 0).Temperature as Temperature, get((SELECT decoded_payload.Pressure FROM uplink_message), 0).Pressure as Pressure, get((SELECT decoded_payload.Air_quality FROM uplink_message), 0).Air_quality as Air_quality, received_at FROM 'lorawan/+/uplink'"

Here is the configuration I used for the rule:

![kuva](https://github.com/LasseRapo/TTN-AWS-IoT-Project/assets/71126392/8888526f-19cc-430c-b366-cc6cec81e57b)

(If you know how to flash firmware on the testbed you can skip steps 4-7)

4 - Drop the main.c and Makefile in a folder that is in the same directory as the RIOT code base. You can find the RIOT OS here: https://github.com/RIOT-OS/RIOT.  In my case the code files are inside a test_application folder. The test_application folder is in the same directory as the RIOT code base. The path of the RIOT folder is specified in the Makefile:

![kuva](https://github.com/LasseRapo/TTN-AWS-IoT-Project/assets/71126392/2bc1e2ba-b653-4cb8-a22a-764aef295ecb)

Modify the RIOTBASE parameter so that it refers to the path of your RIOT code base if it is somewhere else.

5 - In a terminal navigate to the folder that has the main.c and Makefile and run "make all".

If you get an error such as "USE_NEWLIB_NANO==1 but nano include folder not found!", run "source /opt/riot.source" and try "make all" again.

6 - Download the generated firmware file "lorawan_sensors.elf" generated. It should be under this path -> "/bin/b-l072z-lrwan1/lorawan_sensors.elf".

7 - Go to FIT IoT-LAB and create a "new experiment" with one node (st-lrwan1 (Sx1276))

8 - Then "flash firmware" on the node. Use the .elf file downloaded in step 6. After this it should send the message to TTN and TTN should send it to AWS, and then store it to DynamoDB. You can monitor the terminal to see if the messages are sent correctly. If the connection to TTN fails, try resetting the board.

<br/>

### Efficiency and Resilience Evaluation of the Project

1. Sensing Layer Efficiency:

Our project utilizes the HTS221 sensor for humidity and temperature data collecting, and the LPS22HB sensor for air pressure data collecting.
We also simulated air quality data collecting by using a random value generator. The readings are based on the air quality index.
The projects sensing layer appears to be efficient in terms of capturing environmental data using the different sensors, and 
the efficiency of this layer depends on the accuracy and reliability of those sensors and the data produced by them.
The efficiency can be affected by precision, calibration, and environmental conditions of the sensors.

2. Networking Layer Efficiency: 

Our networking layer uses the LoRaWAN protocol to transmit data to TTN (The Things Network) and then forwards it to AWS using MQTT.
Networking layer efficiency depends on the chosen data rate, which in our case is set to DR5. The data sending function is timed to 
sleep for 20 seconds between each message, which may impact real-time responsiveness but contributes to energy efficiency. The efficiency also depends on the quality of the LoRa signal and potential interferences in the wireless environment.

3. Resilience in Wireless Network:

The design includes handle sensor initialization, and error handling for failed sensor readings and unsuccessful message transmission errors.
However, it doesn't address data loss during packet collisions or other events in the wireless network. LoRaWAN itself provides some resilience features, but the impact of collisions or network congestion on data loss is not explicitly handled in the code.

4. Data Management Layer Efficiency:

The data management layer works by sending JSON-formatted data to TTN from the IoT-LAB, which is then forwarded to AWS and finally stored in DynamoDB.
The efficiency of the management layer depends on how often the data is transmitted, the size of the payloads, and the clouds processing capabilities. AWS services, including DynamoDB, are chosen for their high availability and durability. DynamoDB automatically replicates data across multiple servers and availability zones, enhancing resilience.

5. Data Reliability and Calibration:

The data reliability depends on many things, but most important is the accuracy and calibration of the sensors. The code doesn't include any 
calibration checks or correction mechanisms if the calibration is bad. However, the temperature, air pressure and humidity readings are within a reasonable range, suggesting that the sensors are providing consistent and plausible data.

   
In summary, our project demonstrates a functional end-to-end IoT system with different sensors and a robust database. One real-world application for this system could be a Smart HVAC system. Further improvement on handling issues on network, data loss and calibration
changes would improve the reliability of the project.
