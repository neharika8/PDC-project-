#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <omp.h>
using namespace std;

struct Accident
{
  int id, vehicles, injuries, fatalities, nearby_accidents, hour;
  double lat, lon;
  string severity, road_condition, weather;
  string accident_cause, traffic_density, lane_utilization;
  // date/time not stored as strings to reduce memory; hour precomputed
};

string trim(const string &str)
{
  size_t start = str.find_first_not_of(" \t"), end = str.find_last_not_of(" \t");
  return (start == string::npos) ? "" : str.substr(start, end - start + 1);
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
    return (hour >= 0 && hour < 24) ? hour : -1;
  }
  catch (...)
  {
    return -1;
  }
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
      // Precompute hour from time string (tokens[2])
      a.hour = extractHour(tokens[2]);
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

template <typename K, typename V>
void printMap(const unordered_map<K, V> &map, const string &title)
{
  cout << title << ": ";
  for (auto &p : map)
    cout << p.first << "=" << p.second << " ";
  cout << "\n";
}

void analyze(const vector<Accident> &accidents)
{
  int n = accidents.size();
  if (n == 0)
    return;

  // Global accumulators
  long long total_inj = 0, total_fat = 0, total_veh = 0, total_nearby = 0;
  vector<int> hourly(24, 0);

// Parallel numerical + hourly aggregation
#pragma omp parallel
  {
    long long local_inj = 0, local_fat = 0, local_veh = 0, local_nearby = 0;
    vector<int> local_hourly(24, 0);

#pragma omp for schedule(static)
    for (int i = 0; i < n; i++)
    {
      local_inj += accidents[i].injuries;
      local_fat += accidents[i].fatalities;
      local_veh += accidents[i].vehicles;
      local_nearby += accidents[i].nearby_accidents;
      int h = accidents[i].hour;
      if (h >= 0 && h < 24)
        local_hourly[h]++;
    }

// Merge local results using atomic updates
#pragma omp atomic
    total_inj += local_inj;
#pragma omp atomic
    total_fat += local_fat;
#pragma omp atomic
    total_veh += local_veh;
#pragma omp atomic
    total_nearby += local_nearby;

    for (int h = 0; h < 24; ++h)
    {
      if (local_hourly[h] != 0)
      {
#pragma omp atomic
        hourly[h] += local_hourly[h];
      }
    }
  }

  // Parallel string aggregation with thread-local maps
  int num_threads = omp_get_max_threads();
  vector<unordered_map<string, int>> local_sev(num_threads);
  vector<unordered_map<string, int>> local_cause(num_threads);
  vector<unordered_map<string, int>> local_traffic(num_threads);
  vector<unordered_map<string, int>> local_weather(num_threads);

#pragma omp parallel
  {
    int tid = omp_get_thread_num();
#pragma omp for schedule(static)
    for (int i = 0; i < n; i++)
    {
      local_sev[tid][accidents[i].severity]++;
      local_cause[tid][accidents[i].accident_cause]++;
      local_traffic[tid][accidents[i].traffic_density]++;
      local_weather[tid][accidents[i].weather]++;
    }
  }

  // Merge thread-local maps
  unordered_map<string, int> sevCount, causeCount, trafficCount, weatherCount;
  for (int t = 0; t < num_threads; t++)
  {
    for (auto &p : local_sev[t])
      sevCount[p.first] += p.second;
    for (auto &p : local_cause[t])
      causeCount[p.first] += p.second;
    for (auto &p : local_traffic[t])
      trafficCount[p.first] += p.second;
    for (auto &p : local_weather[t])
      weatherCount[p.first] += p.second;
  }

  // Output
  double avg_nearby = static_cast<double>(total_nearby) / n;
  cout << "\n--- BASIC STATISTICS ---\n";
  cout << "Total Accidents: " << n << "\nTotal Injuries: " << total_inj
       << "\nTotal Fatalities: " << total_fat << "\nAvg Vehicles/accident: " << fixed << setprecision(2)
       << static_cast<double>(total_veh) / n << "\nAvg Nearby Accidents: " << avg_nearby << "\n";

  cout << "\n--- KEY DISTRIBUTIONS ---\n";
  printMap(sevCount, "Severity");
  printMap(causeCount, "Accident Cause");
  printMap(trafficCount, "Traffic Density");
  printMap(weatherCount, "Weather");

  int peak_hour = max_element(hourly.begin(), hourly.end()) - hourly.begin();
  cout << "\n--- TEMPORAL PATTERNS ---\n";
  cout << "Peak Hour: " << setfill('0') << setw(2) << peak_hour
       << ":00 (" << hourly[peak_hour] << " accidents)\n";

  int high_cluster = 0;
#pragma omp parallel for reduction(+ : high_cluster)
  for (int i = 0; i < n; i++)
    if (accidents[i].nearby_accidents > 20)
      high_cluster++;

  cout << "Accidents in High-Cluster Zones (>20 nearby): " << high_cluster << "\n";
}

int main()
{
  cout << "Name: A. Pranathi, Roll No: 2023BCD0065\n";
  cout << "Name: K. Neharika, Roll No: 2023BCD0053\n\n";

  string originalFile = "india_traffic_accidents.csv";
  cout << "\nAnalyzing original dataset: " << originalFile << "\n"
       << string(60, '=') << "\n";

  auto start_load = chrono::high_resolution_clock::now();
  auto accidents = readData(originalFile);
  auto end_load = chrono::high_resolution_clock::now();

  if (accidents.empty())
  {
    cout << "Warning: Dataset not found or empty.\n";
  }
  else
  {
    cout << "Loaded " << accidents.size() << " records in " << fixed << setprecision(3)
         << chrono::duration<double>(end_load - start_load).count() << " seconds\n";

    auto start_analysis = chrono::high_resolution_clock::now();
    analyze(accidents);
    auto end_analysis = chrono::high_resolution_clock::now();

    cout << "[Analysis completed in "
         << chrono::duration<double>(end_analysis - start_analysis).count()
         << " seconds]\n";
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

    auto start_load = chrono::high_resolution_clock::now();
    auto accidents = readData(userFile);
    auto end_load = chrono::high_resolution_clock::now();

    if (accidents.empty())
    {
      cout << "No valid data found in '" << userFile << "'.\n";
      continue;
    }

    cout << "\nLoaded " << accidents.size() << " records in " << fixed << setprecision(3)
         << chrono::duration<double>(end_load - start_load).count() << " seconds\n";

    auto start_analysis = chrono::high_resolution_clock::now();
    analyze(accidents);
    auto end_analysis = chrono::high_resolution_clock::now();

    cout << "[Analysis completed in "
         << chrono::duration<double>(end_analysis - start_analysis).count()
         << " seconds]\n";
  }

  cout << "Exiting program.\n";
  return 0;
}