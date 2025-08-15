// Geoid Height Data 
// Simplified stub data for demonstration
namespace pixhawk {
namespace data {

const double GEOID_HEIGHTS[][3] = {
    // lat, lon, height (meters)
    {-90.0, -180.0, -30.0},
    {-90.0, 0.0, -25.0},
    {-90.0, 180.0, -30.0},
    {0.0, -180.0, 15.0},
    {0.0, 0.0, 20.0},
    {0.0, 180.0, 15.0},
    {90.0, -180.0, 10.0},
    {90.0, 0.0, 15.0},
    {90.0, 180.0, 10.0},
    // ... truncated for brevity
};

const int GEOID_HEIGHTS_COUNT = sizeof(GEOID_HEIGHTS) / sizeof(GEOID_HEIGHTS[0]);

} // namespace data
} // namespace pixhawk