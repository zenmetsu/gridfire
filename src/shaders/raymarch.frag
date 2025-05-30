#version 450
layout(location = 0) in vec2 fragCoord;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 camPos;
    float time; // Added time uniform for animation
} ubo;

float sphereSDF(vec3 p, vec3 center, float radius) {
    return length(p - center) - radius;
}

float cubeSDF(vec3 p, vec3 center, float size) {
    vec3 d = abs(p - center) - vec3(size * 0.5);
    return length(max(d, 0.0)) + min(max(d.x, max(d.y, d.z)), 0.0);
}

float gridSDF(vec3 p) {
    float gridSpacing = 8.0;
    float lineThickness = 0.04;
    vec3 q = mod(p, gridSpacing) - 0.5 * gridSpacing;
    float dx = min(length(vec2(q.y, q.z)), length(vec2(q.y, q.z - gridSpacing)));
    float dy = min(length(vec2(q.x, q.z)), length(vec2(q.x, q.z - gridSpacing)));
    float dz = min(length(vec2(q.x, q.y)), length(vec2(q.x, q.y - gridSpacing)));
    return min(min(dx, dy), dz) - lineThickness;
}

// Smooth minimum function for blending SDFs
float smin(float a, float b, float k) {
    float h = clamp(0.5 + 0.5 * (b - a) / k, 0.0, 1.0);
    return mix(b, a, h) - k * h * (1.0 - h);
}

struct SceneHit {
    float dist;
    vec3 color;
};

SceneHit sceneSDF(vec3 p) {
    // Sphere at origin
    float sphereDist = sphereSDF(p, vec3(0.0, 0.0, 0.0), 1.0);
    vec3 sphereColor = vec3(0.08, 0.6, 0.5); // Black sphere

    // Cube orbital parameters
    float cube_semiMajorAxis = 2.75; // Semi-major axis
    float cube_eccentricity = 0.8182; // Eccentricity
    float cube_inclination = 0.0; // Inclination (radians)
    float cube_longAscNode = 0.0; // Longitude of ascending node (radians)
    float cube_argPeriapsis = 0.0; // Argument of periapsis (radians)
    float cube_period = 6.0; // Orbital period (seconds)

    // Mean anomaly
    float M = 2.0 * 3.14159265359 * ubo.time / cube_period;

    // Solve Kepler's equation for eccentric anomaly (E) using Newton's method
    float E = M;
    const int maxIterations = 10; // Increased for better convergence
    for (int j = 0; j < maxIterations; j++) {
        float delta = (E - cube_eccentricity * sin(E) - M) / (1.0 - cube_eccentricity * cos(E));
        E -= delta;
        if (abs(delta) < 1e-6) break; // Convergence check
    }

    // True anomaly
    float cos_v = (cos(E) - cube_eccentricity) / (1.0 - cube_eccentricity * cos(E));
    float sin_v = sqrt(1.0 - cube_eccentricity * cube_eccentricity) * sin(E) / (1.0 - cube_eccentricity * cos(E));
    float v = atan(sin_v, cos_v); // Use atan(y, x) for GLSL 450 compatibility

    // Distance from focus
    float r = cube_semiMajorAxis * (1.0 - cube_eccentricity * cube_eccentricity) / (1.0 + cube_eccentricity * cos(v));

    // Position in orbital plane
    vec3 pos = vec3(r * cos(v), r * sin(v), 0.0);

    // Rotation matrices for orbital elements
    mat3 rotOmega = mat3(
        cos(cube_longAscNode), -sin(cube_longAscNode), 0.0,
        sin(cube_longAscNode),  cos(cube_longAscNode), 0.0,
        0.0,                   0.0,                  1.0
    );
    mat3 rotI = mat3(
        1.0, 0.0,                 0.0,
        0.0, cos(cube_inclination), -sin(cube_inclination),
        0.0, sin(cube_inclination),  cos(cube_inclination)
    );
    mat3 rotOmegaPeri = mat3(
        cos(cube_argPeriapsis), -sin(cube_argPeriapsis), 0.0,
        sin(cube_argPeriapsis),  cos(cube_argPeriapsis), 0.0,
        0.0,                    0.0,                   1.0
    );

    // Transform position to world coordinates
    vec3 cubeCenter = rotOmega * rotI * rotOmegaPeri * pos;

    // Cube SDF
    float cubeDist = cubeSDF(p, cubeCenter, 1.0); // Unit cube
    vec3 cubeColor = vec3(1.0, 0.0, 0.0); // Red cube

    // Grid
    float gridDist = gridSDF(p);
    vec3 gridColor = vec3(0.0, 0.0, 0.0); // Black grid

    // Smoothly blend sphere and cube
    float k = 0.5; // Blending factor
    float dist = smin(sphereDist, cubeDist, k);
    dist = min(dist, gridDist);

    // Blend colors based on distances
    vec3 color;
    if (dist == gridDist) {
        color = gridColor;
    } else {
        float t = clamp(0.5 + 0.5 * (cubeDist - sphereDist) / k, 0.0, 1.0);
        color = mix(cubeColor, sphereColor, t);
    }

    return SceneHit(dist, color);
}

vec3 calcNormal(vec3 p) {
    float h = 0.001;
    vec2 k = vec2(1, -1);
    return normalize(
        k.xyy * sceneSDF(p + k.xyy * h).dist +
        k.yyx * sceneSDF(p + k.yyx * h).dist +
        k.yxy * sceneSDF(p + k.yxy * h).dist +
        k.xxx * sceneSDF(p + k.xxx * h).dist
    );
}

void main() {
    float aspectRatio = ubo.proj[1][1] / ubo.proj[0][0];
    vec2 uv = fragCoord * 0.5;
    uv.x *= aspectRatio;
    vec3 ro = ubo.camPos;
    vec3 rd = normalize((inverse(ubo.view) * inverse(ubo.proj) * vec4(uv, 1.0, 1.0)).xyz);

    float t = 0.0;
    vec3 p;
    bool hit = false;
    vec3 color;
    for (int i = 0; i < 100; ++i) {
        p = ro + rd * t;
        SceneHit hitInfo = sceneSDF(p);
        float dist = hitInfo.dist;
        if (dist < 0.001) {
            hit = true;
            color = hitInfo.color;
            break;
        }
        t += dist;
        if (t > 400.0) break;
    }

    vec4 bgColor = vec4(1.0, 1.0, 1.0, 1.0); // White background
    vec4 fogColor = vec4(1.0, 1.0, 1.0, 1.0); // White fog
    float fogDensity = 0.01;

    float fogAmount = 1.0 - exp(-fogDensity * t);

    if (hit) {
        // Simple diffuse lighting
        vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0)); // Directional light from (1, 1, 1)
        vec3 normal = calcNormal(p);
        float diffuse = max(dot(normal, lightDir), 0.0); // Lambertian diffuse term
        float lightIntensity = 0.8; // Adjustable light intensity
        float ambient = 0.2; // Ambient term to avoid complete darkness
        vec3 litColor = color * (diffuse * lightIntensity + ambient);

        outColor = mix(vec4(litColor, 1.0), fogColor, fogAmount);
    } else {
        outColor = mix(bgColor, fogColor, fogAmount);
    }
}
