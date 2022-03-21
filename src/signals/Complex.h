#pragma once
#include <utility>

using Complex = std::pair<float, float>;

inline Complex ComplexSum(const Complex& numberOne, const Complex& numberTwo)
{
	return { numberOne.first + numberTwo.first, numberOne.second + numberTwo.second };
}
inline Complex ComplexNegation(const Complex& numberOne, const Complex& numberTwo)
{
	return { numberOne.first - numberTwo.first, numberOne.second - numberTwo.second };
}


inline float ComplexMagnitude(const Complex& numberOne)
{
	return sqrt(numberOne.first * numberOne.first + numberOne.second* numberOne.second);
}

inline Complex ComplexMultiply(const Complex& numberOne, const Complex& numberTwo)
{
	Complex result;

	//how complex multiplation is handled
	//(a + ai) * (b + bi) = a * b + a * bi + ai * b + ai * bi
	//and continued...    ai * bi = ab * i *i 
	// i*i = -1
	// so:   ab * i *i  = - ab
	//result:    (a*b - (ai * bi), a * bi + ai * b)
	result.first = numberOne.first * numberTwo.first - (numberOne.second * numberTwo.second);
	result.second = numberOne.first * numberTwo.second + numberOne.second * numberTwo.first;

	return result;
}
