/**
  ******************************************************************************
  * @file           : neo6m.h
  * @brief          : Header file for NEO-6M sensor driver
  * @author         : Clarke Needles
  ******************************************************************************
  */

#define GPSBUFSIZE  128       // GPS buffer size

/*
 * ============================================================================
 *                     	  ##### TYPE DEFINITIONS #####
 * ============================================================================
 */

typedef struct{
    // Calculated values
    float dec_longitude;
    float dec_latitude;
    float altitude_ft;

    // GGA - Global Positioning System Fixed Data
    float nmea_longitude;
    float nmea_latitude;
    float utc_time;
    char ns, ew;
    int lock;
    int satelites;
    float hdop;
    float msl_altitude;
    char msl_units;

    // RMC - Recommended Minimmum Specific GNS Data
    char rmc_status;
    float speed_k;
    float course_d;
    int date;

    // GLL
    char gll_status;

    // VTG - Course over ground, ground speed
    float course_t; // ground speed true
    char course_t_unit;
    float course_m; // magnetic
    char course_m_unit;
    char speed_k_unit;
    float speed_km; // speek km/hr
    char speed_km_unit;
} NEO6M_Data_t;

typedef struct
{
	UART_HandleTypeDef *huart;

	// Receive buffer
	uint8_t rx_data = 0;
	uint8_t rx_buffer[GPSBUFSIZE];
	uint8_t rx_index = 0;
} NEO6M_Handle_t;

/*
 * ============================================================================
 *                     	   ##### PUBLIC API #####
 * ============================================================================
 */

void NEO6M_Init(NEO6M_Handle_t *dev);
void NEO6M_UART_CallBack(NEO6M_Handle_t *dev);
uint8_t NEO6M_Validate(char *nmea_str);
void NEO6M_Parse(char *GPSstrParse);
float GPS_nmea_to_dec(float deg_coord, char nsew);
