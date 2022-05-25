#pragma once

struct Smooth
{
    static float flip(float x)
    {
        return 1 - x;
    }

    static float dumbPow(float x, uint32_t p)
    {
        float res = 1.0f;
        for (uint32_t i(p); i--;) {
            res *= x;
        }
        return res;
    }

    static float smoothStop(float t, uint32_t power)
    {
        return flip(dumbPow(flip(t), power));
    }
};
