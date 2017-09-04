#include <iostream>
#include "../include/SSM.h"
#include <random>
#include <string>

int main(){
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(1,2);

	std::vector<int> alphabet = {1,2};
	SSM<int, std::string> a(alphabet, "Idle", 0);

	a.addState("Sleeping");
	a.addState("Eating");

	a.addTransition("Idle", 1, "Eating", 50, 1);
	a.addTransition("Idle", 1, "Sleeping", 50, 1);
	a.addTransition("Idle", 2, "Idle", 70, 1);
	a.addTransition("Idle", 2, "Eating", 30, 1);
	a.addTransition("Eating", 1, "Eating", 20, 1);
	a.addTransition("Eating", 1, "Sleeping", 80, 1);
	a.addTransition("Eating", 2, "Eating", 20, 1);
	a.addTransition("Eating", 2, "Sleeping", 80, 1);
	a.addTransition("Sleeping", 1, "Sleeping", 70, 1);
	a.addTransition("Sleeping", 2, "Sleeping", 70, 1);
	a.addTransition("Sleeping", 1, "Idle", 30, 1);
	a.addTransition("Sleeping", 2, "Idle", 30, 1);

	a.addTransition("Idle", 0, "Idle", 100, 1);
	a.addTransition("Eating", 0, "Eating", 100, 1);
	a.addTransition("Sleeping", 0, "Sleeping", 100, 1);

	a.verify();
	a.listStates();
	a.run();

	while(1){
		a.queueInput(dis(gen));
		std::this_thread::sleep_for(std::chrono::milliseconds(900));
	}
	
}
