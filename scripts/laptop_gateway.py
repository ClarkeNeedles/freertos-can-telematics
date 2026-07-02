import struct
import can
import openmeteo_requests
import requests_cache
from retry_requests import retry

# Configure the CAN bus interface for the SH-C30A adapter
# Windows utilizes the 'gs_usb' backend
#bus = can.Bus(interface='gs_usb', channel=0, bitrate=500000)

# Define application CAN Identifiers (Match these in your STM32 code)
STM32_GPS_CAN_ID = 0x123
LAPTOP_WX_CAN_ID = 0x200

# Setup the Open-Meteo API Client with persistent local cache and automatic retries
cache_session = requests_cache.CachedSession('.cache', expire_after=3600)
retry_session = retry(cache_session, retries=5, backoff_factor=0.2)
openmeteo = openmeteo_requests.Client(session=retry_session)

def fetch_weather_data(lat, lon):
    """
    Fetches real-time weather metrics using the official openmeteo-requests SDK.
    Parses flat data arrays directly using flat-binary Protocol Buffers.
    """
    url = "https://api.open-meteo.com/v1/forecast"
    
    # Target only the immediate "current" parameter metrics
    params = {
        "latitude": lat,
        "longitude": lon,
        "current": ["temperature_2m", "relative_humidity_2m", "wind_speed_10m", "weather_code"],
        "models": "gem_seamless"  # Forces the use of Canadian Meteorological Centre's model
    }
    
    try:
        # SDK query returns a list of location responses
        responses = openmeteo.weather_api(url, params=params)
        response = responses[0]
        
        # Access the real-time "Current" data block properties
        current = response.Current()
        
        # Variables match the order provided in the params["current"] list array index
        return {
            "temp_c": float(current.Variables(0).Value()),
            "humidity": float(current.Variables(1).Value()),
            "wind_speed": float(current.Variables(2).Value()),
            "condition_id": int(current.Variables(3).Value())
        }
        
    except Exception as e:
        print(f"[-] Open-Meteo SDK Processing Failure: {e}")
        return None

def send_weather_to_stm32(weather_dict):
    """
    Packs data into an 8-byte frame using 16-bit half-precision floats.
    Format string elements:
      'e' = 16-bit float (2 bytes)
      'H' = 16-bit unsigned integer (2 bytes)
    Layout: [3 x 16-bit Float] [1 x 16-bit Integer ID] = 8 Bytes
    """
    temp = weather_dict["temp_c"]
    wind = weather_dict["wind_speed"]
    humidity = weather_dict["humidity"]
    w_id = weather_dict["condition_id"]
    
    # 'eee' packs three 16-bit floats, 'H' packs one 16-bit unsigned integer
    payload = struct.pack("eeeH", temp, wind, humidity, w_id)
    
    msg = can.Message(
        arbitration_id=LAPTOP_WX_CAN_ID,
        data=payload,
        is_extended_id=False
    )
    
    try:
        bus.send(msg)
        print(f"[+] TX -> STM32 | Packed 8-bytes via FP16 successfully.")
    except can.CanError:
        print("[-] CAN Transmission Error")

def main_rx_loop():
    print("[*] Monitoring CAN Bus for STM32 GPS data transmissions...")
    
    while True:
        # Blocks execution until a packet arrives on the bus interface
        msg = bus.recv(timeout=1.0)
        if msg is None:
            continue
            
        # Target only the incoming GPS packet identifier
        if msg.arbitration_id == STM32_GPS_CAN_ID:
            if len(msg.data) == 8:
                # Unpack the 8-byte payload back into two standard 32-bit floats
                latitude, longitude = struct.unpack("ff", msg.data)
                print(f"\n[+] RX <- STM32 | Latitude: {latitude:.5f} | Longitude: {longitude:.5f}")
                
                # Fetch data using the parsed coordinates
                weather = fetch_weather_data(latitude, longitude)
                
                if weather:
                    # Echo the returned metrics back down to the STM32 nodes
                    send_weather_to_stm32(weather)
            else:
                print(f"[-] Erroneous data frame layout detected. Expected 8 bytes, got {len(msg.data)}")

if __name__ == "__main__":
    try:
        main_rx_loop()
    except KeyboardInterrupt:
        print("\n[*] Script terminated by user. Releasing CAN Bus interface.")
        bus.shutdown()
