#version 450
layout(location = 0) in vec2 fragCoord;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 camPos;
    float time;
    float aspectRatio;
    int showHUD;
} ubo;

layout(std430, binding = 1) buffer HUDData {
    vec2 crosshairPos;
    float crosshairSize;
    vec4 crosshairColor;
    vec2 windowPos;
    vec2 windowSize;
    float windowAlpha;
    vec3 playerPos;
    vec3 playerForward;
    vec4 playerRotation;
} hud;

float crosshairSDF(vec2 uv, vec2 center, float size, float thickness) {
    vec2 d = abs(uv - center);
    float h = size * 0.5; // Half length (32 pixels / 2)
    float t = thickness * 0.5; // Half thickness (4 pixels / 2)
    float horizontal = max(d.x - t, h - d.y);
    float vertical = max(d.y - t, h - d.x);
    return min(horizontal, vertical);
}

float tetrahedronSDF(vec3 p, vec3 center, float size) {
    vec3 v0 = center + vec3(0.0, size, 0.0);
    vec3 v1 = center + vec3(size * 0.866, -size * 0.5, size * 0.5);
    vec3 v2 = center + vec3(-size * 0.866, -size * 0.5, size * 0.5);
    vec3 v3 = center + vec3(0.0, -size * 0.5, -size);

    vec3 n0 = normalize(cross(v1 - v0, v2 - v0));
    vec3 n1 = normalize(cross(v2 - v1, v3 - v1));
    vec3 n2 = normalize(cross(v3 - v2, v0 - v2));
    vec3 n3 = normalize(cross(v0 - v3, v1 - v3));

    float d0 = dot(p - v0, n0);
    float d1 = dot(p - v1, n1);
    float d2 = dot(p - v2, n2);
    float d3 = dot(p - v3, n3);

    return max(max(d0, d1), max(d2, d3));
}

struct SceneHit {
    float dist;
    vec4 color;
};

SceneHit hudSDF(vec2 uv) {
    float minDist = 1e10;
    vec4 color = vec4(0.0);

    if (ubo.showHUD == 0) {
        return SceneHit(minDist, color);
    }

    // Crosshair
    float crossDist = crosshairSDF(uv, hud.crosshairPos, hud.crosshairSize, hud.crosshairSize / 8.0);
    if (crossDist < 0.001) {
        minDist = crossDist;
        color = hud.crosshairColor;
    }

    // Player status window
    vec2 windowMin = hud.windowPos - hud.windowSize * 0.5;
    vec2 windowMax = hud.windowPos + hud.windowSize * 0.5;
    if (uv.x >= windowMin.x && uv.x <= windowMax.x && uv.y >= windowMin.y && uv.y <= windowMax.y) {
        vec3 ro = vec3(0.0, 0.0, 5.0);
        vec3 rd = normalize(vec3(uv * hud.windowSize * 5.0, -1.0));
        vec3 p = ro;

        float tetraDist = tetrahedronSDF(p, hud.playerPos, 0.5);
        if (tetraDist < minDist) {
            minDist = tetraDist;
            color = vec4(0.18, 0.18, 0.18, hud.windowAlpha);
        }
    }

    return SceneHit(minDist, color);
}

void main() {
    vec2 uv = fragCoord * 0.5;
    uv.x *= ubo.aspectRatio;

    SceneHit hitInfo = hudSDF(uv);
    if (hitInfo.dist < 0.001) {
        outColor = hitInfo.color;
    } else {
        outColor = vec4(0.0); // Transparent
    }
}
