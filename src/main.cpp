#include <GL/glew.h>
#include <iostream>
#include <GLFW/glfw3.h>
#include <fstream>
#include <chrono>
#include <thread>
#include <IL/il.h>

#define BUFFER_WIDTH  (1504)
#define BUFFER_HEIGHT (1024)
#define SCREEN_FACTOR 1

#define TICKS_PER_FRAME 30

unsigned int compile_shader(const std::string& source, unsigned int type) {
    unsigned int id = glCreateShader(type);
    const char* c_source = source.c_str();
    glShaderSource(id, 1, &c_source, nullptr);
    glCompileShader(id);

    // Error handling

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if(result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);

        std::cout << "Failed to compile: " << type << std::endl;
        std::cout << message << std::endl;
    }

    return id;
}

unsigned int CreateProgram(const std::string& vertex_source, const std::string& fragment_source) {
    std::cout << "creating program!" << std::endl;

    unsigned int program = glCreateProgram();
    unsigned int vertex_shader = compile_shader(vertex_source, GL_VERTEX_SHADER);

    std::cout << "built vertex shader!" << std::endl;

    unsigned int fragment_shader = compile_shader(fragment_source, GL_FRAGMENT_SHADER);

    std::cout << "built fragment shader!" << std::endl;


    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);

    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

void replace_substring(std::string* input, const std::string& substring, const std::string& replacement) {
    int index = 0;
    int substring_length = substring.length();

    while(true) {
        index = input->find(substring, index);

        if (index == std::string::npos) return;

        input->replace(index, substring_length, replacement);

        index += substring_length;
    }
}

void replace_buffer_size(std::string* input) {
    replace_substring(input, std::string("BUFFER_WIDTH"), std::to_string(BUFFER_WIDTH));
    replace_substring(input, std::string("BUFFER_HEIGHT"), std::to_string(BUFFER_HEIGHT));
}

float aspect_ratio = 1.0;

void on_resize(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    aspect_ratio = ((float)width / (float)BUFFER_WIDTH) / ((float)height / (float)BUFFER_HEIGHT);
}

GLuint create_compute_program(const std::string& shader_source) {
    GLuint program_id = glCreateProgram();
    GLuint shader_id = compile_shader(shader_source, GL_COMPUTE_SHADER);

    glAttachShader(program_id, shader_id);
    
    glLinkProgram(program_id);

    // check errors
    int result;
    glGetProgramiv(program_id, GL_LINK_STATUS, &result);
    if (!result) {
        std::cout << "Error in linking compute shader program" << std::endl;

        GLsizei length;
        glGetShaderiv(program_id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetProgramInfoLog(program_id, 10239, &length, message);
        std::cout << message << std::endl;
        exit(41);
    }   

    glValidateProgram(program_id);

    glDeleteShader(shader_id);
    
    return program_id;
}

// error checking
const char * GetGLErrorStr(GLenum err)
{
    switch (err)
    {
    case GL_NO_ERROR:          return "No error";
    case GL_INVALID_ENUM:      return "Invalid enum";
    case GL_INVALID_VALUE:     return "Invalid value";
    case GL_INVALID_OPERATION: return "Invalid operation";
    case GL_STACK_OVERFLOW:    return "Stack overflow";
    case GL_STACK_UNDERFLOW:   return "Stack underflow";
    case GL_OUT_OF_MEMORY:     return "Out of memory";
    default:                   return "Unknown error";
    }
}

void CheckGLError()
{
    while (true)
    {
        const GLenum err = glGetError();
        if (GL_NO_ERROR == err)
            break;

        std::cout << "GL Error: " << GetGLErrorStr(err) << std::endl;
    }
}

GLuint gen_empty_texture(int width, int height, int internal_format, int format, int type, GLuint unit) {
    GLuint id;

    glGenTextures(1, &id);
	glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    std::cout << "binding tex" << std::endl;
    CheckGLError();
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, type, nullptr);
    std::cout << "glTexImage2D" << std::endl;
    CheckGLError();
    glBindImageTexture(unit, id, 0, GL_FALSE, 0, GL_READ_WRITE, internal_format);

    std::cout << "done" << std::endl;
    CheckGLError();
}

/*
    Entry point
*/

typedef std::chrono::time_point<std::chrono::_V2::steady_clock, std::chrono::nanoseconds> Time;

int elapsed_microseconds(Time begin, Time end) {
    return std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
}

