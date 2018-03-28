#pragma once

#include <type_traits>
#include <utility>
#include <iostream>
#include <chrono>

//https://stackoverflow.com/questions/28498852/c-function-decorator/33642149



template <class T>
struct RetWrapper
{
	template <class Tfunc, class... Targs>
	RetWrapper(Tfunc &&func, Targs &&... args)
		: val(std::forward<Tfunc>(func)(std::forward<Targs>(args)...)) {}

	//T&& value() { return std::move(val); }
	T value() { return val; }


private:
	T val;
};


template <>
struct RetWrapper<void>
{
	template <class Tfunc, class... Targs>
	RetWrapper(Tfunc &&func, Targs &&... args)
	{
		std::forward<Tfunc>(func)(std::forward<Targs>(args)...);
	}

	void value() {}
};



//template <class Tfunc, class Tbefore, class Tafter>
template <class Tfunc>
//auto decorate(Tfunc &&func, Tbefore &&before, Tafter &&after)
auto AddTimer(Tfunc&& func, std::string name)
{
	return [
			func = std::forward<Tfunc>(func),
			name
			//before = std::forward<Tbefore>(before),
			//after = std::forward<Tafter>(after)
		] (auto&& ... args) -> decltype(auto)
	{
		//before(std::forward<decltype(args)>(args)...);

		using namespace std;
		typedef chrono::microseconds DurT;
		static int count = 0;
		static DurT dur_min = DurT::max();
		static DurT dur_max = DurT::min();
		static DurT dur_sum = DurT::zero();
		static auto time_start = chrono::high_resolution_clock::now();

		auto _start = chrono::high_resolution_clock::now();
		////////////////////////////////////////////////////////////
		RetWrapper<std::result_of_t<Tfunc(decltype(args)...)>> ret(
            func, std::forward<decltype(args)>(args)...
        );
		////////////////////////////////////////////////////////////
		auto _end = chrono::high_resolution_clock::now();

		DurT _duration = chrono::duration_cast<DurT>(_end - _start);
		++count;
		dur_sum += _duration;
		if(_duration < dur_min)
			dur_min = _duration;
		if(_duration > dur_max)
			dur_max = _duration;

		if( chrono::duration_cast<chrono::milliseconds>(
				chrono::high_resolution_clock::now() - time_start).count() >= 1000
			)
		{
			DurT avg = dur_sum / count;
			cout<<name<<" stats. "
				<<" min:"<<dur_min.count()/1e3<<"ms"
				<<" avg:"<<avg.count()/1e3<<"ms"
				<<" max:"<<dur_max.count()/1e3<<"ms"
				<<" "<<count<<"/s"<<endl;

			count = 0;
			dur_min = DurT::max();
			dur_max = DurT::min();
			dur_sum = DurT::zero();
			time_start = chrono::high_resolution_clock::now();
		}
		//after(std::forward<decltype(args)>(args)...);

		// does not work if return directly
		return ret.value();
		//auto result = ret.value();
		//return result;
	};
}
