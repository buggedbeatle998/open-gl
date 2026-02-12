#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "../include/glad/glad.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "../include/linmath.h"


typedef struct {
    vec2 pos;
    vec3 col;
} Vertex;


static const Vertex vertices[3] = {
    {
        {0.5f, 0.5f},
        {1.f, 0.f, 0.f}
    },
    {
        {0.f, -0.5f},
        {0.f, 1.f, 0.f}
    },
    {
        {-0.5f, 0.5f},
        {0.f, 0.f, 1.f}
    }
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
    

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    
    GLuint vert_buff;
    glad_glGenBuffers(1, &vert_buff);
    glad_glBindBuffer(GL_ARRAY_BUFFER, vert_buff);
    glad_glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint ray_text;
    glad_glGenTextures(1, &ray_text);
    glad_glBindTexture(GL_TEXTURE_2D, ray_text);
    
    const GLuint vert = load_shd("../shd/triangle.vert.spv", GL_VERTEX_SHADER, "main");
    const GLuint frag = load_shd("../shd/triangle.frag.spv", GL_FRAGMENT_SHADER, "main");

    const GLuint program = glad_glCreateProgram();
    glad_glAttachShader(program, vert);
    glad_glAttachShader(program, frag);
    glad_glLinkProgram(program);
    glad_glDeleteShader(vert);
    glad_glDeleteShader(frag);
    
    const GLint vpos_loc = 0;//glad_glGetAttribLocation(program, "v_pos");
    printf("%d\n", vpos_loc);
    const GLint vcol_loc = 1;//glad_glGetAttribLocation(program, "v_col");
    printf("%d\n", vcol_loc);
    const GLint mvp_loc = 2;//glad_glGetUniformLocation(program, "MVP");
    printf("%d\n", mvp_loc);
    
    glad_glEnableVertexAttribArray(vpos_loc);
    glad_glVertexAttribPointer(vpos_loc, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void *)offsetof(Vertex, pos));
    glad_glEnableVertexAttribArray(vcol_loc);
    glad_glVertexAttribPointer(vcol_loc, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void *)offsetof(Vertex, col));

    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        const float ratio = width / (float)height;
        
        glad_glViewport(0, 0, width, height);
        glad_glClear(GL_COLOR_BUFFER_BIT);

        mat4x4 m, p, mvp;
        mat4x4_identity(m);
        mat4x4_rotate_Z(m, m, (float)glfwGetTime());
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);


        glad_glUseProgram(program);
        glad_glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, (const GLfloat*) &mvp);
        glad_glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glad_glDeleteProgram(program);
    glad_glDeleteBuffers(1, &vert_buff);
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
