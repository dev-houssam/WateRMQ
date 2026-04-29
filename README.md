# WateRMQ
Juste un simple Consumer. so....

# WateRMQ - RabbitMQ Consumer with IPC Support

A C++ library for consuming RabbitMQ messages with inter-process communication (IPC) capabilities using shared memory and semaphores.

## Warning

This project is currently in development. The code has known issues that need to be fixed before production use. See the "Known Issues" section below.

## Features

- RabbitMQ message consumption
- Shared memory queue for IPC
- Semaphore-based synchronization
- JSON message parsing with nlohmann/json
- Extensible notifier system
- Configurable exception messages

## Architecture

WateRMQ uses a producer-consumer pattern with IPC:

```
[RabbitMQ] -> [Backend Consumer] -> [Shared Memory Queue] -> [Frontend Application]
                                           ^
                                           |
                                   [Semaphore Sync]
```

### Components

- **SemaphoreCPP**: Wrapper for POSIX named semaphores
- **WateRMQ**: Main library managing shared memory queue and consumer thread
- **Notifier**: Abstract interface for message notifications (console, file, etc.)

## Dependencies

- C++17 compatible compiler
- CMake 3.28.3 or higher
- pthread
- POSIX shared memory (shm_open, mmap)
- POSIX semaphores (sem_open, sem_wait, sem_post)
- nlohmann/json (for JSON parsing)

### Install dependencies (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install build-essential cmake libjsoncpp-dev nlohmann-json3-dev
```

## Building

```bash
mkdir build && cd build
cmake ..
make
```

## Usage

### Basic Example

```cpp
#include "WateRMQ.hpp"

int main() {
    // Create instance with shared memory name
    WateRMQ rabbit("my_shared_memory");
    
    // Configure connection
    rabbit.setExceptionMessage(
        "Login error", "Opening error", "Queue error",
        "Binding error", "Consuming error", "Closing channel error",
        "Closing connection error", "Ending Connection error"
    );
    
    rabbit.declareExchange("amq.topic", "routing.key", "my_queue");
    rabbit.connection("localhost", 5672);
    rabbit.login("/", 0, 0, 0, "guest", "guest");
    
    // Start consumer
    rabbit.start();
    
    // Read messages
    while (true) {
        std::string msg = rabbit.consumingMessage();
        if (!msg.empty()) {
            // Process message
            std::cout << msg << std::endl;
        }
    }
    
    rabbit.destroy();
    return 0;
}
```

### Complete Example with JSON and Notifiers

```cpp
#include "WateRMQ.hpp"
#include <nlohmann/json.hpp>
#include <memory>
#include <vector>

using json = nlohmann::json;

class ConsoleNotifier : public Notifier {
public:
    void sendNotification(const std::string& sensorId,
                          const std::string& message,
                          const std::string& timestamp) override {
        std::cout << "[NOTIFICATION] Sensor: " << sensorId 
                  << " | Message: " << message 
                  << " | Time: " << timestamp << std::endl;
    }
};

int main() {
    WateRMQ rabbit("watemq_shm");
    
    rabbit.declareExchange("raw_data_exchange", "capteurs_data", "capteurs_data");
    rabbit.connection("localhost", 5672);
    rabbit.login("/", 0, 0, 0, "guest", "guest");
    
    std::vector<std::unique_ptr<Notifier>> notifiers;
    notifiers.push_back(std::make_unique<ConsoleNotifier>());
    
    rabbit.start();
    
    while (true) {
        std::string data = rabbit.consumingMessage();
        if (!data.empty()) {
            try {
                auto json_data = json::parse(data);
                std::string sensorId = json_data.value("sensor_id", "unknown");
                std::string message = json_data.value("message", "");
                std::string timestamp = json_data.value("timestamp", "");
                
                for (auto& notifier : notifiers) {
                    notifier->sendNotification(sensorId, message, timestamp);
                }
            } catch (const json::parse_error& e) {
                std::cerr << "JSON parse error: " << e.what() << std::endl;
            }
        }
    }
    
    rabbit.destroy();
    return 0;
}
```

## API Reference

### WateRMQ Class

| Method | Description |
|--------|-------------|
| `WateRMQ(const std::string& filename)` | Constructor, creates shared memory with given name |
| `~WateRMQ()` | Destructor, cleans up resources |
| `setExceptionMessage(...)` | Configure custom error messages |
| `declareExchange(exchange, key, queue)` | Set exchange, binding key, and queue names |
| `connection(hostname, port)` | Set RabbitMQ server connection parameters |
| `login(vhost, channel_max, frame_max, heartbeat, username, password)` | Set authentication parameters |
| `start()` | Start the consumer thread |
| `destroy()` | Clean up resources (automatically called on destruction) |
| `consumingMessage()` | Get next message from queue (non-blocking) |

### SemaphoreCPP Class

| Method | Description |
|--------|-------------|
| `SemaphoreCPP(const std::string& name, bool create = true)` | Create or open named semaphore |
| `~SemaphoreCPP()` | Destructor, closes semaphore |
| `wait()` | Decrement semaphore (block if zero) |
| `post()` | Increment semaphore |
| `close()` | Close semaphore |
| `unlink()` | Remove semaphore from system |

### Notifier Abstract Class

| Method | Description |
|--------|-------------|
| `sendNotification(sensorId, message, timestamp)` | Pure virtual, implement notification logic |

## Known Issues

The current version has several known issues that need to be addressed:

1. **_init() method**: Contains an infinite loop with unreachable code
2. **SemaphoreCPP**: Mixes `sem_open` and `sem_init` incorrectly
3. **Memory management**: Potential memory leaks in queue operations
4. **Thread safety**: Queue operations not properly protected
5. **Backend consumer**: `_taskConsumer` is a placeholder, actual RabbitMQ consumption not implemented
6. **execl() usage**: Incorrect passing of std::string to execl()
7. **RAII**: Incomplete, some resources not automatically cleaned up

## Planned Fixes

- [ ] Fix _init() infinite loop
- [ ] Reimplement SemaphoreCPP with correct API
- [ ] Add proper memory cleanup
- [ ] Implement thread-safe queue operations
- [ ] Create actual RabbitMQ consumer backend
- [ ] Fix execl() arguments
- [ ] Add comprehensive RAII

## Contributing

When contributing to WateRMQ, please focus on:

1. Fixing the known issues listed above
2. Adding proper error handling
3. Implementing the actual RabbitMQ consumer
4. Adding unit tests

## Comparison with GalactikRMQ

| Feature | WateRMQ | GalactikRMQ |
|---------|---------|-------------|
| IPC support | Yes (shared memory) | No |
| Simplicity | Complex | Simple |
| Stability | In development | Stable |
| Performance | Lower (IPC overhead) | Higher |
| Use case | Multi-process | Single-process |

## License

MIT

## Disclaimer

This is a learning project. The code as provided has significant issues and should not be used in production without thorough testing and fixes.
