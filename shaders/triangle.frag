#version 430

const int buffer_width = BUFFER_WIDTH;
const int buffer_height = BUFFER_HEIGHT;

out vec4 color;

in vec2 v_texture_coordinate;

layout(binding = 0, r32i) uniform iimage2D input_world;

const int mode = 0;

void main(){
    if(mode == 0) {
        int x = int(float(buffer_width) * v_texture_coordinate.x);
        int y = int(float(buffer_height) * v_texture_coordinate.y);

        if(imageLoad(input_world, ivec2(x, y)).x == 1) {
            color = vec4(1.0, 1.0, 1.0, 1.0);
        }
        else {
            color = vec4(0.0, 0.0, 0.0, 1.0);
        }
    }
    else if(mode == 1) {
        float x = float(buffer_width) * v_texture_coordinate.x;
        float y = float(buffer_height) * v_texture_coordinate.y;
        float y_mod = y - floor(y);
        float x_mod = x - floor(x);
        int left = int(floor(x));
        int right = left + 1;
        int down = int(floor(y));
        int top = down + 1;

        float brightness = 0.0;

        if(imageLoad(input_world, ivec2(left, down)).x == 1) {
            brightness += 1.0f - y_mod + 1.0f - x_mod;
        }
        if(imageLoad(input_world, ivec2(left, top)).x == 1) {
            brightness += y_mod + 1.0f - x_mod;
        }
        if(imageLoad(input_world, ivec2(right, down)).x == 1) {
            brightness += 1.0f - y_mod + x_mod;
        }
        if(imageLoad(input_world, ivec2(right, top)).x == 1) {
            brightness += y_mod + x_mod;
        }

        brightness /= 4.0f;

        color = vec4(brightness, brightness, brightness, 1.0);
    }
}