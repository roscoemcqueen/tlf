import requests
import configparser

# Function to calculate Maidenhead Grid Square
def calculate_maidenhead_grid(latitude, longitude):
    # Define the characters used for grid square encoding
    alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

    # Convert latitude and longitude to float
    latitude = float(latitude)
    longitude = float(longitude)

    # Calculate the latitude and longitude portions of the grid square
    lat_index = int((90 + latitude) / 10)
    lon_index = int((180 + longitude) / 20)

    # Calculate the characters for the grid square
    grid_square = alphabet[lon_index] + alphabet[lat_index]

    # Calculate the subsquare within the grid square
    lat_remainder = (90 + latitude) % 10
    lon_remainder = (180 + longitude) % 20
    grid_square += str(int(lon_remainder)) + str(int(lat_remainder)

    return grid_square

# Initialize the configuration parser
config = configparser.ConfigParser()

# Read the configuration file
config.read('logcfg.dat')

# Read the APRS_CALL and APRS_API fields
aprs_call = config.get('Configuration', 'APRS_CALL')
aprs_api = config.get('Configuration', 'APRS_API')

# Your API key (You can replace this with your actual APRS.fi API key)
api_key = aprs_api

# Use the APRS_CALL from configuration in the API request
call_sign = aprs_call
