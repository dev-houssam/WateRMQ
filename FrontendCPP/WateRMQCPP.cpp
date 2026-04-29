#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>
#include <memory>
#include <string>

#include "WateRMQ.hpp"

using json = nlohmann::json;
using namespace std;

class Notifier {
public:
    virtual ~Notifier() = default;
    virtual void sendNotification(const string& sensorId, 
                                  const string& message,
                                  const string& timestamp) = 0;
};

class ConsoleNotifier : public Notifier {
public:
    void sendNotification(const string& sensorId,
                          const string& message,
                          const string& timestamp) override {
        cout << "[NOTIFICATION] Sensor: " << sensorId 
             << " | Message: " << message 
             << " | Time: " << timestamp << endl;
    }
};

class FileNotifier : public Notifier {
private:
    string _filename;
public:
    FileNotifier(const string& filename) : _filename(filename) {}
    
    void sendNotification(const string& sensorId,
                          const string& message,
                          const string& timestamp) override {
        // Implementation would write to file
        cout << "[FILE] Would write to " << _filename << endl;
    }
};

int main() {
    cout << "=== WateRMQ Consumer Starting ===" << endl;
    
    WateRMQ rabbit("watemq_shm");
    
    rabbit.setExceptionMessage(
        "Login error", 
        "Opening error", 
        "Queue error", 
        "Binding error", 
        "Consuming error", 
        "Closing channel error",
        "Closing connection error", 
        "Ending Connection error");
    
    rabbit.declareExchange(
        "raw_data_exchange", 
        "capteurs_data", 
        "capteurs_data");
    
    rabbit.connection("localhost", 5672);  // RabbitMQ default port is 5672, not 15672
    
    rabbit.login("/", 0, 0, 0, "guest", "guest");
    
    // Create notifiers
    vector<unique_ptr<Notifier>> notifiers;
    notifiers.push_back(make_unique<ConsoleNotifier>());
    notifiers.push_back(make_unique<FileNotifier>("notifications.log"));
    
    rabbit.start();
    
    cout << "Listening for messages... Press Ctrl+C to stop" << endl;
    
    int message_count = 0;
    while (true) {
        try {
            string data_from_queue = rabbit.consumingMessage();
            if (!data_from_queue.empty()) {
                message_count++;
                cout << "\n--- Message #" << message_count << " ---" << endl;
                cout << "Raw: " << data_from_queue << endl;
                
                auto data = json::parse(data_from_queue);
                string sensorId = data.value("sensor_id", "unknown");
                string message = data.value("message", "No message");
                string timestamp = data.value("timestamp", "unknown");
                string severity = data.value("severity", "info");
                
                cout << "Parsed - Sensor: " << sensorId 
                     << " | Severity: " << severity << endl;
                
                for (auto& notifier : notifiers) {
                    notifier->sendNotification(sensorId, message, timestamp);
                }
            }
        } catch (const json::parse_error& e) {
            cerr << "JSON parse error: " << e.what() << endl;
        } catch (const exception& e) {
            cerr << "Error: " << e.what() << endl;
        }
        
        // Small sleep to prevent CPU spinning
        usleep(10000);  // 10ms
    }
    
    rabbit.destroy();
    cout << "=== WateRMQ Consumer Finished ===" << endl;
    
    return 0;
}
