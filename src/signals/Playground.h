#pragma once
#include "Signal.h"
#include "DFT.h"
#include "Util.h"

struct SignalPlayground
{
	void Play(SignalSlot& topSlot, SignalSlot& middleSlot, SignalSlot& bottomSlot);
};

void DFTOnSignal(SignalSlot& topSlot, SignalSlot& middleSlot, SignalSlot& bottomSlot)
{
	//do discrete fourier on signal					
	//RawSignalPtr sineSignalRaw = ToRawSignal({ &SineSignal{ 2.5f, 4.f, 0.f, 2 }, &SineSignal{ 1.5f, 6.5f, 0.f, 2 }, &SineSignal{ 1.5f, 16.5f, 0.f, 2 } });
	RawSignalPtr sineSignalRaw = ToRawSignal({ &SineSignal{ 2.5f, 4.f, 0.f, 2 }, &SineSignal{ 1.5f, 6.5f, 0.f, 2 }, &SineSignal{ 1.5f, 16.5f, 0.f, 2 } });


	RawSignalPtr fCoefficients = DiscreteFT(sineSignalRaw);
	//RawSignalPtr fCoefficients = DiscreteFT2(SineSignal{ 2.5f, 4.f, 0.f, 2 });
	RawSignalPtr amplitudes = GetAmplitudesFromSignals(fCoefficients);


	//todo FIX Inverse FT
	//possible problemes:
	//originals iterate to 2001 this to 2000
	//wny inverse FT is not reconstructing phase ( TODO add code for phase)
	RawSignalPtr reconstructedSignal = InverseFT2(fCoefficients);

	//draw signals at top slot
	topSlot.AddSignal(move(sineSignalRaw));

	//draw signal amplitudes
	middleSlot.AddSignal(move(amplitudes));

	//draw signal amplitudes
	bottomSlot.AddSignal(move(reconstructedSignal));
}

void DFTOnSignalFast(SignalSlot& topSlot, SignalSlot& middleSlot, SignalSlot& bottomSlot)
{
	SineSignal signal1{ 2.5f, 4.f, 0.f, 2 };
	SineSignal signal2{ 1.5f, 6.f, 0.f, 2 };
	SineSignal signal3{ 2.5f, 4.f, 0.f, 2 };


	SignalPtr	signal(new CombinedSignal{ {
			&signal1,
			&signal2,
			&signal3}, 2 });

	RawSignalPtr fCoefficients = DiscreteFT2(*signal.get());
	RawSignalPtr amplitudes = GetAmplitudesFromSignals(fCoefficients);

	RawSignalPtr reconstructedSignal = InverseFT(fCoefficients);

	//draw signals at top slot
	topSlot.AddSignal(move(signal));

	//draw signal amplitudes
	middleSlot.AddSignal(move(amplitudes));

	//draw signal amplitudes
	bottomSlot.AddSignal(move(reconstructedSignal));
}

void SimpleSignalFiltering(SignalSlot& topSlot, SignalSlot& middleSlot, SignalSlot& bottomSlot)
{
	//do discrete fourier on signal					
	RawSignalPtr sineSignalRaw = ToRawSignal({ &SineSignal{ 2.5f, 4.f, 0.f, 5 }, &SineSignal{ 1.5f, 6.5f, 0.f, 5 }, &SineSignal{ 1.5f, 16.5f, 0.f, 5 } });

	unsigned int SignalSize = 3001;

	RawSignalPtr fCoefficients = DiscreteFT(sineSignalRaw);

	RawSignalPtr amplitudes = GetAmplitudesFromSignals(fCoefficients);

	//remove two signals for 16.5 hz and 6 hz we will reconstruct with 4hz signal
	auto GetIndexForFrequency = [&](float frequency)
	{
		//get frequency index location from signal lenght divided by sampling rate
		float index = frequency * (sineSignalRaw->Size() / (float)sineSignalRaw->GetSamplingRate());
		return (unsigned int)index;
	};


	//////remove 4hz signal
	//unsigned int index4hz = GetIndexForFrequency(4.f);
	//fCoefficients->_dataVec[index4hz] = { 0.f, 0.f };
	//fCoefficients->_dataVec[SignalSize - index4hz] = { 0.f, 0.f };
	//amplitudes->_dataVec[index4hz] = { 0.f, 0.f };
	//amplitudes->_dataVec[SignalSize - index4hz] = { 0.f, 0.f };


	//////remove 6.5 hz signal
	//unsigned int index6hz = GetIndexForFrequency(6.5f);
	//fCoefficients->_dataVec[index6hz] = { 0.f, 0.f };
	//amplitudes->_dataVec[index6hz] = { 0.f, 0.f };
	////hack 
	//fCoefficients->_dataVec[20] = { 0.f, 0.f };
	//amplitudes->_dataVec[20] = { 0.f, 0.f };

	//fCoefficients->_dataVec[SignalSize - index6hz] = { 0.f, 0.f };
	//amplitudes->_dataVec[SignalSize - index6hz] = { 0.f, 0.f };
	////hack
	//fCoefficients->_dataVec[SignalSize - index6hz -1] = { 0.f, 0.f };
	//amplitudes->_dataVec[SignalSize - index6hz - 1] = { 0.f, 0.f };



	//todo FIX Inverse FT
	//possible problemes:
	//originals iterate to 2001 this to 2000
	//wny inverse FT is not reconstructing phase ( TODO add code for phase)
	RawSignalPtr reconstructedSignal = InverseFT(fCoefficients);

	//draw signals at top slot
	topSlot.AddSignal(move(sineSignalRaw));

	//draw signal amplitudes
	middleSlot.AddSignal(move(amplitudes));

	//draw signal amplitudes
	bottomSlot.AddSignal(move(reconstructedSignal));
}

