import csv
import random
from datetime import datetime, timedelta

num_rows = 1000000
OUTPUT_FILE = "india_traffic_accidents.csv"

MIN_LAT, MAX_LAT = 8.0, 37.0
MIN_LON, MAX_LON = 68.0, 97.0

CITIES = [
    (19.0760, 72.8777), (28.6139, 77.2090), (12.9716, 77.5946),
    (22.5726, 88.3639), (17.3850, 78.4867), (13.0827, 80.2707),
    (23.0225, 72.5714), (18.5204, 73.8567), (26.9124, 75.7873),
    (25.5941, 85.1376)
]

SEVERITY_LEVELS = ["Low", "Medium", "High", "Critical"]
ROAD_CONDITIONS = ["Dry", "Wet", "Potholed", "Flooding", "Construction", "Muddy"]
WEATHER_CONDITIONS = ["Clear", "Cloudy", "Rain", "Heavy Rain", "Fog", "Dust Storm"]
ACCIDENT_CAUSES = ["Human Error", "Mechanical Failure", "Weather", "Poor Road", "Animal Crossing", "Signal Violation"]
TRAFFIC_DENSITY = ["Light", "Moderate", "Heavy"]
LANE_UTILIZATION = ["Single Lane", "Lane Change", "Overtaking", "Congested Multi-Lane"]

def get_season(month):
    if month in [3,4,5]: return "Summer"
    elif month in [6,7,8,9]: return "Monsoon"
    else: return "Winter"

def get_weather_by_season(season):
    if season == "Monsoon": return random.choices(WEATHER_CONDITIONS, weights=[5,10,50,30,3,2])[0]
    elif season == "Summer": return random.choices(WEATHER_CONDITIONS, weights=[40,30,10,2,5,13])[0]
    else: return random.choices(WEATHER_CONDITIONS, weights=[30,40,15,2,10,3])[0]

def get_road_condition(weather):
    if "Rain" in weather or weather == "Fog": return random.choices(["Wet","Flooding","Muddy"], weights=[70,20,10])[0]
    elif weather == "Dust Storm": return "Muddy"
    else: return random.choices(["Dry","Potholed","Construction"], weights=[60,30,10])[0]

def get_severity(road, weather):
    if road in ["Flooding","Muddy"] or "Heavy Rain" in weather:
        return random.choices(SEVERITY_LEVELS, weights=[5,15,40,40])[0]
    elif road == "Wet" or weather == "Rain":
        return random.choices(SEVERITY_LEVELS, weights=[10,30,40,20])[0]
    else:
        return random.choices(SEVERITY_LEVELS, weights=[50,30,15,5])[0]

def get_accident_cause(severity, weather, road):
    if weather in ["Heavy Rain","Fog","Dust Storm"]: return "Weather"
    elif road in ["Potholed","Construction","Flooding"]: return "Poor Road"
    elif severity in ["Critical","High"]:
        return random.choices(["Human Error","Signal Violation","Mechanical Failure"], weights=[60,25,15])[0]
    else:
        return random.choices(ACCIDENT_CAUSES, weights=[50,10,10,20,5,5])[0]

def get_traffic_density(hour, near_city):
    rush_hour = hour in list(range(8,12)) + list(range(17,21))
    if rush_hour and near_city:
        return random.choices(TRAFFIC_DENSITY, weights=[5,20,75])[0]
    elif near_city:
        return random.choices(TRAFFIC_DENSITY, weights=[20,50,30])[0]
    elif rush_hour:
        return random.choices(TRAFFIC_DENSITY, weights=[30,50,20])[0]
    else:
        return random.choices(TRAFFIC_DENSITY, weights=[70,25,5])[0]

def get_lane_utilization(traffic, severity):
    if traffic == "Heavy": return "Congested Multi-Lane"
    elif severity in ["Critical","High"]: return random.choices(["Lane Change","Overtaking"], weights=[60,40])[0]
    else: return random.choices(["Single Lane","Lane Change"], weights=[70,30])[0]

# NEW: Fast & realistic nearby_accidents
def get_nearby_accidents(near_city, traffic):
    if not near_city:
        return random.randint(0, 5)  # Rural: very few nearby
    elif traffic == "Heavy":
        return min(50, random.randint(20, 60))  # Urban rush hour: high clustering
    elif traffic == "Moderate":
        return random.randint(5, 25)
    else:
        return random.randint(1, 10)

# Generate and write in ONE PASS
print("Generating 1,000,000-row dataset...")

with open(OUTPUT_FILE, 'w', newline='', encoding='utf-8') as f:
    writer = csv.writer(f)
    writer.writerow([
        "id", "date", "time", "latitude", "longitude", "severity",
        "road_condition", "weather", "vehicles_involved", "injuries", "fatalities",
        "accident_cause", "traffic_density", "lane_utilization", "nearby_accidents"
    ])
    
    base_date = datetime(2020, 1, 1)
    
    for i in range(1, num_rows + 1):
        days = random.randint(0, 4*365 + 1)
        secs = random.randint(0, 86399)
        dt = base_date + timedelta(days=days, seconds=secs)
        month, hour = dt.month, dt.hour
        
        season = get_season(month)
        weather = get_weather_by_season(season)
        road = get_road_condition(weather)
        severity = get_severity(road, weather)
        
        near_city = random.random() < 0.7
        if near_city:
            city_lat, city_lon = random.choice(CITIES)
            lat = round(city_lat + random.uniform(-0.5, 0.5), 4)
            lon = round(city_lon + random.uniform(-0.5, 0.5), 4)
            lat = max(MIN_LAT, min(MAX_LAT, lat))
            lon = max(MIN_LON, min(MAX_LON, lon))
        else:
            lat = round(random.uniform(MIN_LAT, MAX_LAT), 4)
            lon = round(random.uniform(MIN_LON, MAX_LON), 4)
        
        vehicles = random.randint(1, 4)
        cause = get_accident_cause(severity, weather, road)
        traffic = get_traffic_density(hour, near_city)
        lane_use = get_lane_utilization(traffic, severity)
        nearby = get_nearby_accidents(near_city, traffic)
        
        if severity == "Critical":
            injuries, fatalities = random.randint(2, 8), random.randint(0, 3)
        elif severity == "High":
            injuries, fatalities = random.randint(1, 4), random.randint(0, 1)
        elif severity == "Medium":
            injuries, fatalities = random.randint(0, 2), 0
        else:
            injuries = fatalities = 0
        
        writer.writerow([
            i, dt.strftime("%Y-%m-%d"), dt.strftime("%H:%M"),
            lat, lon, severity, road, weather, vehicles, injuries, fatalities,
            cause, traffic, lane_use, nearby
        ])
        
        if i % 50000 == 0:
            print(f"  {i} rows written...")

print(f"\nDone! File: {OUTPUT_FILE} ({num_rows:,} rows)")
print("Time: ~5-8 seconds | Size: ~35 MB")