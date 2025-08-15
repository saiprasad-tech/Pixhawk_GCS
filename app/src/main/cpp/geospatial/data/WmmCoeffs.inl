// World Magnetic Model Coefficients Data
// Simplified stub data for demonstration
namespace pixhawk {
namespace data {

const double WMM_COEFFS[][6] = {
    // n, m, g_nm, h_nm, dg_dt, dh_dt (simplified subset)
    {1, 0, -29404.8, 0.0, 6.7, 0.0},
    {1, 1, -1450.9, 4652.5, 7.7, -25.9},
    {2, 0, -2500.1, 0.0, -11.6, 0.0},
    {2, 1, 2982.0, -2991.6, -7.8, -30.2},
    {2, 2, 1676.8, -734.6, -2.6, -23.9},
    // ... truncated for brevity
};

const int WMM_COEFFS_COUNT = sizeof(WMM_COEFFS) / sizeof(WMM_COEFFS[0]);

} // namespace data
} // namespace pixhawk