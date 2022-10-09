#pragma once

#include <JuceHeader.h>
#include <float.h>

class BitModulation
{
public:
    enum Operation
    {
        NONE,
        XOR,
        AND,
        OR
    };
    
    using OperationFunc = std::function<float(float /*a*/, float /*b*/)>;
	static OperationFunc getOpFunc(Operation operation)
	{
		if (operation == Operation::XOR)
			return [](float a, float b) { return fp_xor(a, b); };

		if (operation == Operation::AND)
			return [](float a, float b) { return fp_and(a, b); };

		if (operation == Operation::OR)
			return [](float a, float b) { return fp_or(a, b); };
        
        return [](float a, float) { return a; };
	}

private:
	static inline std::pair<long long, int> ifrexp(float d)
	{
		int exp;
		float mant = std::frexpf(d, &exp);
		return std::make_pair(static_cast<long long>((mant * std::powf(2.0f, FLT_MANT_DIG))), static_cast<int>(exp));
	}

	static inline float and_(float a, float b)
	{
		auto pa = ifrexp(a);
		auto ma = pa.first;
		auto ea = pa.second;
		auto pb = ifrexp(b);
		auto mb = pb.first;
		auto eb = pb.second;

		mb = mb >> (ea - eb);

		if (ma < 0)
			return (mb & ~(-ma)) * std::powf(2.0f, ea - FLT_MANT_DIG);

		if (mb < 0)
			return (~(-mb) & ma) * std::powf(2.0f, ea - FLT_MANT_DIG);

		return (mb & ma) * std::powf(2.0f, ea - FLT_MANT_DIG);
	}

	static inline float or_(float a, float b)
	{
		auto pa = ifrexp(a);
		auto ma = pa.first;
		auto ea = pa.second;
		auto pb = ifrexp(b);
		auto mb = pb.first;
		auto eb = pb.second;

		mb = mb >> (ea - eb);

		if (ma < 0)
			return (-(~(mb | ~(-ma)))) * std::powf(2.0f, ea - FLT_MANT_DIG);

		if (mb < 0)
			return (-(~(~(-mb) | ma))) * std::powf(2.0f, ea - FLT_MANT_DIG);

		return (mb | ma) * std::powf(2.0f, ea - FLT_MANT_DIG);
	}

	static inline float xor_(float a, float b)
	{
		auto pa = ifrexp(a);
		auto ma = pa.first;
		auto ea = pa.second;
		auto pb = ifrexp(b);
		auto mb = pb.first;
		auto eb = pb.second;

		mb = mb >> (ea - eb);

		return (mb ^ ma) * std::powf(2.0f, ea - FLT_MANT_DIG);
	}

	static inline bool isPositive(float a)
	{
		return std::copysignf(1.0f, a) == 1.0f;
	}

	static inline bool absGeq(float a, float b)
	{
		return std::fabs(a) >= std::fabs(b);
	}

	static float fp_xor(float a, float b)
	{
		if (a == 0.0f)
		{
			if (isPositive(a))
				return b;

			return -b;
		}
		if (b == 0.0f)
			return fp_xor(b, a);

		if (a < 0.0f)
		{
			if (b < 0.0f)
				return fp_xor(-a, -b);

			return -fp_xor(-a, b);
		}
		if (b < 0.0f)
			return -fp_xor(a, -b);

		if (absGeq(a, b))
			return xor_(a, b);

		return xor_(b, a);
	}

	static float fp_and(float a, float b)
	{
		if (a == 0.0)
		{
			if (isPositive(a))
				return 0.0f;

			return b;
		}
		if (b == 0.0f)
			return fp_and(b, a);

		if (a < 0.0f && b < 0.0f)
			return -fp_or(-a, -b);

		if (absGeq(a, b))
			return and_(a, b);

		return and_(b, a);
	}

	static float fp_or(float a, float b)
	{
		if (a == 0.0f)
		{
			if (isPositive(a))
				return b;

			return -0.0f;
		}
		if (b == 0.0f)
			return fp_or(b, a);

		if (a < 0.0f && b < 0.0f)
			return -fp_and(-a, -b);

		if (absGeq(a, b))
			return or_(a, b);

		return or_(b, a);
	}
};
