#pragma once

#include <limits>
#include <math.h>
#include <initializer_list>
#include "Complex.h"

static const float PI = 3.14159265f;
static const unsigned int SamplingRate = 1000;

struct StandartTimeFunc
{

	mutable unsigned int _size{ 0 };
	unsigned int SamplingRate = 1000;
	unsigned int NumSeconds = 1;

	StandartTimeFunc() {};

	StandartTimeFunc(unsigned int SamplingRate, unsigned int NumSeconds) :
		SamplingRate(SamplingRate),
		NumSeconds(NumSeconds)
	{
	}

	virtual unsigned int GetSize() const
	{
		//size is not calculated precompute 
		if (_size == 0)
		{
			//round to power of two 
			unsigned int sizeNeeded = SamplingRate * NumSeconds + 1;

			auto sizeNeededMagnitudeOfTwo = log2(sizeNeeded);
			auto magnitudeOfTwo = std::ceil(sizeNeededMagnitudeOfTwo);
			_size = static_cast<unsigned int>(std::pow(2, magnitudeOfTwo));
		}

		return _size;
	}

	virtual float GetTime(unsigned int index) const
	{
		float timeStep = (float)index / SamplingRate;

		return timeStep;
	}
};

struct FourierTimeFunc :public StandartTimeFunc
{
	FourierTimeFunc() {}
	FourierTimeFunc(unsigned int SamplingRate, unsigned int NumSeconds) :
		StandartTimeFunc(SamplingRate, NumSeconds)
	{
	}

	float GetTime(unsigned int index)const override
	{
		return (float)index / GetSize();
	}
};


struct Signal
{
protected:
	unsigned int _signalLenght{ 1 };//lenght in seconds
	std::unique_ptr<StandartTimeFunc> _timeFunction{ new StandartTimeFunc() };

	Signal(unsigned int signalLenght) :
		_signalLenght(signalLenght)
	{
		_timeFunction->NumSeconds = signalLenght;
	}
public:
	unsigned int GetLenght() const {
		return _signalLenght;
	}

	unsigned int GetSize() const
	{
		return _timeFunction->GetSize();
	}

	unsigned int GetSamplingRate() const
	{
		return _timeFunction->SamplingRate;
	}

	float GetTime(unsigned int index) const
	{
		return _timeFunction->GetTime(index);
	}

	virtual float Evaluate(float time) const = 0;
	virtual float Evaluate2(float time) const { return 0.f; }

	virtual float Evaluate(unsigned int index) const
	{
		float time = GetTime(index);
		return Evaluate(time);
	}

	virtual float Evaluate2(unsigned int index) const
	{
		float time = GetTime(index);
		return Evaluate2(time);
	}
};

struct RawSignal : public Signal
{
	//data time and function in one slot
	std::vector<float> _timeVec;
	std::vector<std::pair<float, float>> _dataVec;

	RawSignal() :RawSignal(0) {}

	RawSignal(unsigned int size) :Signal(size / SamplingRate)
	{
		_timeVec.resize(size);
		_dataVec.resize(size);
	}

	unsigned int Size()
	{
		return static_cast<unsigned int>(_timeVec.size());
	}

	float GetTime(unsigned int index) const
	{
		return _timeVec[index];
	}

	float Evaluate(float time) const override
	{
		return 0.f;
	}

	float Evaluate2(float time) const override
	{
		return 0.f;
	}

	virtual float Evaluate(unsigned int index) const
	{
		return _dataVec[index].first;
	}

	virtual float Evaluate2(unsigned int index) const
	{
		return _dataVec[index].second;
	}

};

using RawSignalPtr = std::unique_ptr<RawSignal>;
using SignalPtr = std::unique_ptr<Signal>;

struct CombinedSignal :public Signal
{
	std::vector<Signal*> _signals;

	CombinedSignal(std::initializer_list<Signal*> signals, unsigned int signalLenght) :Signal(signalLenght)
	{
		_signals.assign(signals);
	}

	/*float GetTime(unsigned int index) const override
	{
		if (_signals.size() == 0)
		{
			return 0.f;
		}
		return _signals.front()->GetTime(index);
	}*/

	float Evaluate(float time) const override
	{
		float result = 0.f;
		for (auto signal : _signals)
		{
			result += signal->Evaluate(time);
		}
		return result;
	}

	float Evaluate2(float time) const override
	{
		float result = 0.f;
		for (auto signal : _signals)
		{
			result += signal->Evaluate2(time);
		}
		return result;
	}
};

class SineSignal :public Signal
{
public:
	SineSignal(float amplitude, float frequency, float phase, unsigned int lenght) : Signal(lenght),
		_amlitiude(amplitude),
		_frequency(frequency),
		_phase(phase) {}

	//float GetTime(unsigned int index) const
	//{
	//	float timeStep = (float)1 / SamplingRate;

	//	float time = timeStep * index;

