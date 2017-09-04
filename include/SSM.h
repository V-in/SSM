#ifndef ___STATE___
#define ___STATE___
#include <vector>
#include <list>
#include <cassert>
#include "./Helpers.h"
#include <unordered_set>
#include <algorithm>
#include <set>
#include <map>
#include <chrono>
#include <functional>
#include <strstream>
#include <thread>
#include <array>
#include <utility>
#include <random>

#define SLEEP_DURATION 500

template <typename Symbol_t, typename StateId_t = int>
class SSM{
	Helpers::Random random;
	Helpers::SafeQueue<Symbol_t> inputQueue;
	class State{
		StateId_t thisId;
		std::map<Symbol_t, std::vector<std::map<StateId_t,double>> > nextStates;
		public:
		State(StateId_t id) : thisId(id){}
		StateId_t getId() const;
		const std::map<Symbol_t, std::vector<std::map<StateId_t, double>> >& getNextStates() const;	
		void addNextState(Symbol_t, StateId_t, double);
		bool operator== (const State& other) const;
		bool operator< (const State& other) const;
	};

	typedef std::pair<StateId_t, std::pair<Symbol_t, StateId_t> > transition; 

    typename std::vector<Symbol_t> alphabet;
	Symbol_t emptyString;
	typename std::set<State> states;
	std::vector<double> weights;
	std::map<transition, std::function<void(void)> > transitionSideEffects;


	StateId_t startState;
	StateId_t currentState; 
	std::thread loopThread;
	public:
	SSM() = delete;
	SSM(std::vector<Symbol_t> alpha, std::set<StateId_t> states_, StateId_t start, Symbol_t empty){};
	SSM(std::vector<Symbol_t> alpha, StateId_t start, Symbol_t empty);
	~SSM(){
		if(loopThread.joinable())
			loopThread.join();
	}
	void addState(StateId_t s);
	void setStart(StateId_t s);
	void addTransition(StateId_t start, Symbol_t input, StateId_t end, double probability, double weight, const std::function<void(void)>& sideEffect = nullptr);
	void advanceState(const Symbol_t input, const State& state);
	void run();
	void listStates();
	void queueInput(Symbol_t);
	StateId_t getCurrentState();
	void verify();
//TODO-----------------------------------------
//	void makeDeterministic();
//	bool isDeterministic();
};
template <typename Symbol_t, typename StateId_t>
StateId_t SSM<Symbol_t, StateId_t>::getCurrentState(){
	return currentState;
}
template <typename Symbol_t, typename StateId_t>
void SSM<Symbol_t, StateId_t>::queueInput(Symbol_t input){
	inputQueue.push(input);
}

template<typename Symbol_t, typename StateId_t>
void SSM<Symbol_t, StateId_t>::advanceState(const Symbol_t input, const SSM<Symbol_t, StateId_t>::State& state){
	typedef const std::vector<std::map<StateId_t, double> >& refType;
	transition trans;
 	refType outs = (*states.find(currentState)).getNextStates().at(input);
	double rand = random();
	double acc = 0;
	for(auto outS : outs){
		for(auto out : outS){
			acc+= out.second/100;
			if(acc >= rand){
				currentState = out.first;
			}
		}
	}
}
/*
template<typename Symbol_t, typename StateId_t>
bool SSM<Symbol_t, StateId_t>::isDeterministic(){
	std::vector<Symbol_t> linkedSymbols;
	double acc;
	for(auto state : states){
		linkedSymbols.clear();
		for(auto symbol : state.getNextStates()){
			for(auto symbolOuts : symbol){
				for(auto out : symbolOuts){
					linkedSymbols.push_back(out.first);
				}
			}
		}
	}
	std::vector<Symbol_t> fullAlphabet = alphabet;
	fullAlphabet.push_back(emptyString);
	return (std::equal(linkedSymbols.begin(), linkedSymbols.end(), fullAlphabet));
}
*/
template <typename Symbol_t, typename StateId_t>
void SSM<Symbol_t, StateId_t>::State::addNextState(Symbol_t input, StateId_t endState, double probability){
	for(auto e : nextStates[input]){
		for(auto w : e){
			if(w.first == endState)
				throw std::runtime_error("ERROR: Duplicate transition");
		}
	}
	nextStates[input].push_back({{endState, probability}});
}

