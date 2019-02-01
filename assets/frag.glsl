#version 120

#define AMBIENT_OCCLUSION_COLOR_DELTA vec3(0.7)
#define AMBIENT_OCCLUSION_STRENGTH 0.008
#define ANTIALIASING_SAMPLES 1
#define BACKGROUND_COLOR vec3(0.6,0.8,1.0)
#define COL col_scene
#define DE de_scene
#define DIFFUSE_ENABLED 0
#define DIFFUSE_ENHANCED_ENABLED 1
#define FOCAL_DIST 1.73205080757
#define FOG_ENABLED 0
#define LIGHT_COLOR vec3(1.0,0.95,0.8)
#define LIGHT_DIRECTION vec3(-0.36, 0.8, 0.48)
#define MAX_DIST 3000.0
#define MAX_MARCHES 1000
#define MIN_DIST 1e-5
#define PI 3.14159265358979
#define SHADOWS_ENABLED 1
#define SHADOW_DARKNESS 0.7
#define SHADOW_SHARPNESS 10.0
#define SPECULAR_HIGHLIGHT 40
#define SPECULAR_MULT 0.25
#define SUN_ENABLED 1
#define SUN_SHARPNESS 2.0
#define SUN_SIZE 0.004
#define VIGNETTE_STRENGTH 0.5

uniform mat4 iMat;
uniform vec2 iResolution;
uniform float iTime;

uniform float iDebugValues[10];

float log10( in float n ) {
    const float kLogBase10 = 1.0 / log2( 10.0 );
    return log2( n ) * kLogBase10;
}

float rand(vec2 co)
{
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float de_sphere(vec3 p, float radius)
{
    return length(p) - radius;
}

float de_box(vec3 p, vec3 b)
{
    vec3 d = abs(p) - b;
    return length(max(d, 0.0)) + min(max(d.x, max(d.y, d.z)), 0.0);
}

float de_hplane(vec3 p, float y)
{
    return p.y + y;
}

float de_plane(vec3 p, vec4 n)
{
    return dot(p, n.xyz) - n.w;
}

float sdCylinder( vec3 p, vec3 c )
{
  return length(p.xy-c.xz)-c.z;
}

float ri_plane( in vec3 ro, in vec3 rd, in vec4 p )
{
    return -(dot(ro,p.xyz)+p.w)/dot(rd,p.xyz);
}

float de_scene(in vec3 p)
{
    float plane = de_hplane(p, 1.0);
    p.x = mod(p.x, 7) - 3.5;
    //p.y = mod(p.y, 5) - 2.5;
    p.z = mod(p.z, 7) - 3.5;
    float cylinder = sdCylinder(p, vec3(0.0, 0.0, 0.7));
    float s1 = de_sphere(p, 1.0);
    float s2 = de_box(p, vec3(0.75)) - 0.1;

    return plane * iDebugValues[2] + (1-iDebugValues[2]) * min(max(max(s1, s2), -cylinder), plane);
}

vec3 nv_scene(in vec3 p)
{
    const vec3 small_step = vec3(0.001, 0.0, 0.0);

    float gradient_x = de_scene(p + small_step.xyy) - de_scene(p - small_step.xyy);
    float gradient_y = de_scene(p + small_step.yxy) - de_scene(p - small_step.yxy);
    float gradient_z = de_scene(p + small_step.yyx) - de_scene(p - small_step.yyx);

    vec3 normal = vec3(gradient_x, gradient_y, gradient_z);

    return normalize(normal);
}

float raymarch(in vec3 ro, in vec3 rd, out int stepcount)
{
    float total_distance_traveled = 0.0;
    for (int i = 0; i < MAX_MARCHES; ++i)
    {
        stepcount = i;
        vec3 current_position = ro + total_distance_traveled * rd;

        float d = de_scene(current_position);

        if (d < MIN_DIST)
            return total_distance_traveled;

        if (total_distance_traveled > MAX_DIST)
            break;

        total_distance_traveled += d;
    }
    return min(total_distance_traveled, MAX_DIST);
}

vec3 applyFog(in vec3 col, in float dist, in vec3 rayDir, in vec3 sunDir)
{
    float fogAmount = 1.0 - exp(-dist);
    float sunAmount = max(dot(rayDir, sunDir), 0.0);
    vec3 fogColor = mix( vec3(0.5, 0.6, 0.7),
                         vec3(1.0, 0.9, 0.7),
                         pow(sunAmount, 8.0));
    fogColor *= pow(max(sunDir.y, 0.1), 2.0);
    return mix(col, fogColor, fogAmount);
}

vec4 render_scene(in vec2 uv)
{
    vec4 col;
    vec3 lpos = vec3(cos(iTime * 0.2), sin(iTime* 0.2), 0.0);
    vec3 ldir = normalize(lpos);
    vec3 camera_position = vec3(0.0, 0.0, -5.0);
    vec3 ro = iMat[3].xyz;
    vec3 rd = normalize((iMat * vec4(uv, FOCAL_DIST * (1+iDebugValues[1]), 0.0)).xyz);

    int stepcount;
    float d = raymarch(ro, rd, stepcount);
    float dplane = ri_plane(ro, rd, vec4(0.0, 1.0, 0.0, -iDebugValues[0] + 1.01));
    vec4 debug_color = vec4(0.0);
    if (dplane > 0.0 && dplane < d)
    {
        float ddplane = de_scene(ro + rd * dplane);
        ddplane = min(mod(100*(ddplane/pow(10,ceil(log10(ddplane)))), 10), 1.0);
        debug_color = vec4(vec3(ddplane), 0.5);
    }
    vec3 p = ro + rd * d;
    if (d < 0.0 || d >= MAX_DIST)
    {
        col = vec4(0.01);//vec4(0.30, 0.36, 0.60, 1.0) - (rd.y * 0.7);
    }
    else
    {
        vec3 color = vec3(0.4, 0.8, 0.1);
        vec3 ambient = vec3(0.02, 0.021, 0.02) * 0.1;

        // diffuse
        vec3 normal = nv_scene(p);
        float diffuse_intensity = max(0.0, dot(normal, ldir));

        // shadow
        vec3 srd;
        vec3 sro = p + normal*0.001;
        float shadow = 0.0;
        const float shadowRayCount = 4;
        srd = normalize(lpos);
        for (int x = 0; x < shadowRayCount; ++x)
        {
            float r = rand(vec2(srd.xy) + x) * 2.0 - 1.0;
            int shadowstepcount;
            float sd = raymarch(sro, srd + r * 0.001, shadowstepcount);
            if (sd < MAX_DIST)//(sd > 0. && sd < distance(lpos, p))
            {
                shadow += 1.0;
            }
        }
        shadow = 1 - shadow/shadowRayCount;

        col = vec4(color * (diffuse_intensity * shadow + ambient), 1.0);
    }
    // fog
    col.xyz = applyFog(col.xyz, d*0.0006, rd, ldir*1.4);
    // ambient occlusion
    col.xyz *= (1.0-vec3(stepcount/1000));

    return vec4(debug_color.xyz * debug_color.w, 1.0) + (1.0 - debug_color.w) * col;
}

void main()
{
    vec2 screen_pos = (gl_FragCoord.xy) / iResolution.xy;
    vec2 uv = 2*screen_pos - 1;
    uv.x *= iResolution.x / iResolution.y;

    vec4 col = render_scene(uv);//vec4(0.5, 0.25, 0.0, 1.0);
    col = pow(col, vec4(vec3(0.4545), 1.0)); // Gamma correction (1.0 / 2.2)
    col.w = 0.9;

    gl_FragColor = smoothstep(0.0, 1.0, col);
}

