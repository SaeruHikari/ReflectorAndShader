#include "std/std.hpp"
#include "std/functions/math.hpp"

namespace skr::shader {

enum class E : uint {
    A, B, C, D, E, F, G
};

float4 mandelbrot(uint2 tid, uint2 tsize) {
    const float x = float(tid.x) / (float)tsize.x;
    const float y = float(tid.y) / (float)tsize.y;
    const float2 uv = float2(x, y);
    float n = 0.0f;
    float2 c = float2(-0.444999992847442626953125f, 0.0f);
    c = c + (uv - float2(0.5f, 0.5f)) * 2.3399999141693115234375f;
    float2 z = float2(0.f, 0.f);
    const int M = 128;
    for (int i = 0; i < M; i++) {
        z = float2((z.x * z.x) - (z.y * z.y), (2.0f * z.x) * z.y) + c;
        if (dot(z, z) > 2.0f) {
            break;
        }
        n += 1.0f;
    }
    // we use a simple cosine palette to determine color:
    // http://iquilezles.org/www/articles/palettes/palettes.htm
    const float t = float(n) / float(M);
    const float3 d = float3(0.3f, 0.3f, 0.5f);
    const float3 e = float3(-0.2f, -0.3f, -0.5f);
    const float3 f = float3(2.1f, 2.0f, 3.0f);
    const float3 g = float3(0.0f, 0.1f, 0.0f);
    return float4(d + (e * cos(((f * t) + g) * 2.f * pi)), 1.0f);
}


[[kernel_2d(16, 16)]] 
void kernel([[builtin("ThreadID")]] uint2 tid,  Buffer<float4>& output)
{
    const uint2 tsize = uint2(1024, 1024);
    const uint32 row_pitch = tsize.x;
    /*
    auto mandelbrot = [&]() {
        const float x = float(tid.x) / (float)tsize.x;
        const float y = float(tid.y) / (float)tsize.y;
        const float2 uv = float2(x, y);
        float n = 0.0f;
        float2 c = float2(-0.444999992847442626953125f, 0.0f);
        c = c + (uv - float2(0.5f, 0.5f)) * 2.3399999141693115234375f;
        float2 z = float2(0.f, 0.f);
        const int M = 128;
        for (int i = 0; i < M; i++) {
            z = float2((z.x * z.x) - (z.y * z.y), (2.0f * z.x) * z.y) + c;
            if (dot(z, z) > 2.0f) {
                break;
            }
            n += 1.0f;
        }
        // we use a simple cosine palette to determine color:
        // http://iquilezles.org/www/articles/palettes/palettes.htm
        const float t = float(n) / float(M);
        const float3 d = float3(0.3f, 0.3f, 0.5f);
        const float3 e = float3(-0.2f, -0.3f, -0.5f);
        const float3 f = float3(2.1f, 2.0f, 3.0f);
        const float3 g = float3(0.0f, 0.1f, 0.0f);
        return float4(d + (e * cos(((f * t) + g) * 2.f * pi)), 1.0f);
    };
    */
    float4 c = mandelbrot(tid, tsize);
    output.store(tid.x + tid.y * row_pitch, mandelbrot(tid, tsize));
}

} // namespace skr::shader