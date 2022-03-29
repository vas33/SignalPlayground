#pragma once
#include "Signal.h"
#include "Util.h"

std::pair<float, float>RawSignalDot(const RawSignalPtr& signal1, RawSignalPtr& signal2)
{

	MeasureExecution<std::chrono::microseconds>  execution("RawSignalDot");
	std::pair<float, float> result{ 0.f, 0.f };

	for (unsigned int index = 0; index < signal1->Size(); ++index)
	{
		float vall1Real = signal1->Evaluate(index);
		float val1Img = signal1->Evaluate2(index);

		float vall2Real = signal2->Evaluate(index);
		float val2Img = signal2->Evaluate2(index);

		result.first += vall2Real * vall1Real;
		result.second += val2Img * vall1Real;
	}

	//result.first = result.first / 2.f;
	//result.second = result.second / 2.f; 

	result.first = result.first / signal1->Size();
	result.second = result.second / signal1->Size();

	return result;
}

std::pair<float, float> RawSignalDot2(const Signal& signal1, const Signal& signal2)
{
	MeasureExecution<std::chrono::microseconds>  execution("RawSignalDot2");
	std::pair<float, float> result{ 0.f, 0.f };

	auto signalSize = signal1.GetSize();

	for (unsigned int index = 0; index < signalSize; ++index)
	{
		float time1 = signal1.GetTime(index);
		float vall1Real = signal1.Evaluate(time1);
		float val1Img = signal1.Evaluate2(time1);

		float time2 = signal2.GetTime(index);
		float vall2Real = signal2.Evaluate(time2);
		float val2Img = signal2.Evaluate2(time2);

		result.first += vall2Real * vall1Real;
		result.second += val2Img * vall1Real;
	}

	result.first = result.first / signalSize;
	result.second = result.second / signalSize;

	return result;
}

RawSignalPtr AddSignals(const RawSignalPtr& signal1, RawSignalPtr& signal2)
{
	if (signal1->Size() == signal2->Size())
	{

	}

	RawSignalPtr result(new RawSignal(signal1->Size()));

	for (unsigned int index = 0; index < signal1->Size(); ++index)
	{
		result->_timeVec[index] = signal1->_timeVec[index];
		result->_dataVec[index].first = signal1->_dataVec[index].first + signal2->_dataVec[index].first;
		result->_dataVec[index].second = signal1->_dataVec[index].second + signal2->_dataVec[index].second;
	}

	return result;
}

void SignalDivide(RawSignalPtr& signal, float divisor)
{
	for (unsigned int index = 0; index < signal->Size(); ++index)
	{
		signal->_dataVec[index].first /= divisor;
		signal->_dataVec[index].second /= divisor;
	}
}

RawSignalPtr DiscreteFT(const RawSignalPtr& signal)
{
	MeasureExecution<>  execution("DiscreteFT");

	unsigned int signalSize = signal->Size();

	//create raw Signal
	RawSignalPtr result(new RawSignal(signalSize));

	for (unsigned int index = 0; index < signalSize; ++index)
	{
		//create complex sine wave
		float frequency = (float)(index);
		auto& csw = ComplexSineSignal(-1.f, frequency, 0.f, signal->GetLenght()).ToRawSignal();

		result->_timeVec[index] = signal->_timeVec[index];
		//compute dot product
		result->_dataVec[index] = RawSignalDot(signal, csw);
	}

	return move(result);
}


std::vector<std::pair<float, float>> FastFTImpl(const std::vector<std::pair<float, float>>& signal)
{
	unsigned int numSamples = (unsigned int)signal.size();

	if (numSamples == 1)
	{
		return signal;
	}

	//half of samples Numer
	unsigned int halfSamples = static_cast<unsigned int>(signal.size()) / 2;

	//declare event and odd halves
	std::vector < std::pair<float, float> > Even(halfSamples, { 0.f, 0.f });
	std::vector < std::pair<float, float> > Odd(halfSamples, { 0.f, 0.f });

	//split to even and odd
	for (unsigned int index = 0; index < halfSamples; ++index)
	{
		Even[index] = signal[2 * index];
		Odd[index] = signal[2 * index + 1];
	}

	std::vector<std::pair<float, float>> fEven = FastFTImpl(Even);
	std::vector<std::pair<float, float>> fOdd = FastFTImpl(Odd);

	std::vector<std::pair<float, float>> frequbins(numSamples, { 0.f, 0.f });

	for (int k = 0; k != halfSamples; ++k)
	{
		Complex complexExponential = ComplexMultiply(
			CompolexPolar(1.f, -2.f * PI * k / numSamples), fOdd[k]);

		frequbins[k] = ComplexSum(fEven[k], complexExponential);
		frequbins[k + halfSamples] = ComplexNegation(fEven[k], complexExponential);

	}

	return frequbins;
}

RawSignalPtr FastFT(const RawSignalPtr& signal)
{
	MeasureExecution<>  execution("FastFT");

	unsigned int signalSize = signal->Size();

	RawSignalPtr result(new RawSignal(signalSize));

	result->_timeVec = signal->_timeVec;
	result->_dataVec = FastFTImpl(signal->_dataVec);

	for (unsigned int i = 0; i < signalSize; ++i)
	{
		result->_dataVec[i].first /= (float)signalSize;
		result->_dataVec[i].second /= (float)signalSize;
	}

	return move(result);
}

