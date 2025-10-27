#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <iomanip>
#include <chrono>
using namespace std;

struct Accident
{
  int id, vehicles, injuries, fatalities, nearby_accidents;
  double lat, lon;
  string date, time, severity, road_condition, weather;
  string accident_cause, traffic_density, lane_utilization;
};

string trim(const string &str)
{
  size_t start = str.find_first_not_of(" \t"), end = str.find_last_not_of(" \t");
  return (start == string::npos) ? "" : str.substr(start, end - start + 1);
}

vector<Accident> readData(const string &filename)
{
  vector<Accident> data;
  ifstream file(filename);
  if (!file.is_open())
  {
    cerr << "Error opening file '" << filename << "'\n";
    return data;
  }
  string line;
  getline(file, line); // skip header
  int lineNum = 1;
  while (getline(file, line))
  {
    lineNum++;
    if (line.empty())
      continue;
    stringstream ss(line);
    string token;
    vector<string> tokens;
    while (getline(ss, token, ','))
      tokens.push_back(trim(token));
    if (tokens.size() != 15)
    {
      cerr << "Skipping line " << lineNum << " (expected 15 cols)\n";
      continue;
    }
    try
    {
      Accident a;
      a.id = stoi(tokens[0]);
      a.date = tokens[1];
      a.time = tokens[2];
      a.lat = stod(tokens[3]);
      a.lon = stod(tokens[4]);
      a.severity = tokens[5];
      a.road_condition = tokens[6];
      a.weather = tokens[7];
      a.vehicles = stoi(tokens[8]);
      a.injuries = stoi(tokens[9]);
      a.fatalities = stoi(tokens[10]);
      a.accident_cause = tokens[11];
      a.traffic_density = tokens[12];
      a.lane_utilization = tokens[13];
      a.nearby_accidents = stoi(tokens[14]);
      data.push_back(a);
    }
    catch (...)
    {
      cerr << "Parse error on line " << lineNum << "\n";
    }
  }
  return data;
}

int extractHour(const string &t)
{
  if (t.empty())
    return -1;

  size_t pos = t.find(':');
  if (pos == string::npos)
    return -1;

  try
  {
    int hour = stoi(t.substr(0, pos));
    if (hour >= 0 && hour < 24)
      return hour;
    else
      return -1;
  }
  catch (...)
  {
    return -1;
  }
}

template <typename K, typename V>
void printMap(const unordered_map<K, V> &map, const string &title)
{
  cout << title << ": ";
  for (auto &p : map)
    cout << p.first << "=" << p.second << " ";
  cout << "\n";
}

// Analysis Function
void analyze(const vector<Accident> &accidents)
{
  long long total_inj = 0, total_fat = 0, total_veh = 0, total_nearby = 0;
  unordered_map<string, int> sevCount, causeCount, trafficCount, weatherCount;
  vector<int> hourly(24, 0);

  for (auto &a : accidents)
  {
    total_inj += a.injuries;
    total_fat += a.fatalities;
    total_veh += a.vehicles;
    total_nearby += a.nearby_accidents;
    sevCount[a.severity]++;
    causeCount[a.accident_cause]++;
    trafficCount[a.traffic_density]++;
    weatherCount[a.weather]++;
    int h = extractHour(a.time);
    if (h >= 0 && h < 24)
      hourly[h]++;
  }

  double avg_nearby = static_cast<double>(total_nearby) / accidents.size();
  cout << "\n--- BASIC STATISTICS ---\n";
  cout << "Total Accidents: " << accidents.size() << "\nTotal Injuries: " << total_inj
       << "\nTotal Fatalities: " << total_fat << "\nAvg Vehicles/accident: " << fixed << setprecision(2)
       << static_cast<double>(total_veh) / accidents.size() << "\nAvg Nearby Accidents: " << avg_nearby << "\n";

  cout << "\n--- KEY DISTRIBUTIONS ---\n";
  printMap(sevCount, "Severity");
  printMap(causeCount, "Accident Cause");
  printMap(trafficCount, "Traffic Density");
  printMap(weatherCount, "Weather");

  int peak_hour = max_element(hourly.begin(), hourly.end()) - hourly.begin();
  cout << "\n--- TEMPORAL PATTERNS ---\n";
  cout << "Peak Hour: " << setfill('0') << setw(2) << peak_hour
       << ":00 (" << hourly[peak_hour] << " accidents)\n";

  int high_cluster = count_if(accidents.begin(), accidents.end(), [](const Accident &a)
                              { return a.nearby_accidents > 20; });
  cout << "Accidents in High-Cluster Zones (>20 nearby): " << high_cluster << "\n";
}

// Main function
int main()
{
  cout << "Name: A. Pranathi, Roll No: 2023BCD0065\n";
  cout << "Name: K. Neharika, Roll No: 2023BCD0053\n"
       << endl;
  string originalFile = "india_traffic_accidents.csv";
  cout << "\nAnalyzing original dataset: " << originalFile << "\n"
       << string(60, '=') << "\n";

  auto start = chrono::high_resolution_clock::now();
  auto accidents = readData(originalFile);
  auto end = chrono::high_resolution_clock::now();

  if (accidents.empty())
    cout << "Warning: Dataset not found or empty.\n";
  else
  {
    cout << "Loaded " << accidents.size() << " records in " << fixed << setprecision(3)
         << chrono::duration<double>(end - start).count() << " seconds\n";

    auto analyze_start = chrono::high_resolution_clock::now();
    analyze(accidents);
    auto analyze_end = chrono::high_resolution_clock::now();

    cout << "Analysis completed in " << fixed << setprecision(3)
         << chrono::duration<double>(analyze_end - analyze_start).count() << " seconds\n";
  }

  cout << "\n"
       << string(60, '=') << "\nINTERACTIVE MODE\n"
       << string(60, '=') << "\n";
  string userFile;
  while (true)
  {
    cout << "\nEnter CSV filename (or 'quit' to exit): ";
    getline(cin, userFile);
    userFile = trim(userFile);
    if (userFile == "quit" || userFile.empty())
      break;

    auto start = chrono::high_resolution_clock::now();
    auto accidents = readData(userFile);
    auto end = chrono::high_resolution_clock::now();

    if (accidents.empty())
    {
      cout << "No valid data found in '" << userFile << "'.\n";
      continue;
    }

    cout << "\nLoaded " << accidents.size() << " records in " << fixed << setprecision(3)
         << chrono::duration<double>(end - start).count() << " seconds\n";

    auto analyze_start = chrono::high_resolution_clock::now();
    analyze(accidents);
    auto analyze_end = chrono::high_resolution_clock::now();

    cout << "Analysis completed in " << fixed << setprecision(3)
         << chrono::duration<double>(analyze_end - analyze_start).count() << " seconds\n";
  }

  cout << "Exiting program.\n";
  return 0;
}