#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;

out vec3 worldPos;
out vec3 nearPoint;
out vec3 farPoint;

vec3 unprojectPoint(float x, float y, float z, mat4 viewInv, mat4 projInv) {
    vec4 unprojectedPoint = viewInv * projInv * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main() {
    mat4 viewInv = inverse(view);
    mat4 projInv = inverse(projection);
    
    // Use position as NDC coordinates for the quad
    nearPoint = unprojectPoint(aPos.x, aPos.z, 0.0, viewInv, projInv).xyz; // unprojecting on the near plane
    farPoint = unprojectPoint(aPos.x, aPos.z, 1.0, viewInv, projInv).xyz; // unprojecting on the far plane
    worldPos = aPos;
    
    gl_Position = vec4(aPos.x, aPos.z, 0.0, 1.0); // using directly the clipped coordinates
}