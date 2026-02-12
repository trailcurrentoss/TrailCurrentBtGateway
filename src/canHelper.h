#pragma once
#include "globals.h"
#include "driver/twai.h"
#define CAN_RX 13
#define CAN_TX 15
#define POLLING_RATE_MS 33
#define CAN_SEND_MESSAGE_IDENTIFIER 0x18;
static bool driver_installed = false;

namespace canHelper
{
    void canSetup()
    {
        twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)CAN_TX, (gpio_num_t)CAN_RX, TWAI_MODE_NO_ACK);
        twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS(); // Look in the api-reference for other speed sets.
        // Accept all CAN messages in order to send them as status updates via bluetooth.
        twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
        // Install TWAI driver
        if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK)
        {
            debugln("Driver installed");
        }
        else
        {
            debugln("Failed to install driver");
            return;
        }

        // Start TWAI driver
        if (twai_start() == ESP_OK)
        {
            debugln("Driver started");
        }
        else
        {
            debugln("Failed to start driver");
            return;
        }

        // Reconfigure alerts to detect frame receive, Bus-Off error and RX queue full states
        uint32_t alerts_to_enable = TWAI_ALERT_RX_DATA | TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_ERROR | TWAI_ALERT_RX_QUEUE_FULL;
        if (twai_reconfigure_alerts(alerts_to_enable, NULL) == ESP_OK)
        {
            debugln("CAN Alerts reconfigured");
        }
        else
        {
            debugln("Failed to reconfigure alerts");
            return;
        }

        // TWAI driver is now successfully installed and started
        driver_installed = true;
    }
    // Sends the current state as mosfets are turned on and off, or power output levels changes.
    void sendDevicesState()
    {
        uint8_t val[8];
        pCharacteristic->setValue(devicesState, 8); // Set the characteristic value
        pCharacteristic->notify();                  // Notify connected device of the new value
    }
    // Sends the solar mppt controller data over bluetooth.
    void sendMpptData()
    {
        debugln("Sending solar data");
        mpptCharacteristic->setValue(mpptData01, 7);
        mpptCharacteristic->notify();
    }

    void sendMpptData02()
    {
        debugln("Sending solar data 02");
        mppt2Characteristic->setValue(mpptData02, 3);
        mppt2Characteristic->notify();
    }

    void sendTempData()
    {
        debugln("Sending temp data");
        tempCharacteristic->setValue(tempValues, 4);
        tempCharacteristic->notify();
    }

    void sendShuntData()
    {
        debugln("Sending shunt data");
        shuntCharacteristic->setValue(shuntData01, 7);
        shuntCharacteristic->notify();
    }

    void sendShuntData02() {
        debugln("Sending shunt data 02");
        shuntCharacteristic2->setValue(shuntData02,3);
        shuntCharacteristic2->notify();
    }

    void action_turn_all_devices_off()
    {
        // Configure message to transmit
        twai_message_t message;
        message.identifier = CAN_SEND_MESSAGE_IDENTIFIER;
        message.extd = false; // Using CAN 2.0 extended id allowing up to 536870911 identifiers
        message.rtr = false;
        message.data_length_code = 2;
        message.data[0] = 8;
        message.data[1] = 0;
        // Queue message for transmission
        twai_transmit(&message, pdMS_TO_TICKS(10));
    }

    void action_turn_all_devices_on()
    {
        // Configure message to transmit
        twai_message_t message;
        message.identifier = CAN_SEND_MESSAGE_IDENTIFIER;
        message.extd = false; // Using CAN 2.0 extended id allowing up to 536870911 identifiers
        message.rtr = false;
        message.data_length_code = 2;
        message.data[0] = 9;
        message.data[1] = 1;
        // Queue message for transmission
        twai_transmit(&message, pdMS_TO_TICKS(10));
    }

    static void send_toggle_message(int btn, int desiredValue)
    {
        // Configure message to transmit
        twai_message_t message;
        message.identifier = 0x18;
        message.extd = false; // Using CAN 2.0 extended id allowing up to 536870911 identifiers
        message.rtr = false;
        message.data_length_code = 2;
        message.data[0] = btn;
        message.data[1] = desiredValue;
        Serial.print("Value Sent: ");
        Serial.print(btn);
        Serial.println(" ");
        Serial.println(desiredValue);
        // Queue message for transmission
        if (twai_transmit(&message, pdMS_TO_TICKS(10)) == ESP_OK)
        {
            // Serial.println("Message queued for transmission");
        }
        else
        {
            // Serial.println("Failed to queue message for transmission");
        }
    }

    static void handle_rx_message(twai_message_t &message)
    {
        // Process received message
        if (message.extd)
        {
            debugln("Message is in Extended Format");
        }
        else
        {
            debugln("Message is in Standard Format");
        }
        debugf("ID: %lx\nByte:", message.identifier);
        if (!(message.rtr))
        {
            switch (message.identifier)
            {
            case 27: // All the Light status messages
                devicesState[0] = message.data[0];
                devicesState[1] = message.data[1];
                devicesState[2] = message.data[2];
                devicesState[3] = message.data[3];
                devicesState[4] = message.data[4];
                devicesState[5] = message.data[5];
                devicesState[6] = message.data[6];
                devicesState[7] = message.data[7];
                sendDevicesState();
                break;
            case 31: // Temperature and humidty
            {
                tempValues[0] = message.data[0]; // Degrees C
                tempValues[1] = message.data[1]; // Degrees F
                tempValues[2] = message.data[2]; // Humidity whole number
                tempValues[3] = message.data[3]; // Humidity decimal
                sendTempData();
            }
            break;
            case 35: // Data from shunt
            {
                shuntData01[0] = message.data[0]; // Battery voltage whole number
                shuntData01[1] = message.data[1]; // Battery voltage decimal number
                shuntData01[2] = message.data[2]; // Is current negative 0 = no, 1 = yes
                shuntData01[3] = message.data[3]; // Current whole number
                shuntData01[4] = message.data[4]; // Current decimal number
                shuntData01[5] = message.data[5]; // Battery state of charge percentage whole number
                shuntData01[6] = message.data[6]; // Battery state of charge decimal number
                sendShuntData();
            }
            break;
            case 36: // Second data from shunt
            {
                shuntData02[0] = message.data[0]; // Shunt wattage is negative 0 = no, 1 = yes
                shuntData02[1] = message.data[1]; // Shunt wattage MSB
                shuntData02[2] = message.data[2]; // Shunt wattage LSB
                sendShuntData02();
            }
            break;
            case 44:
            {
                mpptData01[0] = message.data[0]; // Panel voltage whole number
                mpptData01[1] = message.data[1]; // Panel voltage decimal number
                mpptData01[2] = message.data[2]; // Solar watts MSB
                mpptData01[3] = message.data[3]; // Solar watts LSB
                mpptData01[4] = message.data[4]; // Battery voltage whole number
                mpptData01[5] = message.data[5]; // Battery voltage decimal 
                mpptData01[6] = message.data[6]; // Solar status value
                sendMpptData();
            }
            break;
            case 45:
            {
                mpptData02[0] = message.data[0]; // Is panel current negative 0 = no, 1 = yes
                mpptData02[1] = message.data[1]; // Panel current whole number
                mpptData02[2] = message.data[2]; // Panel current decimal value
                sendMpptData02();
            }
            break;
            default:
                break;
            }
            for (int i = 0; i < message.data_length_code; i++)
            {
                debugg(" %d = %02x,", i, message.data[i]);
            }
            debugln("");
        }
    }

    void canLoop()
    {
        if (!driver_installed)
        {
            // Driver not installed
            delay(1000);
            return;
        }
        // Check if alert happened
        uint32_t alerts_triggered;
        twai_read_alerts(&alerts_triggered, pdMS_TO_TICKS(POLLING_RATE_MS));
        twai_status_info_t twaistatus;
        twai_get_status_info(&twaistatus);

        // Handle alerts
        if (alerts_triggered & TWAI_ALERT_ERR_PASS)
        {
            debugln("Alert: TWAI controller has become error passive.");
        }
        if (alerts_triggered & TWAI_ALERT_BUS_ERROR)
        {
            debugln("Alert: A (Bit, Stuff, CRC, Form, ACK) error has occurred on the bus.");
            debugf("Bus error count: %lu\n", twaistatus.bus_error_count);
        }
        if (alerts_triggered & TWAI_ALERT_RX_QUEUE_FULL)
        {
            debugln("Alert: The RX queue is full causing a received frame to be lost.");
            debugf("RX buffered: %lu\t", twaistatus.msgs_to_rx);
            debugf("RX missed: %lu\t", twaistatus.rx_missed_count);
            debugf("RX overrun %lu\n", twaistatus.rx_overrun_count);
        }

        // Check if message is received
        if (alerts_triggered & TWAI_ALERT_RX_DATA)
        {
            // One or more messages received. Handle all.
            twai_message_t message;
            while (twai_receive(&message, 0) == ESP_OK)
            {
                handle_rx_message(message);
            }
        }
    }
}