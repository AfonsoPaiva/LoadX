#version 330 core
out vec4 FragColor;

uniform vec3 gridColor;
uniform mat4 view;
uniform mat4 projection;

in vec3 worldPos;
in vec3 nearPoint;
in vec3 farPoint;

vec4 grid(vec3 fragPos3D, float scale, bool drawAxis) {
    vec2 coord = fragPos3D.xz * scale;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);
    vec4 color = vec4(gridColor, 1.0 - min(line, 1.0));
    
    // Z axis
    if(fragPos3D.x > -0.1 * minimumx && fragPos3D.x < 0.1 * minimumx)
        color.z = 1.0;
    // X axis
    if(fragPos3D.z > -0.1 * minimumz && fragPos3D.z < 0.1 * minimumz)
        color.x = 1.0;
        
    return color;
}

float computeDepth(vec3 pos) {
    vec4 clip_space_pos = projection * view * vec4(pos.xyz, 1.0);
    return (clip_space_pos.z / clip_space_pos.w);
}

float computeLinearDepth(vec3 pos) {
    vec4 clip_space_pos = projection * view * vec4(pos.xyz, 1.0);
    float clip_space_depth = (clip_space_pos.z / clip_space_pos.w) * 2.0 - 1.0; // put back between -1 and 1
    float linearDepth = (2.0 * 0.1 * 100.0) / (100.0 + 0.1 - clip_space_depth * (100.0 - 0.1)); // get linear value between 0.01 and 100
    return linearDepth / 100.0; // normalize
}

void main() {
    float t = -nearPoint.y / (farPoint.y - nearPoint.y);
    vec3 fragPos3D = nearPoint + t * (farPoint - nearPoint);
    
    gl_FragDepth = computeDepth(fragPos3D);
    
    float linearDepth = computeLinearDepth(fragPos3D);
    float fading = max(0, (0.5 - linearDepth));
    
    // Multiple grid scales for better visual effect
    vec4 grid1 = grid(fragPos3D, 10, true) * float(t > 0); // Large grid
    vec4 grid2 = grid(fragPos3D, 1, true) * float(t > 0);  // Small grid
    
    FragColor = (grid1 + grid2);
    FragColor.a *= fading;
    
    if(FragColor.a <= 0.0) discard;
}