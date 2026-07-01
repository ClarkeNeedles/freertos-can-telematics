/**
  ******************************************************************************
  * @file           : neo6m.c
  * @brief          : NEO-6M sensor driver
  * @author         : Clarke Needles
  ******************************************************************************
  */

#include "neo6m.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * ============================================================================
 *                  ##### PRIVATE FUNCTION PROTOTYPES  #####
 * ============================================================================
 */

static uint8_t NEO6M_Validate(char *rx_buffer);
static NEO6M_Status_t NEO6M_Parse(char *rx_buffer, NEO6M_Data_t *data);
static float NEO6M_NmeaToDec(float deg_coord, char ns_ew);
 
/*
 * ============================================================================
 *                     	   ##### PUBLIC API #####
 * ============================================================================
 */

// -----------------------------------------------------------------------------
NEO6M_Status_t NEO6M_Init(NEO6M_Handle_t *dev)
{
	dev->rx_data = 0;
	dev->rx_index = 0;

    // Receive data over UART using interrupts
    if (HAL_UART_Receive_IT(dev->huart, &dev->rx_data, 1) != HAL_OK)
    {
        return NEO6M_ERR_UART;
    }

    return NEO6M_OK;
}

// -----------------------------------------------------------------------------
NEO6M_Status_t NEO6M_UART_CallBack(NEO6M_Handle_t *dev)
{
    if ((dev->rx_data != '\n') && (dev->rx_index < GPSBUFSIZE))
    {
        dev->rx_buffer[dev->rx_index++] = dev->rx_data; // Move data into buffer
    }
    else
    {
        if(NEO6M_Validate((char*) dev->rx_buffer))
        {
            if (NEO6M_Parse((char*) dev->rx_buffer, &dev->gps_data) != NEO6M_OK)
            {
                return NEO6M_ERR_UART;
            }
        }
        else
        {
            return NEO6M_ERR_VALIDATION;
        }

        // Reset buffer
        dev->rx_index = 0;
        memset(dev->rx_buffer, 0, GPSBUFSIZE);
    }

    // Reset the interrupt
    if (HAL_UART_Receive_IT(dev->huart, &dev->rx_data, 1) != HAL_OK)
    {
        return NEO6M_ERR_UART;
    }

    return NEO6M_OK;
}

/*
 * ============================================================================
 *                ##### PRIVATE FUNCTION IMPLEMENTATIONS #####
 * ============================================================================
 */

/*******************************************************************************
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
 *******************************************************************************/
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

/*******************************************************************************
 * @brief  Parses raw NMEA strings and extracts active telemetry fields.
 * @note   Non-destructive parser; scans fields safely without modifying buffer memory.
 *
 * @param[in]  rx_buffer Pointer to the verified, null-terminated character string.
 * @param[out] data      Pointer to target telemetry destination container.
 *
 * @retval NEO6M_OK          Valid telemetry parsed and committed.
 * @retval NEO6M_ERR_NO_FIX  The module has no active satellite lock.
 *******************************************************************************/
static NEO6M_Status_t NEO6M_Parse(char *rx_buffer, NEO6M_Data_t *data)
{
    char local_lat[15] = {0};
    char local_lon[15] = {0};
    char ns_val = 0;
    char ew_val = 0;
    char status = 'V';

    // We are only using GPRMC
    if (strncmp(rx_buffer, "$GPRMC", 6) != 0)
    {
        return NEO6M_ERR_NO_FIX;
    }

    // Safe non-destructive parsing tracking field boundaries by comma pointers
    char *p_field = rx_buffer;
    int field_index = 1;

    while (p_field != NULL)
    {
        p_field = strchr(p_field, ',');
        if (p_field == NULL)
        {
            break;
        }
        
        p_field++; // Move past the comma token
        field_index++;

        // Calculate length of the isolated text string segment inside commas
        char *p_next_comma = strchr(p_field, ',');
        size_t field_len = (p_next_comma != NULL) ? (size_t)(p_next_comma - p_field) : strlen(p_field);

        if (field_len == 0)
        {
            continue;
        }

        switch (field_index)
        {
            case 3: // Status field validation: 'A' = Active/Valid, 'V' = Void/Warning
                status = p_field[0];
                break;
            case 4: // Raw latitude conversion block
                if (field_len < sizeof(local_lat))
                {
                  strncpy(local_lat, p_field, field_len);
                  local_lat[field_len] = '\0';
                }
                break;
            case 5: // North / South indicator vector flag
                ns_val = p_field[0];
                break;
            case 6: // Raw longitude conversion block
                if (field_len < sizeof(local_lon))
                {
                  strncpy(local_lon, p_field, field_len);
                  local_lon[field_len] = '\0';
                }
                break;
            case 7: // East / West indicator vector flag
                ew_val = p_field[0];
                break;
        }
    }

    if (status == 'A' && local_lat[0] != '\0' && local_lon[0] != '\0')
    {
        float nmea_latitude = strtof(local_lat, NULL);
        float nmea_longitude = strtof(local_lon, NULL);

        data->dec_latitude = NEO6M_NmeaToDec(nmea_latitude, ns_val);
        data->dec_longitude = NEO6M_NmeaToDec(nmea_longitude, ew_val);
        data->valid_fix = 1;

        return NEO6M_OK;
    }

    data->valid_fix = 0;
    return NEO6M_ERR_NO_FIX;
} 

/*******************************************************************************
 * @brief  Converts standard NMEA DDMM.MMMM format coordinates to decimal degrees.
 * @note   Internal math helper utility used exclusively within this file.
 *
 * @param[in] deg_coord Raw coordinate scalar read directly from the NMEA packet stream.
 * @param[in] ns_ew     Cardinal direction indicator character ('N', 'S', 'E', or 'W').
 * @return              The final calibrated coordinate position in true Decimal Degrees.
 *******************************************************************************/
static float NEO6M_NmeaToDec(float deg_coord, char ns_ew) 
{
    int degree = (int)(deg_coord / 100);
    float minutes = deg_coord - degree * 100;
    float dec_deg = minutes / 60;
    float decimal = degree + dec_deg;

    if (ns_ew == 'S' || ns_ew == 'W') 
    { 
        decimal *= -1.0f; // return negative
    }

    return decimal;
}
