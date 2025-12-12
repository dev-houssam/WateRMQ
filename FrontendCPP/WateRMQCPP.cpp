// Author : Houssam BACAR
//Compile : g++ WateRMQCPP.cpp SemaphoreCPP.cpp WateRMQ.cpp -o main

//
// #include "BackLibWateRMQ.c"

#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <nlohmann/json.hpp>
#include <semaphore.h>
#include <fcntl.h>  // For O_* constants
#include <sys/stat.h>  // For mode constants
#include <stdlib.h>

#include <vector>

#include "WateRMQ.hpp"


using json = nlohmann::json;
using namespace std;


class Notifier{};


int main(){
	cout << "Debut de test"<<endl;
	WateRMQ rabbit("semaphore-consumer");

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
	rabbit.connection(
		"localhost", 
		15672);
	rabbit.login(
		"/", 
		0, 0, 0, 
		"guest", 
		"guest");


    std::vector<Notifier> notifiers;
    //notifiers.push_back(new Notifier());

	rabbit.start();

	while(true){
		//Consommation
		try{
			auto data_from_Gal =  rabbit.consumingMessage();
			auto data = json::parse(data_from_Gal);
			std::string sensorId = data.value("sensor_id", "unknown");
		    std::string message  = data.value("message", "No message");
		    std::string timestamp= data.value("timestamp", "unknown");
		    std::string severity = data.value("severity", "info");

		    for (auto& notifier : notifiers) {
		        //notifier->sendNotification(sensorId, message, timestamp);
		    }
		}catch(const std::exception& e){
			std::cerr << "❌ Erreur parsing JSON : " << e.what() << std::endl;
		}
	}

	cout << "Fin de test"<<endl;

	return 0;
}