void SimpleSignalFilteringFast(SignalSlot& topSlot, SignalSlot& middleSlot, SignalSlot& bottomSlot)
{
	//generate time func

	//TODO remove global sampling rate variable and use time func settings

	unsigned int SignalSize = 2001;
	SineSignal signal1{ 2.5f, 4.f, 0.f, 3 };
	SineSignal signal2{ 1.5f, 6.5f, 0.f, 3 };
	SineSignal signal3{ 1.5f, 16.5f, 0.f, 3 };
	CombinedSignal combinedSignal({ &signal1, &signal2, &signal3 }, 3);
	SignalSize = combinedSignal.GetSize();

	//TODO fix combined signal calculations
	RawSignalPtr fCoefficients = DiscreteFT2(combinedSignal);
	//
	RawSignalPtr amplitudes = GetAmplitudesFromSignals(fCoefficients);

	//remove two signals for 16.5 hz and 6 hz we will reconstruct with 4hz signal
	auto GetIndexForFrequency = [&](float frequency)
	{
		//get frequency index location from signal lenght divided by sampling rate
		float index = frequency * (SignalSize / (float)combinedSignal.GetSamplingRate());
		return (unsigned int)index;
	};

	////remove 16.5hz signal
	//unsigned int index4hz = GetIndexForFrequency(16.5f);
	//fCoefficients->_dataVec[index4hz] = { 0.f, 0.f };
	//fCoefficients->_dataVec[SignalSize - index4hz + 1] = { 0.f, 0.f };

	////remove 6.5 hz signal
	//unsigned int index6hz = GetIndexForFrequency(6.5f);
	//fCoefficients->_dataVec[index6hz] = { 0.f, 0.f };
	//fCoefficients->_dataVec[SignalSize - index6hz + 1] = { 0.f, 0.f };


	//todo FIX Inverse FT
	//possible problemes:
	//originals iterate to 2001 this to 2000
	//wny inverse FT is not reconstructing phase ( TODO add code for phase)
	RawSignalPtr reconstructedSignal = InverseFT2(fCoefficients);

	//draw signals at top slot
	topSlot.AddSignal(ToRawSignal({ &combinedSignal }));

	//draw signal amplitudes
	middleSlot.AddSignal(move(amplitudes));

	//draw signal amplitudes
	bottomSlot.AddSignal(move(reconstructedSignal));
}

void FastFTExample(SignalSlot& topSlot, SignalSlot& middleSlot, SignalSlot& bottomSlot)
{
	//Fast FT works well with samples that are power of two
	SineSignal signal1{ 2.5f, 4.f, 0.f, 3 };
	SineSignal signal2{ 1.5f, 6.f, 0.f, 3 };
	SineSignal signal3{ 1.5f, 16.f, 0.f, 3 };
	CombinedSignal combinedSignal({ &signal1, &signal2, &signal3  }, 3);
	unsigned int SignalSize = combinedSignal.GetSize();

	RawSignalPtr fCoefficientsFast = FastFT(ToRawSignal({ &combinedSignal }));

	//remove two signals for 16.5 hz and 6 hz we will reconstruct with 4hz signal
	auto GetIndexForFrequency = [&](float frequency)
	{
		//get frequency index location from signal lenght divided by sampling rate
		float index = frequency * (SignalSize / (float)combinedSignal.GetSamplingRate());
		return (unsigned int)index;
	};

	RawSignalPtr amplitudes = GetAmplitudesFromSignals(fCoefficientsFast);

	//remove 16.hz signal
	RemoveSignalFrequency(fCoefficientsFast, 16.f, combinedSignal.GetLenght());
	
	//remove 4 hz signal
	RemoveSignalFrequency(fCoefficientsFast, 4.f, combinedSignal.GetLenght());

	RawSignalPtr reconstructedSignal = InverseFT2(fCoefficientsFast);

	//draw signals at top slot
	topSlot.AddSignal(ToRawSignal({ &combinedSignal }));

	//draw signal amplitudes
	middleSlot.AddSignal(move(amplitudes));

	//draw signal amplitudes
	bottomSlot.AddSignal(move(reconstructedSignal));
}

void SignalPlayground::Play(SignalSlot& topSlot, SignalSlot& middleSlot, SignalSlot& bottomSlot)
{
	//TODO: implement FFInverse

	////DFTOnSignalFast(topSlot, middleSlot, bottomSlot);
	//{
	//	MeasureExecution<> measure("SimpleSignalFiltering");
	//	SimpleSignalFiltering(topSlot, middleSlot, bottomSlot);
	//}

	//{
	//	MeasureExecution<> measure("SimpleSignalFilteringFast");
	//	SimpleSignalFilteringFast(topSlot, middleSlot, bottomSlot);

	//}

	{
		MeasureExecution<> measure("FastFT signal processing");
		FastFTExample(topSlot, middleSlot, bottomSlot);
	}
	
}
