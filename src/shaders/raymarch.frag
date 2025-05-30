#version 450
layout(location = 0) in vec2 fragCoord;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 camPos;
    float time;
    float aspectRatio;
    int showHUD; // Unused in world shader
} ubo;

struct Object {
    vec3 position;
    vec4 rotation; // Quaternion
    vec3 scale;
    vec4 color;
    int type; // 0: sphere, 1: box
    int materialIndex;
};

layout(std430, binding = 1) buffer ObjectData {
    Object objects[];
};

struct Material {
    float reflectivity;
    float smoothness;
    float transparency;
    float illumination;
};

Material materials[] = {
    {0.2, 0.5, 0.0, 0.0}, // Default: matte, opaque
    {0.8, 0.9, 0.0, 0.0}, // Shiny, reflective
    {0.5, 0.5, 0.5, 0.0}, // Semi-transparent
    {0.2, 0.5, 0.0, 0.5}  // Emissive
};

float sphereSDF(vec3 p, vec3 center, float radius) {
    return length(p - center) - radius;
}

float boxSDF(vec3 p, vec3 center, vec3 size) {
    vec3 d = abs(p - center) - size * 0.5;
    return length(max(d, 0.0)) + min(max(d.x, max(d.y, d.z)), 0.0);
}

struct SceneHit {
    float dist;
    vec4 color;
    int materialIndex;
};

SceneHit sceneSDF(vec3 p) {
    float minDist = 1e10;
    vec4 color = vec4(0.0);
    int materialIndex = 0;

    for (int i = 0; i < objects.length(); ++i) {
        vec3 objPos = objects[i].position;
        vec3 scale = objects[i].scale;
        vec4 rot = objects[i].rotation;
        float dist;

        // Apply rotation (quaternion)
        vec3 pLocal = p - objPos;
        vec3 q = rot.xyz;
        float w = rot.w;
        pLocal = pLocal + 2.0 * cross(q, cross(q, pLocal) + w * pLocal);

        if (objects[i].type == 0) { // Sphere
            dist = sphereSDF(pLocal, vec3(0.0), scale.x * 0.2); // Enemy radius 0.2
        } else { // Box
            dist = boxSDF(pLocal, vec3(0.0), scale);
        }

        if (dist < minDist) {
            minDist = dist;
            color = objects[i].color;
            materialIndex = objects[i].materialIndex;
        }
    }

    return SceneHit(minDist, color, materialIndex);
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
    vec2 uv = fragCoord * 0.5;
    uv.x *= ubo.aspectRatio;
    vec3 ro = ubo.camPos;
    vec3 rd = normalize((inverse(ubo.view) * inverse(ubo.proj) * vec4(uv, 1.0, 1.0)).xyz);

    float t = 0.0;
    vec3 p;
    bool hit = false;
    vec4 color;
    int materialIndex = 0;

    for (int i = 0; i < 100; ++i) {
        p = ro + rd * t;
        SceneHit hitInfo = sceneSDF(p);
        float dist = hitInfo.dist;
        if (dist < 0.001) {
            hit = true;
            color = hitInfo.color;
            materialIndex = hitInfo.materialIndex;
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
        vec3 lightPos = vec3(1.0, 1.0, 1.0);
        vec3 lightDir = normalize(lightPos - p);
        vec3 normal = calcNormal(p);
        float diffuse = max(dot(normal, lightDir), 0.0);
        float lightIntensity = 0.8;
        float ambient = 0.2;

        Material mat = materials[materialIndex];
        vec3 litColor = color.rgb * (diffuse * lightIntensity * (1.0 - mat.illumination) + ambient + mat.illumination);
        float alpha = color.a * (1.0 - mat.transparency) + mat.transparency * 0.5;

        outColor = mix(vec4(litColor, alpha), fogColor, fogAmount);
    } else {
        outColor = mix(bgColor, fogColor, fogAmount);
    }
}
