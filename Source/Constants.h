#pragma once

constexpr int MAX_DELAY_TIME_SEC = 4;

constexpr float MIN_MOD_RATE_HZ = 0.02f;
constexpr float MAX_MOD_RATE_HZ = 10.0f;

constexpr float MIN_GAIN_DB = -60.0f;
constexpr float MAX_GAIN_DB = 0.0f;

constexpr float MIN_NOISE_LEVEL_DB = -60.0f;
constexpr float MAX_NOISE_LEVEL_DB = 0.0f;

constexpr float MIN_BITCRUSHER_Q = 29.8e-9f;
constexpr float MAX_BITCRUSHER_Q = 2.0f;

constexpr float MIN_DECIMATOR_RATIO = 0.0001f;
constexpr float MAX_DECIMATOR_RATIO = 1.0f;

constexpr float MIN_LPF_CUTOFF_FREQ = 20.0f;
constexpr float MAX_LPF_CUTOFF_FREQ = 20480.0f;

constexpr float MIN_LPF_Q = 0.707f;
constexpr float MAX_LPF_Q = 20.0f;

constexpr int NUM_CHANNELS = 2;
