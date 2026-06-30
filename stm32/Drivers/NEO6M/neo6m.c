/**
  ******************************************************************************
  * @file           : neo6m.c
  * @brief          : NEO-6M sensor driver
  * @author         : Clarke Needles
  ******************************************************************************
  */

#include "neo6m.h"

/*
 * ============================================================================
 *                  ##### PRIVATE FUNCTION PROTOTYPES  #####
 * ============================================================================
 */

static uint8_t NEO6M_Validate(char *rx_buffer);
static void NEO6M_Parse(char *rx_buffer, NEO6M_Data_t *data);
static float NEO6M_NmeaToDec(float deg_coord, char nsew);
 
/*
 * ============================================================================
 *                     	   ##### PUBLIC API #####
 * ============================================================================
 */

// -----------------------------------------------------------------------------
HAL_StatusTypeDef NEO6M_Init(NEO6M_Handle_t *dev)
{
	// Receive data over UART using interrupts
	return HAL_UART_Receive_IT(dev->huart, dev->rx_data, 1);
}

// -----------------------------------------------------------------------------
HAL_StatusTypeDef NEO6M_UART_CallBack(NEO6M_Handle_t *dev)
{
	if ((dev->rx_data != '\n') && (dev->rx_index < GPSBUFSIZE))
	{
		dev->rx_buffer[dev->rx_index++] = dev->rx_data; // Move data into buffer
	}
	else
	{
		if(NEO6M_Validate((char*) dev->rx_buffer))
		{
			NEO6M_Parse((char*) dev->rx_buffer, &dev->gps_data);
		}

		// Reset buffer
		dev->rx_index = 0;
		memset(dev->rx_buffer, 0, GPSBUFSIZE);
	}

	return HAL_UART_Receive_IT(dev->huart, &dev->rx_data, 1); // Reset the interrupt
}

/*
 * ============================================================================
 *                ##### PRIVATE FUNCTION IMPLEMENTATIONS #####
 * ============================================================================
 */

/**
 * @brief  Validates the integrity of an NMEA sentence using its trailing checksum.
 * @note   Internal utility; computes the XOR hash of all characters between '$' and '*'.
 *
 * This function walks the incoming raw character string buffer, validating the starting 
 * delimiter and parsing up to the payload end-marker. It extracts the raw two-character 
 * hexadecimal validation suffix, computes an internal running XOR checksum verification 
 * frame, and evaluates parity matching.
 *
 * @param[in] rx_buffer Pointer to the null-terminated NMEA sentence array.
 * @retval 1            Sentence is valid; computed checksum matches received suffix.
 * @retval 0            Validation failed due to bad checksum, size overrun, or missing tokens.
 */
static uint8_t NEO6M_Validate(char *rx_buffer)
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

/**
 * @brief  Parses raw NMEA strings and extracts active telemetry fields.
 * @note   Internal utility; targets and extracts parameters from GGA and RMC fields.
 *
 * This function utilizes strict string format scanning to strip raw text tokens 
 * from verified sentences. It extracts time configurations, sat links, fixes, and 
 * speeds, before running coordinate normalization conversions to map raw values 
 * into decimal degrees.
 *
 * @param[in,out] rx_buffer Pointer to the verified, null-terminated character string.
 */
static void NEO6M_Parse(char *rx_buffer, NEO6M_Data_t *data)
{
    // Defensive pointer guard check
    if (rx_buffer == NULL)
    {
        return;
    }

    if (!strncmp(rx_buffer, "$GPGGA", 6))
    {
        // Verify that all 10 target variables parsed completely
        if (sscanf(rx_buffer, 
                   "$GPGGA,%f,%f,%c,%f,%c,%d,%d,%f,%f,%c",
                   &data->utc_time, &data->nmea_latitude, &data->ns, 
                   &data->nmea_longitude, &data->ew, &data->lock, 
                   &data->satelites, &data->hdop, &data->msl_altitude, 
                   &data->msl_units) == 10)
        {
            // Convert standard NMEA DDMM.MMMM to true Decimal Degrees
            data->dec_latitude = NEO6M_NmeaToDec(data->nmea_latitude, data->ns);
            data->dec_longitude = NEO6M_NmeaToDec(data->nmea_longitude, data->ew);
        }
    }
    else if (!strncmp(rx_buffer, "$GPRMC", 6))
    {
        if (sscanf(rx_buffer, 
                "$GPRMC,%f,%f,%c,%f,%c,%f,%f,%d", 
                &data->utc_time, &data->nmea_latitude, &data->ns, 
                &data->nmea_longitude, &data->ew, &data->speed_k, 
                &data->course_d, &data->date) == 8)
        {
            data->dec_latitude = NEO6M_NmeaToDec(data->nmea_latitude, data->ns);
            data->dec_longitude = NEO6M_NmeaToDec(data->nmea_longitude, data->ew);
        }
    }
} 

/**
 * @brief  Converts standard NMEA DDMM.MMMM format coordinates to decimal degrees.
 * @note   Internal math helper utility used exclusively within this file.
 *
 * This function extracts the raw degree integer from the NMEA scalar, scales the 
 * remaining minutes fractional component into base-64 decimal arcs, and applies 
 * a negative polarity inversion if a Southern ('S') or Western ('W') cardinal 
 * vector is detected.
 *
 * @param[in] deg_coord Raw coordinate scalar read directly from the NMEA packet stream.
 * @param[in] ns_ew     Cardinal direction indicator character ('N', 'S', 'E', or 'W').
 * @return              The final calibrated coordinate position in true Decimal Degrees.
 */
static float NEO6M_NmeaToDec(float deg_coord, char ns_ew) 
{
    int degree = (int)(deg_coord / 100);
    float minutes = deg_coord - degree * 100;
    float dec_deg = minutes / 60;
    float decimal = degree + dec_deg;

    if (ns_ew == 'S' || ns_ew == 'W') 
    { 
        decimal *= -1; // return negative
    }

    return decimal;
}