//faster discrete FT works with compiled signals to skip memory allocation
RawSignalPtr DiscreteFT2(const Signal& signal)
{
	MeasureExecution<> execution("DiscreteFT2");
	unsigned int signalSize = signal.GetSize();

	//create raw Signal
	RawSignalPtr result(new RawSignal(signalSize));

	for (unsigned int index = 0; index < signalSize; ++index)
	{
		//create complex sine wave
		float frequency = (float)(index);
		//TODO see if time scale is different for To raw signal than to evaluate
		//TODO time normalization for ComplexSineSignal
		auto& csw = ComplexSineSignal(-1.f, frequency, 0.f, signal.GetLenght());

		result->_timeVec[index] = signal.GetTime(index);
		//compute dot product
		result->_dataVec[index] = RawSignalDot2(signal, csw);
	}

	return move(result);
}

RawSignalPtr GetAmplitudesFromSignals(const RawSignalPtr& signal)
{
	//create raw Signal
	RawSignalPtr result(new RawSignal(signal->Size()));

	for (unsigned int index = 0; index < signal->Size(); ++index)
	{
		result->_timeVec[index] = signal->_timeVec[index];

		//compute complex vector magnitude on complex number and by two
		result->_dataVec[index].first = 2.f * ComplexMagnitude(signal->_dataVec[index]);
	}

	return move(result);
}

RawSignalPtr InverseFT(const RawSignalPtr& fCoeeficients)
{
	MeasureExecution<> execution("InverseFT");
	unsigned int signalSize = fCoeeficients->Size();
	unsigned int samplingRate = fCoeeficients->GetSamplingRate();

	RawSignalPtr reconstructedSignal(new RawSignal(signalSize));

	for (unsigned int index = 0; index < signalSize; ++index)
	{
		//create complex sine wave
		float frequency = (float)(index);

		//get amplitude from fourier coeeficients
		auto fCoefficient = fCoeeficients->_dataVec[index];

		auto csw = ComplexSineSignal(1.f, frequency, 0.f, signalSize / samplingRate).ToRawSignalAndMultiply(fCoefficient);

		//sum signals	
		reconstructedSignal = AddSignals(reconstructedSignal, csw);
	}

	//SignalDivide(reconstructedSignal, signalSize);

	return reconstructedSignal;
}

RawSignalPtr InverseFT2(const RawSignalPtr& fCoeeficients)
{
	MeasureExecution<>  execution("InverseFT2");
	unsigned int signalSize = fCoeeficients->Size();
	unsigned int signalLenghtSeconds = fCoeeficients->GetLenght();

	RawSignalPtr reconstructedSignal(new RawSignal(signalSize));

	for (unsigned int index = 0; index < signalSize; ++index)
	{
		//create complex sine wave
		float frequency = (float)(index);

		//get amplitude from fourier coeeficients
		auto fCoefficient = fCoeeficients->_dataVec[index];

		Signal& csw = ComplexSineSignal(1.f, frequency, 0.f, signalLenghtSeconds);// .ToRawSignalAndMultiply(fCoefficient);

		//multiply complex sine wave by fourier coefficient and add to signal
		for (unsigned int index = 0; index < signalSize; ++index)
		{
			std::pair<float, float> cswValue{ csw.Evaluate(index), csw.Evaluate2(index) };
			auto cswMultiplied = ComplexMultiply(cswValue, fCoefficient);

			reconstructedSignal->_dataVec[index].first += cswMultiplied.first;
			reconstructedSignal->_dataVec[index].second += cswMultiplied.second;
		}
	}

	//SignalDivide(reconstructedSignal, signalSize);

	return reconstructedSignal;
}


std::pair<unsigned int, unsigned int> GetSignalIndexStart(float frequency, RawSignalPtr& signal, unsigned int SignalLenght)
{
	float index = frequency * SignalLenght;
	return { static_cast<unsigned int>(floor(index)) , static_cast<unsigned int>(ceil(index)) };
}

std::pair<unsigned int, unsigned int>GetSignalIndexEnd(float frequency, RawSignalPtr& signal, unsigned int SignalLenght)
{
	float index = signal->GetSize() - frequency * SignalLenght;
	return { static_cast<unsigned int>(floor(index)) , static_cast<unsigned int>(ceil(index)) };
}

void RemoveSignalFrequency(RawSignalPtr& signal, float frequency, unsigned int SignalLenght)
{
	//GetStartSignal index
	auto startIndex = GetSignalIndexStart(frequency, signal, SignalLenght);
	auto endIndex = GetSignalIndexEnd(frequency, signal, SignalLenght);

	auto RemoveSignal = [](RawSignalPtr& signal, std::pair<unsigned int, unsigned int> indices)
	{
		//expand indices 
		unsigned int boundFront = indices.first - 1;
		unsigned int boundEnd = indices.second + 1;

		//calculate median
		std::pair<float, float> median;

		auto valStart = signal->_dataVec[boundFront].first;
		auto valEnd = signal->_dataVec[boundEnd].first;
		median.first = (valStart + valEnd) / 2;
		median.first = 0.f;

		valStart = signal->_dataVec[boundFront].second;
		valEnd = signal->_dataVec[boundEnd].second;
		median.second = (valStart + valEnd) / 2;
		median.second = 0.f;

		//set median
		signal->_dataVec[indices.first] = median;
		signal->_dataVec[indices.second] = median;
	};

	RemoveSignal(signal, startIndex);
	RemoveSignal(signal, endIndex);
}