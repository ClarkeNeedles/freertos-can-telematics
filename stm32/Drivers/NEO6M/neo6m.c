/**
  ******************************************************************************
  * @file           : neo6m.c
  * @brief          : NEO-6M sensor driver
  * @author         : Clarke Needles
  ******************************************************************************
  */

#include <stdio.h>
#include <string.h>
#include <usart.h>
#include "neo6m.h"

/*******************************************************************************
 NEO6M_Init()
 ******************************************************************************/
void NEO6M_Init(NEO6M_Handle_t *dev)
{
	// Receive data over UART using interrupts
	HAL_UART_Receive_IT(dev->huart, dev->rx_data, 1);
} /* NEO6M_Init() */

/*******************************************************************************
 NEO6M_UART_CallBack()
 ******************************************************************************/
void NEO6M_UART_CallBack(NEO6M_Handle_t *dev)
{
	if ((dev->rx_data != '\n') && (dev->rx_index < GPSBUFSIZE))
	{
		dev->rx_buffer[dev->rx_index++] = dev->rx_data; // Move data into buffer
	}
	else
	{
		if(NEO6M_Validate((char*) dev->rx_buffer))
		{
			NEO6M_Parse((char*) dev->rx_buffer);
		}

		// Reset buffer
		dev->rx_index = 0;
		memset(dev->rx_buffer, 0, GPSBUFSIZE);
	}

	HAL_UART_Receive_IT(dev->huart, &dev->rx_data, 1); // Reset the interrupt
} /*  NEO6M_UART_CallBack() */


uint8_t NEO6M_Validate(char *rx_buffer)
{
    char expected_checksum[3];
    char received_checksum[3];
    int checksum = 0;

    // Ensure that string starts with '$'
    if (rx_buffer[0] != '$')
    {
    	return 0;
    }

    // Loop until the end of the NMEA string
    int i = 1;
    while((rx_buffer[i] != 0) && (rx_buffer[i] != '*') && (i < 75))
    {
        checksum ^= rx_buffer[i++]; // Calculate the checksum
    }

    // String was too long, return an error
    if(i >= 75)
    {
        return 0;
    }

    if (rx_buffer[i] == '*')
    {
    	// Put hex chars in check string
    	expected_checksum[0] = rx_buffer[i+1];
    	expected_checksum[1] = rx_buffer[i+2];
        expected_checksum[2] = 0;
    }
    else
    {
    	// No checksum separator found
    	return 0;
    }

    // Convert received checksum to hex for comparison
    sprintf(received_checksum,"%02X",checksum);

    return (received_checksum[0] == expected_checksum[0]) &&
    	   (received_checksum[1] == expected_checksum[1]);
}

void NEO6M_Parse(char *rx_buffer){
    if(!strncmp(rx_buffer, "$GPGGA", 6))
    {
    	if (sscanf(rx_buffer,
    			   "$GPGGA,%f,%f,%c,%f,%c,%d,%d,%f,%f,%c",
				   &GPS.utc_time, &GPS.nmea_latitude,
				   &GPS.ns, &GPS.nmea_longitude,
				   &GPS.ew,
				   &GPS.lock,
				   &GPS.satelites,
				   &GPS.hdop,
				   &GPS.msl_altitude,
				   &GPS.msl_units) >= 1)
    	{
    		GPS.dec_latitude = GPS_nmea_to_dec(GPS.nmea_latitude, GPS.ns);
    		GPS.dec_longitude = GPS_nmea_to_dec(GPS.nmea_longitude, GPS.ew);
    	}
    }
    else if (!strncmp(GPSstrParse, "$GPRMC", 6))
    {
    	if(sscanf(GPSstrParse, "$GPRMC,%f,%f,%c,%f,%c,%f,%f,%d", &GPS.utc_time, &GPS.nmea_latitude, &GPS.ns, &GPS.nmea_longitude, &GPS.ew, &GPS.speed_k, &GPS.course_d, &GPS.date) >= 1)
    		return;

    }
    else if (!strncmp(GPSstrParse, "$GPGLL", 6))
    {
        if(sscanf(GPSstrParse, "$GPGLL,%f,%c,%f,%c,%f,%c", &GPS.nmea_latitude, &GPS.ns, &GPS.nmea_longitude, &GPS.ew, &GPS.utc_time, &GPS.gll_status) >= 1)
            return;
    }
    else if (!strncmp(GPSstrParse, "$GPVTG", 6))
    {
        if(sscanf(GPSstrParse, "$GPVTG,%f,%c,%f,%c,%f,%c,%f,%c", &GPS.course_t, &GPS.course_t_unit, &GPS.course_m, &GPS.course_m_unit, &GPS.speed_k, &GPS.speed_k_unit, &GPS.speed_km, &GPS.speed_km_unit) >= 1)
            return;
    }
}

float GPS_nmea_to_dec(float deg_coord, char nsew) {
    int degree = (int)(deg_coord/100);
    float minutes = deg_coord - degree*100;
    float dec_deg = minutes / 60;
    float decimal = degree + dec_deg;
    if (nsew == 'S' || nsew == 'W') { // return negative
        decimal *= -1;
    }
    return decimal;
}