template<typename Symbol_t, typename StateId_t>
void SSM<Symbol_t, StateId_t>::addTransition (StateId_t start_, Symbol_t input, StateId_t end_, double probability, double weight, const std::function<void(void)>& sideEffect){
	typedef typename std::set<SSM<Symbol_t, StateId_t>::State>::iterator iterator;
	auto start = SSM<Symbol_t, StateId_t>::State(start_); 
	iterator it = states.find(start);
	if(it == states.end())
		std::cerr << "Error, start state \"" << start_ << "\" in transition function does not exist" << std::endl;
	else{
		//Copy State and add transition
		auto tmpCopy = const_cast<typename std::remove_const<decltype(*it)>::type >(*it);
		tmpCopy.addNextState(input, end_, probability);
		//Remove old State and insert new one
		states.erase(start);
		states.insert(tmpCopy);
		weights.push_back(weight);
		if(sideEffect != nullptr){
			auto transition_ = std::make_pair(start_, std::make_pair(input, end_) );
			transitionSideEffects[transition_] = sideEffect;
		}
	}
}

template<typename Symbol_t, typename StateId_t>
SSM<Symbol_t, StateId_t>::SSM(std::vector<Symbol_t> alpha, StateId_t start, Symbol_t empty) : random(0,1){
    if ((find(alpha.begin(), alpha.end(), empty) != alpha.end()))
		throw(std::runtime_error("Error, empty string is part of the alphabet"));
	alphabet = alpha;	
	emptyString = empty;
	addState(start);
	currentState = start;
}

template<typename Symbol_t, typename StateId_t>
bool SSM<Symbol_t, StateId_t>::State::operator< (const SSM<Symbol_t, StateId_t>::State& other) const {
	return (thisId < other.getId()) ? true:false;
}

template<typename Symbol_t, typename StateId_t>
StateId_t SSM<Symbol_t, StateId_t>::State::getId() const{
	return thisId;
}

template<typename Symbol_t, typename StateId_t>
const std::map<Symbol_t, std::vector<std::map<StateId_t, double> > >& SSM<Symbol_t, StateId_t>::State::getNextStates() const{
//	const std::map<Symbol_t, std::vector<std::map<StateId_t, double> > > copy = nextStates;
	return nextStates;
}

template<typename Symbol_t, typename StateId_t>
bool SSM<Symbol_t,StateId_t>::State::operator== (const SSM<Symbol_t, StateId_t>::State& other) const{
	return (other.id == thisId)? true:false;
}

template <typename Symbol_t, typename StateId_t>
void SSM<Symbol_t, StateId_t>::addState(StateId_t id){
	typename SSM<Symbol_t, StateId_t>::State newState(id);
	try{
		states.insert(newState);
	}
	catch(...){
		throw std::runtime_error("Failed to insert state into set of states");
	}
}

template<typename Symbol_t, typename StateId_t>
void SSM<Symbol_t, StateId_t>::verify(){
	for (auto state : states){
		for(auto input : state.getNextStates()){
			double acc = 0;
			for(auto output : input.second){
				for(auto IdAndProb : output){
					acc += IdAndProb.second;
				} 
			}
			if(acc > 100.0){
				std::strstream stream;
				stream << "Error, transition probability for symbol \"" << input.first << "\" from state \"" << state.getId() <<"\" adds up to more than 100%" << std::endl;
				throw std::runtime_error(stream.str());
			}
		}
	}
}

template <typename Symbol_t, typename StateId_t>
void SSM<Symbol_t, StateId_t>::setStart(StateId_t id){
	startState = id;
}

template <typename Symbol_t, typename StateId_t>
void SSM<Symbol_t, StateId_t>::listStates(){
	std::cout << "WARNING: Weights aren't taken into account in this function\n" << std::endl;
	for(auto state : states){
		std::cout << "Transitions for  state \"" << state.getId() << "\":\n";
		for(auto input : state.getNextStates()){
			std::cout << "\t" << input.first << " :\n";
			for(auto outputs : input.second){
				for(auto idAndProb : outputs){
					std::cout << "\t\t" << "Out: "<< idAndProb.first << ". Probability : " << idAndProb.second << "%." <<std::endl;
				}
			}
		}
		std::cout << std::endl;
	}
}

template<typename Symbol_t, typename StateId_t>
void SSM<Symbol_t, StateId_t>::run(){
	loopThread = std::thread([&](){
		while(1){
			Symbol_t currentInput = (inputQueue.empty())? emptyString : inputQueue.front();
			advanceState(currentInput, std::ref(*states.find(currentState)));
			std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_DURATION));
			std::cout << "Current state: " << currentState << std::endl;
		}
	});
}

#endif