	//	return time;
	//}

	float Evaluate(float time) const  override
	{
		if (time > _signalLenght)
		{
			return 0.f;
		}
		return _amlitiude * sin(2.f * PI * _frequency * time + _phase);
	}

private:
	float _amlitiude{ 2.f };
	float _frequency{ 10.f }; //10 hz
	float _phase{ 0.f };
};

class ComplexSineSignal : public Signal
{
public:
	ComplexSineSignal(float amplitude, float frequency, float phase, unsigned int lenght) : Signal(lenght),
		_amlitiude(amplitude),
		_frequency(frequency),
		_phase(phase)
	{
		/*unsigned int seconds = _timeFunction.NumSeconds;
		unsigned int samplRate = _timeFunction.SamplingRate;*/

		_timeFunction = std::unique_ptr<FourierTimeFunc>(new FourierTimeFunc(_timeFunction->SamplingRate, lenght));
	}

	float Evaluate(float time) const override
	{
		//real part amlitude is always positive 
		return abs(_amlitiude) * cos(2 * PI * _frequency * time + _phase);
	}

	float Evaluate2(float time) const override
	{
		//imaginary part amplitude might be negative
		return _amlitiude * sin(2 * PI * _frequency * time + _phase);
	}

	std::unique_ptr<RawSignal> ToRawSignal()
	{
		unsigned int numItems = _signalLenght * this->_timeFunction->SamplingRate + 1;
		float timeStep = (float)_signalLenght / numItems;

		std::unique_ptr < RawSignal> result(new RawSignal());
		result->_timeVec.reserve(numItems);
		result->_dataVec.reserve(numItems);

		for (unsigned int i = 0; i < numItems; ++i)
		{
			float time = (float)i / (numItems);
			float value = Evaluate(time);
			float value2 = Evaluate2(time);

			result->_timeVec.push_back(time);
			result->_dataVec.push_back({ value, value2 });
		}

		return result;
	}

	std::unique_ptr<RawSignal> ToRawSignalAndMultiply(std::pair<float, float> multiplier = { 1.f, 1.f })
	{
		auto z = std::numeric_limits<unsigned int>::max;

		FourierTimeFunc fTF{ _timeFunction->SamplingRate, _signalLenght };

		unsigned int numItems = fTF.GetSize();

		float timeStep = (float)_signalLenght / numItems;

		std::unique_ptr < RawSignal> result(new RawSignal(numItems));
		for (unsigned int i = 0; i < numItems; ++i)
		{
			//float time = (float)i / (numItems);
			float time = fTF.GetTime(i);

			float value = Evaluate(time);
			float value2 = Evaluate2(time);

			result->_timeVec[i] = time;

			result->_dataVec[i] = ComplexMultiply({ value, value2 }, multiplier);
		}

		return result;
	}


private:
	float _amlitiude{ 2.f };
	float _frequency{ 10.f }; //10 hz
	float _phase{ 0.f };
};

struct SignalSlot
{
	std::pair<float, float> RelativePosition{ 0.f, 5.f };
	std::vector < SignalPtr > _signals;
	unsigned int _signalLenght{ 0 };

	void AddSignal(SignalPtr&& signal)
	{
		_signalLenght = max(signal->GetLenght() * signal->GetSamplingRate(), _signalLenght);
		_signals.push_back(move(signal));
	}

	unsigned int size()
	{
		return _signalLenght;
	}

	float evaluate(unsigned int index)
	{
		float val = 0.f;
		for (const auto& signal : _signals)
		{
			if (signal->GetSize() > index)
			{
				val += signal->Evaluate(index);
			}

		}
		return val;
	}

	float evaluate2(unsigned int index)
	{
		float val = 0.f;
		for (const auto& signal : _signals)
		{
			if (signal->GetSize() > index)
			{
				val += signal->Evaluate2(index);
			}

		}
		return val;
	}
};
//TODO implement combine signals with variadic templates
inline RawSignalPtr ToRawSignal(std::initializer_list<const Signal*> signals)
{
	unsigned int signalLenghtSeconds = 0;
	unsigned int numItems = 0;
	for (const auto& signal : signals)
	{
		signalLenghtSeconds = max(signal->GetLenght(), signalLenghtSeconds);
		numItems = max((unsigned int)signal->GetSize(), numItems);
	}

	RawSignalPtr result(new RawSignal(numItems));
	float timeStep = (float)signalLenghtSeconds / numItems;

	for (unsigned int index = 0; index < numItems; ++index)
	{
		float y = 0.f;
		float z = 0.f;

		float time = timeStep * index;
		result->_timeVec[index] = time;

		for (const Signal* signal : signals)
		{
			y += signal->Evaluate(time);
			z += signal->Evaluate2(time);

			result->_dataVec[index] = { y,z };
		}
	}
	return result;
}


//SignalPtr DecorateSignal(SignalPtr& signal, [)