#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <lodepng.h>

#include <iostream>
#include <sstream>
#include <string>
#include <memory>
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <exception>
#include <vector>
#include <fstream>

using namespace std;
using namespace glm;

auto load_file(const string& fname) {
    ifstream file(fname);
    string line;
    vector<string> rv;
    while (getline(file, line)) {
        line += "\n";
        rv.push_back(line);
    }
    return rv;
}

auto compile_shader(GLenum type, const vector<string>& file) {
    GLuint rv = glCreateShader(type);
    if (rv == 0) {
        throw runtime_error("Failed to create shader!");
    }

    vector<const GLchar*> code;
    code.reserve(file.size());
    transform(begin(file), end(file), back_inserter(code), [](const auto& line) { return line.data(); });

    glShaderSource(rv, code.size(), &code[0], nullptr);

    glCompileShader(rv);

    GLint result;
    glGetShaderiv(rv, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        cerr << "Shader compilation failed!" << endl;

        GLint logLen;
        glGetShaderiv(rv, GL_INFO_LOG_LENGTH, &logLen);

        if (logLen > 0) {
            auto log = make_unique<GLchar[]>(logLen);
            glGetShaderInfoLog(rv, logLen, nullptr, log.get());

            cerr << "Shader compilation log:\n" << log.get() << endl;
        }
    }

    return rv;
}

auto link_program(GLuint vertex_shader, GLuint frag_shader) {
    GLuint rv = glCreateProgram();
    if (rv == 0) {
        throw runtime_error("Failed to create shader program!");
    }

    glAttachShader(rv, vertex_shader);
    glAttachShader(rv, frag_shader);
    glLinkProgram(rv);

    GLint result;
    glGetProgramiv(rv, GL_LINK_STATUS, &result);
    if (result == GL_FALSE) {
        cerr << "Shader link failed!" << endl;

        GLint logLen;
        glGetProgramiv(rv, GL_INFO_LOG_LENGTH, &logLen);

        if (logLen > 0) {
            auto log = make_unique<GLchar[]>(logLen);
            glGetProgramInfoLog(rv, logLen, nullptr, log.get());

            cerr << "Shader compilation log:\n" << log.get() << endl;
        }
    }

    return rv;
}

struct VAO {
    GLuint handle = 0;
    GLuint vbo = 0;
    int num_tris = 0;
};

VAO vao_from_obj(const string& fname, GLint posAttrib, GLint uvAttrib, GLint normAttrib) {
    ifstream file (fname);

    vector<vec3> pos;
    vector<vec2> uv;
    vector<vec3> norm;
    vector<GLfloat> data;
    int num_tris = 0;

    string line;
    string word;
    while (getline(file,line)) {
        istringstream iss (line);
        iss >> word;
        if (word == "v") {
            vec3 v;
            iss >> v.x >> v.y >> v.z;
            pos.push_back(v);
        } else if (word == "vt") {
            vec2 v;
            iss >> v.x >> v.y;
            uv.push_back(v);
        } else if (word == "vn") {
            vec3 v;
            iss >> v.x >> v.y >> v.z;
            norm.push_back(v);
        } else if (word == "f") {
            string fs[3];
            iss >> fs[0] >> fs[1] >> fs[2];
            for (auto& f : fs) {
                replace(begin(f),end(f),'/',' ');
                istringstream fiss (f);
                int ipos;
                int iuv;
                int inorm;
                fiss >> ipos >> iuv >> inorm;
                --ipos;
                --iuv;
                --inorm;
                GLfloat vals[] = {
                        pos[ipos].x,
                        pos[ipos].y,
                        pos[ipos].z,
                        uv[iuv].x,
                        1.f - uv[iuv].y,
                        norm[inorm].x,
                        norm[inorm].y,
                        norm[inorm].z,
                };
                data.insert(end(data),begin(vals),end(vals));
                ++num_tris;
            }
        } else if (word[0] == '#') {
        } else {
            clog << "Warning: Unknown OBJ directive \"" << word << "\"" << endl;
        }
    }

    VAO vao;
    glGenBuffers(1, &vao.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vao.vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size()*sizeof(GLfloat), &data[0], GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao.handle);
    vao.num_tris = num_tris;

    glBindVertexArray(vao.handle);

    auto stride = sizeof(GLfloat)*(3+2+3);
    glEnableVertexAttribArray(posAttrib);
    glEnableVertexAttribArray(uvAttrib);
    glEnableVertexAttribArray(normAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const GLvoid*>(0));
    glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const GLvoid*>(sizeof(GLfloat)*3));
    glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const GLvoid*>(sizeof(GLfloat)*(3+2)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    clog << "Created vao " << vao.handle << " with " << vao.num_tris << " tris." << endl;
    return vao;
}

