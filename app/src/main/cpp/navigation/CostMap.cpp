// Stub CostMap implementation  
namespace pixhawk {

class CostMap {
public:
    CostMap() = default;
    ~CostMap() = default;
    double getCost(int x, int y) { return 1.0; }
};

} // namespace pixhawk