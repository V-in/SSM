#ifndef THREAD_SAFE_QUEUE
#define THREAD_SAFE_QUEUE
#include <queue>
#include <thread>
#include <mutex>
#include <stdexcept>
template<typename T>
class PublisherSubscriber{
	std::mutex mutex;
	typename std::queue<T> container;
	public: 
	PublisherSubscriber(){}
	T front();
	void push(T value);
	bool empty();
	std::size_t size();
};
template<typename T>
bool PublisherSubscriber<T>::empty(){
	std::lock_guard<std::mutex> guard(mutex);
	return container.empty();
}
template<typename T>
T PublisherSubscriber<T>::front(){
	std::lock_guard<std::mutex> guard(mutex);
	T a = container.front();
	if(container.empty()) throw std::runtime_error("Stack is empty");
	container.pop();
	return a;
}
template <typename T>
void PublisherSubscriber<T>::push(T value){
	std::lock_guard<std::mutex> guard(mutex);
	container.push(value);
}
template<typename T>
std::size_t PublisherSubscriber<T>::size(){
	std::lock_guard<std::mutex> guard(mutex);
	return container.size();
}
#endif