struct Texture {
    GLuint handle = 0;
    int width = 0;
    int height = 0;
};

Texture load_texture(const string& fname) {
    std::vector<unsigned char> image;
    unsigned width, height;
    unsigned error = lodepng::decode(image, width, height, fname);

    Texture rv;

    if (error != 0) {
        clog << "Error: Unable to load texture \"" << fname << "\"" << endl;
        return rv;
    }

    rv.width = width;
    rv.height = height;

    glGenTextures(1, &rv.handle);
    glBindTexture(GL_TEXTURE_2D, rv.handle);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

    glBindTexture(GL_TEXTURE_2D, 0);

    return rv;
}

void error_cb(int error, const char* description) {
    ostringstream oss;
    oss << "ERROR " << error << ": " << description << endl;
    throw runtime_error(oss.str());
}

int main() try {
    glfwSetErrorCallback(error_cb);

    if (!glfwInit()) {
        throw runtime_error("Failed to init GLFW!");
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Shader Sandy", nullptr, nullptr);
    if (!window) {
        throw runtime_error("Failed to open window!");
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGL()) {
        throw runtime_error("Failed to load GL!");
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glDepthFunc(GL_LESS);
    glClearDepth(1.f);

    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, load_file("data/vertex.glsl"));
    GLuint frag_shader = compile_shader(GL_FRAGMENT_SHADER, load_file("data/frag.glsl"));
    GLuint shader = link_program(
            vertex_shader,
            frag_shader
    );
    glDeleteShader(vertex_shader);
    glDeleteShader(frag_shader);
    glUseProgram(shader);

    VAO mesh = vao_from_obj(
            "data/kawaii.obj",
            glGetAttribLocation(shader, "VertexPosition"),
            glGetAttribLocation(shader, "VertexTexcoord"),
            glGetAttribLocation(shader, "VertexNormal")
    );

    Texture meshTexture = load_texture("data/kawaii.png");

    mat4 camProj = perspective(90.f, 4.f/3.f, 0.01f, 100.f);
    mat4 camView = translate(mat4(1.f), vec3(0.f, -2.f, -5.f));
    mat4 modelPos = mat4(1.f);

    GLint camProjUniform = glGetUniformLocation(shader, "camProj");
    GLint camViewUniform = glGetUniformLocation(shader, "camView");
    GLint modelPosUniform = glGetUniformLocation(shader, "modelPos");

    double last_time = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        double this_time = glfwGetTime();
        double delta = this_time - last_time;

        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUniformMatrix4fv(camProjUniform, 1, GL_FALSE, value_ptr(camProj));
        glUniformMatrix4fv(camViewUniform, 1, GL_FALSE, value_ptr(camView));
        glUniformMatrix4fv(modelPosUniform, 1, GL_FALSE, value_ptr(modelPos));

        glBindTexture(GL_TEXTURE_2D, meshTexture.handle);
        glBindVertexArray(mesh.handle);
        glDrawArrays(GL_TRIANGLES, 0, mesh.num_tris*3);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();

        modelPos = rotate(mat4(1.f), float(this_time), vec3(0.f, 1.f, 0.f));

        last_time = this_time;
    }

    glDeleteVertexArrays(1, &mesh.handle);
    glDeleteProgram(shader);
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
} catch (const exception& e) {
    cerr << e.what() << endl;
    glfwTerminate();
    return EXIT_FAILURE;
}
