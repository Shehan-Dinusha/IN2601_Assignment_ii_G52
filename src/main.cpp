#define MODE 1 // Set to 1 to run the Colorful Triangle (GLFW), Set to 2 to run Line Clipping (GLUT)

#if MODE == 1
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// Vertex Shader source code
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "out vec3 ourColor;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 1.0);\n"
    "   ourColor = aColor;\n"
    "}\n\0";

// Fragment Shader source code
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 ourColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(ourColor, 1.0f);\n"
    "}\n\0";

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main() {
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Colorful Triangle", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Build and compile shader program
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Set up vertex data (positions and colors)
    float vertices[] = {
        // positions         // colors
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // bottom right (Red)
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left (Green)
         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // top (Blue)
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // Clear screen
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw the triangle
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // De-allocate resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}


#elif MODE == 2
#include <GL/glut.h>
#include <iostream>
#include <vector>

using namespace std;

// // Clipping window boundaries
float xmin = 100, ymin = 100, xmax = 300, ymax = 250;

// // Line coordinates
float x1_orig, y1_orig, x2_orig, y2_orig;
float cx1, cy1, cx2, cy2;

// // States
bool isClipped = false;
enum ClipMethod { NONE, LIANG_BARSKY, COHEN_SUTHERLAND };
ClipMethod currentMethod = NONE;

// // --- COHEN-SUTHERLAND CONSTANTS & FUNCTION ---
const int INSIDE = 0; // 0000
const int LEFT = 1;   // 0001
const int RIGHT = 2;  // 0010
const int BOTTOM = 4; // 0100
const int TOP = 8;    // 1000

int computeCode(double x, double y) {
    int code = INSIDE;
    if (x < xmin) code |= LEFT;
    else if (x > xmax) code |= RIGHT;
    if (y < ymin) code |= BOTTOM;
    else if (y > ymax) code |= TOP;
    return code;
}

void cohenSutherland(float x1, float y1, float x2, float y2) {
    int code1 = computeCode(x1, y1);
    int code2 = computeCode(x2, y2);
    bool accept = false;

    while (true) {
        if (!(code1 | code2)) { accept = true; break; }
        else if (code1 & code2) { break; }
        else {
            int codeOut = code1 ? code1 : code2;
            float x, y;
            if (codeOut & TOP) { x = x1 + (x2 - x1) * (ymax - y1) / (y2 - y1); y = ymax; }
            else if (codeOut & BOTTOM) { x = x1 + (x2 - x1) * (ymin - y1) / (y2 - y1); y = ymin; }
            else if (codeOut & RIGHT) { y = y1 + (y2 - y1) * (xmax - x1) / (x2 - x1); x = xmax; }
            else if (codeOut & LEFT) { y = y1 + (y2 - y1) * (xmin - x1) / (x2 - x1); x = xmin; }

            if (codeOut == code1) { x1 = x; y1 = y; code1 = computeCode(x1, y1); }
            else { x2 = x; y2 = y; code2 = computeCode(x2, y2); }
        }
    }
    if (accept) { cx1 = x1; cy1 = y1; cx2 = x2; cy2 = y2; isClipped = true; }
}

// // --- LIANG-BARSKY FUNCTION ---
void liangBarsky(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1, dy = y2 - y1;
    float p[] = {-dx, dx, -dy, dy};
    float q[] = {x1 - xmin, xmax - x1, y1 - ymin, ymax - y1};
    float t1 = 0, t2 = 1;

    for (int i = 0; i < 4; i++) {
        if (p[i] == 0 && q[i] < 0) return;
        float t = q[i] / p[i];
        if (p[i] < 0) { if (t > t1) t1 = t; }
        else if (p[i] > 0) { if (t < t2) t2 = t; }
    }
    if (t1 < t2) {
        cx1 = x1 + t1 * dx; cy1 = y1 + t1 * dy;
        cx2 = x1 + t2 * dx; cy2 = y1 + t2 * dy;
        isClipped = true;
    }
}

// // --- RENDERING ---
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

//     // Window (White)
    glColor3f(1, 1, 1);
    glBegin(GL_LINE_LOOP);
    glVertex2f(xmin, ymin); glVertex2f(xmax, ymin);
    glVertex2f(xmax, ymax); glVertex2f(xmin, ymax);
    glEnd();

//     // Original Line (Red)
    glColor3f(1, 0, 0);
    glBegin(GL_LINES);
    glVertex2f(x1_orig, y1_orig); glVertex2f(x2_orig, y2_orig);
    glEnd();

//     // Clipped Line (Blue)
    if (isClipped) {
        glColor3f(0, 0, 1);
        glLineWidth(3.0);
        glBegin(GL_LINES);
        glVertex2f(cx1, cy1); glVertex2f(cx2, cy2);
        glEnd();
        glLineWidth(1.0);
    }
    glFlush();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case '1': x1_orig = 150; y1_orig = 150; x2_orig = 250; y2_orig = 200; isClipped = false; break; // Inside
        case '2': x1_orig = 10; y1_orig = 10; x2_orig = 50; y2_orig = 50; isClipped = false; break;    // Outside
        case '3': x1_orig = 50; y1_orig = 150; x2_orig = 350; y2_orig = 200; isClipped = false; break; // Left-Right
        case '4': x1_orig = 200; y1_orig = 50; x2_orig = 220; y2_orig = 280; isClipped = false; break; // Bottom-Top
        case 'c': case 'C': cohenSutherland(x1_orig, y1_orig, x2_orig, y2_orig); break;
        case 'l': case 'L': liangBarsky(x1_orig, y1_orig, x2_orig, y2_orig); break;
        case 'r': case 'R': isClipped = false; break;
    }
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    // Default starting line (case 3)
    x1_orig = 50; y1_orig = 150; x2_orig = 350; y2_orig = 200;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(600, 500);
    glutCreateWindow("Clipping Demo: 1-4 Select Line, C/L Clip, R Reset");
    gluOrtho2D(0, 400, 0, 300);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}
#endif
