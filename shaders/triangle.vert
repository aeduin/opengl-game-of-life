#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texture_coordinate;

out vec2 v_texture_coordinate;
uniform float aspect_ratio;

void main()
{
    if(aspect_ratio < 1.0) {
        gl_Position = vec4(position.x, position.y * aspect_ratio, position.z, 1.0);
    }
    else {
        gl_Position = vec4(position.x / aspect_ratio, position.y, position.z, 1.0);
    }
    v_texture_coordinate = texture_coordinate;
}