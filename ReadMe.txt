Preliminary requirements:
	1. Setup TTN application and in TTN "Payload formatters" -> "Uplink" use "Custom 		Javascript formatter" 
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
		
	2. Setup AWS IoT from the application you created in TTN and follow these instructions 			"https://www.thethingsindustries.com/docs/integrations/cloud-integrations/aws-			iot/deployment-guide/"
	3. Setup DynamoDB following these instructions 							"https://docs.aws.amazon.com/iot/latest/developerguide/iot-ddrule.html"
		For "SQL statement" to get our project work use this "SELECT get((SELECT 			decoded_payload.Humidity FROM uplink_message), 0).Humidity as Humidity, 			get((SELECT decoded_payload.Temperature FROM uplink_message), 				0).Temperature as Temperature, get((SELECT decoded_payload.Pressure 			FROM uplink_message), 0).Pressure as Pressure, get((SELECT 				decoded_payload.Air_quality FROM uplink_message), 0).Air_quality as 			Air_quality, received_at FROM 'lorawan/+/uplink'"

1. Login to the iotlab grenoble server and go to folder that has main.c and Makefile
2. run "make all"
3. download the file "lorawan_sensors.elf" it should be under this bath -> "/bin/b-l072z-	lrwan1/lorawan_sensors.elf"
4. Then go to FIT IOT-LAB and create "new experiment", with one node (st-lrwan1 (Sx1276) Saclay)
5. Then "flash firmware" after this it should send the message to TTN and TTN should send it to 	AWS and then store it to DynamoDB