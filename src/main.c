#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>

#include "../include/glad/glad.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "../include/linmath.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


static const float vertices[8] = {
    -1.f, -1.f,
    -1.f, 1.f,
    1.f, -1.f,
    1.f, 1.f,
};

GLuint load_shd(const char *filename, GLenum type, const char *entry);


int main(void) {
    if (!glfwInit())
        return -1;

    GLFWwindow *window = glfwCreateWindow(640, 480, "Hello, World!", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    
    size_t tex_w = 64;
    size_t tex_h = 48;
    float *thing = calloc(4 * tex_w * tex_h, sizeof(float));
    thing[0] = 1.f;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    
    GLuint vert_buff;
    glad_glGenBuffers(1, &vert_buff);
    glad_glBindBuffer(GL_ARRAY_BUFFER, vert_buff);
    glad_glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint ray_text;
    glad_glGenTextures(1, &ray_text);
    glad_glActiveTexture(GL_TEXTURE0);
    glad_glBindTexture(GL_TEXTURE_2D, ray_text);
    glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glad_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, thing);
    
    glad_glBindImageTexture(0, ray_text, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    
    const GLuint comp = load_shd("../shd/raytrace.comp.spv", GL_COMPUTE_SHADER, "main");
    const GLuint vert = load_shd("../shd/texture.vert.spv", GL_VERTEX_SHADER, "main");
    const GLuint frag = load_shd("../shd/texture.frag.spv", GL_FRAGMENT_SHADER, "main");

    const GLuint compute = glad_glCreateProgram();
    glad_glAttachShader(compute, comp);
    glad_glLinkProgram(compute);

    const GLuint program = glad_glCreateProgram();
    glad_glAttachShader(program, vert);
    glad_glAttachShader(program, frag);
    glad_glLinkProgram(program);

    glad_glDeleteShader(frag);
    glad_glDeleteShader(vert);
    glad_glDeleteShader(comp);
    
    const GLint tex_loc = 0;
    const GLint vpos_loc = 0;
    
    glad_glEnableVertexAttribArray(vpos_loc);
    glad_glVertexAttribPointer(vpos_loc, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void *)0);
    

    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        const float ratio = width / (float)height;
        
        glad_glViewport(0, 0, width, height);

        glad_glUseProgram(compute);
        glad_glUniform1i(tex_loc, 0);

        glad_glDispatchCompute((tex_w + 15) / 16, (tex_h + 15) / 16, 1);
        
        glad_glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glad_glUseProgram(program);
        glad_glUniform1i(tex_loc, 0);

        glad_glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glad_glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glad_glDeleteProgram(program);
    glad_glDeleteTextures(1, &ray_text);
    glad_glDeleteBuffers(1, &vert_buff);
    free(thing);
    glfwTerminate();
    return 0;
}


GLuint load_shd(const char *filename, GLenum type, const char *entry) {
    FILE *f = fopen(filename, "rb");
    fseek(f, 0, SEEK_END);
    const size_t len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buff = malloc(len);
    fread(buff, len, 1, f);
    fclose(f);

    const GLuint shd = glad_glCreateShader(type);
    glad_glShaderBinary(1, &shd, GL_SHADER_BINARY_FORMAT_SPIR_V, (const void *)buff, len);
    glad_glSpecializeShader(shd, entry, 0, NULL, NULL);
    
    GLint compiled;
    glad_glGetShaderiv(shd, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE) {
        GLint shd_len;
        glad_glGetShaderiv(shd, GL_INFO_LOG_LENGTH, &shd_len);

        if (shd_len > 0) {
            GLchar *log = malloc(sizeof(GLchar) * shd_len);
            glad_glGetShaderInfoLog(shd, shd_len, &shd_len, log);
            free(log);
        }
    }
    free(buff);

    return shd;
}