int main(int argc, char** argv) {
    // init glew / glfw
    glewExperimental = true;
    if(!glfwInit()) {
        std::cout << "Failed to initialize glfw!" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(BUFFER_WIDTH * SCREEN_FACTOR, BUFFER_HEIGHT * SCREEN_FACTOR, "Game of Life", nullptr, nullptr);
    if(window == nullptr) {
        std::cout << "Failed to initialize window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if(glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, on_resize);

    // open shader files
    std::ifstream vertex_file("./shaders/triangle.vert");
    std::string vertex_source((std::istreambuf_iterator<char>(vertex_file)),
                       (std::istreambuf_iterator<char>()));

    std::ifstream fragment_file("./shaders/triangle.frag");
    std::string fragment_source((std::istreambuf_iterator<char>(fragment_file)),
                       (std::istreambuf_iterator<char>()));

    std::ifstream comp_file("./shaders/game-of-life.comp");
    std::string comp_source((std::istreambuf_iterator<char>(comp_file)),
                       (std::istreambuf_iterator<char>()));

    std::ifstream fill_file("./shaders/fill-random.comp");
    std::string fill_source((std::istreambuf_iterator<char>(fill_file)),
                       (std::istreambuf_iterator<char>()));

    // std::cout << fragment_source << std::endl << vertex_source << std::endl;

    // create draw program
    replace_buffer_size(&vertex_source);
    replace_buffer_size(&fragment_source);
    unsigned int program_id = CreateProgram(vertex_source, fragment_source);

    // create compute program
    replace_buffer_size(&comp_source);
    GLuint compute_program_id = create_compute_program(comp_source);

    //create fill program
    replace_buffer_size(&fill_source);
    GLuint fill_program_id = create_compute_program(fill_source);
    

    GLuint vertex_array_id;
    glGenVertexArrays(1, &vertex_array_id);
    glBindVertexArray(vertex_array_id);

    static const float vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 

        -1.0f, -1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 
    };

    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);
    glUseProgram(program_id);

    static const float position_buffer_data[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
    };

    GLuint position_buffer;
    glGenBuffers(1, &position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(position_buffer_data), position_buffer_data, GL_STATIC_DRAW);
    glUseProgram(program_id);

    // compute shader stuff

    // static const GLuint dat[] = {
    //     0, 1, 0, 1,
    //     0, 1, 0, 1,
    //     0, 1, 0, 1,
    //     0, 1, 0, 1,
    // };

    static const float dat[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
    };
    

    // glGenTextures(1, &world_buffer);
	// glActiveTexture(GL_TEXTURE0);

    // glBindTexture(GL_TEXTURE_2D, world_buffer);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, BUFFER_WIDTH, BUFFER_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
    // glBindImageTexture(0, world_buffer, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    GLuint world_buffer = gen_empty_texture(BUFFER_WIDTH, BUFFER_HEIGHT, GL_R32I, GL_RED_INTEGER, GL_INT, 0);
    GLuint second_world_buffer = gen_empty_texture(BUFFER_WIDTH, BUFFER_HEIGHT, GL_R32I, GL_RED_INTEGER, GL_INT, 1);
    // GLuint world_buffer = gen_empty_texture(BUFFER_WIDTH, BUFFER_HEIGHT, GL_RGBA32F, GL_RGBA, GL_FLOAT, 0);
    // GLuint second_world_buffer = gen_empty_texture(BUFFER_WIDTH, BUFFER_HEIGHT, GL_RGBA32F, GL_RGBA, GL_FLOAT, 1);

    GLuint input_world_unit = 0;
    GLuint output_world_unit = 1;

    CheckGLError();


    glUseProgram(fill_program_id);

    glUniform1i(glGetUniformLocation(fill_program_id, "input_world"), input_world_unit);
    glDispatchCompute(BUFFER_WIDTH, BUFFER_HEIGHT, 1);
    glMemoryBarrier( GL_ALL_BARRIER_BITS );

    std::cout << "entering mainloop" << std::endl;

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    Time start_draw_time = std::chrono::steady_clock::now();
    Time previous_fps_time = start_draw_time;

    Time time_since_draw = std::chrono::steady_clock::now();
    Time time_since_begin = time_since_draw;
    int draw_count = 0;
    int tick_count = 0;
    int counter = 0;
    int ticks_in_current_frame = 0;

    do {
        start_draw_time = std::chrono::steady_clock::now();
        // run compute shader
        glUseProgram(compute_program_id);

        glUniform1i(glGetUniformLocation(compute_program_id, "input_world"), input_world_unit);
        glUniform1i(glGetUniformLocation(compute_program_id, "output_world"), output_world_unit);
        
        glDispatchCompute(BUFFER_WIDTH / 8, BUFFER_HEIGHT / 8, 1);
        glMemoryBarrier( GL_ALL_BARRIER_BITS );

        // draw
        
        //std::cout << elapsed_microseconds(time_since_draw, std::chrono::steady_clock::now()) << std::endl;

        //if(elapsed_microseconds(time_since_draw, std::chrono::steady_clock::now()) >= 2000) {
        if(++ticks_in_current_frame > TICKS_PER_FRAME) {
            ticks_in_current_frame = 0;
        //if(counter++ > 10) {
            // counter = 0;
            // if(elapsed_microseconds(time_since_draw, std::chrono::steady_clock::now()) > 20000) {
            //     std::cout << "slow frame" << std::endl;
            // }
            draw_count++;
            time_since_draw = std::chrono::steady_clock::now();
            //std::cout << "drawing! \n";
            glUseProgram(program_id);

            glClear(GL_COLOR_BUFFER_BIT);

            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

            glUniform1i(glGetUniformLocation(program_id, "input_world"), output_world_unit);
            glUniform1f(glGetUniformLocation(program_id, "aspect_ratio"), aspect_ratio);

            glDrawArrays(GL_TRIANGLES, 0, 3 * 2);
            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        
        CheckGLError();
        
        // std::this_thread::sleep_for(std::chrono::microseconds(1));


        // swap units
        GLuint temp = input_world_unit;
        input_world_unit = output_world_unit;
        output_world_unit = temp;
        
        tick_count++;
        
        if(elapsed_microseconds(previous_fps_time, std::chrono::steady_clock::now()) > 1000000) {
            previous_fps_time = std::chrono::steady_clock::now();
            std::cout << draw_count << " fps - " << tick_count << " ticks/s" << std::endl;
            draw_count = 0;
            tick_count = 0;
        }
    }
    while(!glfwGetKey(window, GLFW_KEY_ESCAPE) && !glfwWindowShouldClose(window));
    
    return 0;
}