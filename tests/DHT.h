#ifndef DHT_H
#define DHT_H
class DHT {
public:
    DHT() {}
    DHT(int p, int t) {}
    void begin() {}
    float readTemperature() { return 25.0; }
    float readHumidity() { return 50.0; }
};
#define DHT22 22
#endif
