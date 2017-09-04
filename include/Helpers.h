#ifndef _FORMAIS_HELPERS_
#define _FORMAIS_HELPERS_
#include <random>
#include <thread>
#include <mutex>
#include <queue>
#include <stdexcept>
namespace Helpers{
	class Random{
		std::random_device rd;
		std::mt19937 gen;
		std::uniform_real_distribution<> dis;
		public:
		Random() = delete;
		Random(int lowerBound, int upperBound) : dis(lowerBound, upperBound){
			gen.seed(rd());
		} 
		~Random(){}
		double operator() ();
	};
//--------------------------------------------------------------//
	template<typename T>
	class SafeQueue{
		std::mutex mutex;
		typename std::queue<T> container;
		public: 
		SafeQueue(){}
		T front();
		void push(T value);
		bool empty();
		std::size_t size();
	};
}
//////////////////////////////////////////////////////////////////
double Helpers::Random::operator()(){
	return dis(gen);
}
template<typename T>
bool Helpers::SafeQueue<T>::empty(){
	std::lock_guard<std::mutex> guard(mutex);
	return container.empty();
}

template<typename T>
T Helpers::SafeQueue<T>::front(){
	std::lock_guard<std::mutex> guard(mutex);
	T a = container.front();
	if(container.empty()) throw std::runtime_error("Stack is empty");
	container.pop();
	return a;
}

template <typename T>
void Helpers::SafeQueue<T>::push(T value){
	std::lock_guard<std::mutex> guard(mutex);
	container.push(value);
}

template<typename T>
std::size_t Helpers::SafeQueue<T>::size(){
	std::lock_guard<std::mutex> guard(mutex);
	return container.size();
}

M
#endif
