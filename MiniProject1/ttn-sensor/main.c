#include <string.h>

#include "board.h"
#include "timex.h"
#include "ztimer.h"

#include "sx127x.h"
#include "sx127x_netdev.h"
#include "sx127x_params.h"

#include "net/loramac.h"     // Core loramac definitions
#include "semtech_loramac.h" // Package API
#include "hts221.h"
#include "hts221_params.h"

#include "lpsxxx.h"
#include "lpsxxx_params.h"

static sx127x_t sx127x;      // sx127x radio driver descriptor

static semtech_loramac_t loramac;  // loramac stack descriptor
static hts221_t hts221;

static lpsxxx_t lps22hb;

// TTN Device and application parameters required for OTAA activation
static const uint8_t appeui[LORAMAC_APPEUI_LEN] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const uint8_t deveui[LORAMAC_DEVEUI_LEN] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const uint8_t appkey[LORAMAC_APPKEY_LEN] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static void data_sending(void)
{
    while(1) {

        /* Measuring pressure, humidity and temperature */
        uint16_t humidity = 0;
        int16_t temperature = 0;
        uint16_t pressure = 0;
        int16_t air_quality_index = 0;
        char air_quality_status[31];

        /* Simulate air quality */
        air_quality_index = rand() % 201;
        if (air_quality_index <= 50){
            strcpy(air_quality_status, "Good");
        }
        else if (air_quality_index >= 51 && air_quality_index <= 100){
            strcpy(air_quality_status, "Moderate");
        }
        else if (air_quality_index >= 101 && air_quality_index <= 150){
            strcpy(air_quality_status, "Unhealty for Sensitive Groups");
        }
        else if (air_quality_index >= 151 && air_quality_index <= 200){
            strcpy(air_quality_status, "Unhealty");
        }

        if (hts221_read_humidity(&hts221, &humidity) != HTS221_OK) {
            puts("Cannot read humidity!");
        }
        if (hts221_read_temperature(&hts221, &temperature) != HTS221_OK) {
            puts("Cannot read temperature!");
        }

        lpsxxx_read_pres(&lps22hb, &pressure);

        /* Initializing the message */
        char message[64];
        sprintf(message, "{\"Humidity\": \"%d.%d %%\", \"Temperature\": \"%d.%d C\", \"Pressure\": \"%u hPa\", \"Air_quality\": \"%i %s\"}",
                (humidity / 10), (humidity % 10),
                (temperature / 10), (temperature % 10),
                pressure,
                air_quality_index, air_quality_status);
        printf("Sending message '%s'\n", message);

        /* Sending the message to TTN */
        if (semtech_loramac_send(&loramac,
                                 (uint8_t *)message, strlen(message)) != SEMTECH_LORAMAC_TX_DONE) {
            printf("Cannot send message '%s'\n", message);
        }
        else {
            printf("Message '%s' sent\n", message);
        }

        ztimer_sleep(ZTIMER_MSEC, 20 * MS_PER_SEC); // Wait 20 seconds between each message
    }

    return; // This should never be reached
}

int main(void)
{
    if (hts221_init(&hts221, &hts221_params[0]) != HTS221_OK) {
        puts("Sensor hts221 initialization failed");
        return 1;
    }

    if (hts221_power_on(&hts221) != HTS221_OK) {
        puts("Sensor hts221 initialization power on failed");
        return 1;
    }

    if (hts221_set_rate(&hts221, hts221.p.rate) != HTS221_OK) {
        puts("Sensor hts221 continuous mode setup failed");
        return 1;
    }

    lpsxxx_init(&lps22hb, &lpsxxx_params[0]);
   

    puts("Initializing the radio driver...");
    sx127x_setup(&sx127x, &sx127x_params[0], 0);
    loramac.netdev = &sx127x.netdev;
    loramac.netdev->driver = &sx127x_driver;

    puts("Initializing the loramac stack...");
    semtech_loramac_init(&loramac);

    puts("Setting the LoRaWAN keys...");
    semtech_loramac_set_appeui(&loramac, appeui);
    semtech_loramac_set_deveui(&loramac, deveui);
    semtech_loramac_set_appkey(&loramac, appkey);

    puts("Setting the datarate to DR5...");
    semtech_loramac_set_dr(&loramac, 5);

    puts("Starting the OTAA join procedure");
    if (semtech_loramac_join(&loramac, LORAMAC_JOIN_OTAA) != SEMTECH_LORAMAC_JOIN_SUCCEEDED) {
        puts("Join procedure failed");
        return 1;
    }
    puts("Join procedure succeeded");

    data_sending(); // Start the data sending

    return 0; // This should never be reached
}
