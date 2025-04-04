/*
 BY Mazhar Ziadeh, Adnan Jaber, MTE department at KHAS

description:  This program is a simulation of particles collision inside a sphere container, the particles are generated randomly inside the sphere container,
              and they have random velocity, and the particles have a sphere shape, and the container has a sphere shape too, and the particles are bouncing
              inside the container, and they are colliding with each other.

              spheres are modifiable, and generated mathematically, the particles are generated mathematically too with the same logic, 
              camera uses the mouse to rotate around, and WASD keys to move in the scene, 
              F: Refresh particles
              G: Decrease the number of particles by 1, with a minimum of 1
              H : Increase the number of particles by 1
              J : Reset the number of particles to 10
              O : Toggle wireframe mode
              P : Toggle pause
              I : Toggle background gradient

*/

//_______________________________________________________________________________________________________________________
#include <filesystem>                                 // For file path manipulation
#include <iostream>                                  // For console output
#include <glad/glad.h>                              // For OpenGL function pointers
#include <GLFW/glfw3.h>                            // For creating a window
#include <glm/glm.hpp>                            // For math operations
#include <glm/gtc/matrix_transform.hpp>          // For matrix transformations
#include <glm/gtc/type_ptr.hpp>                 // For passing matrices to shaders
#include "MyShader.h"                          // For shader program management
#include "BufferManagerSphere.h"              // For sphere vertex and index buffers
#include "BufferManagerParticles.h"          // For particles vertex and index buffers
#include "Camera.h"                         // For camera movement
#include "Particles.h"                     // For particles movement
//_______________________________________________________________________________________________________________________



// Function to check OpenGL errors, in openGl the errors do not show up as debug error, so we need to check for them manually in the console
void CheckOpenGLError(const char* operation) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error:  " << operation << ": " << error << std::endl;
    }
}

//___________________________________________________________________________

// full hd resolution cuz why not?
const unsigned int width = 1920;
const unsigned int height = 1080;

// Number of segments and rings for the sphere, plz note that when i say "Spphere" i mean the contaier in which
// the particles are in, not the particles themselves. since both particles and the container have a sphere shape
const int numSphereSegments = 20; 
const int numSphereRings = 20;
// Number of segments and rings for the particles
const int numParticleSegments = 20;
const int numParticleRings = 20;
//___________________________________________________________________________
																			
std::vector<GLfloat> sphereVertices;
std::vector<GLuint> sphereIndices;


/* And God said let there be spheres,
/ not 3D object loaded, but actual mathematically generated spheres.
Function to generate a sphere, store the vertices, normals, texture coordinates, and indices.
*/




//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void generateSphere(bool useNormalsForColor = false) {
    float phi, theta;

    // Loop through rings (latitude)
    for (int ring = 0; ring <= numSphereRings; ++ring) {
        // Loop through segments (longitude)
        for (int segment = 0; segment <= numSphereSegments; ++segment) {
            // Compute spherical coordinates
            theta = ring * glm::pi<float>() / numSphereRings;
            phi = segment * 2.0f * glm::pi<float>() / numSphereSegments;

            // Convert spherical coordinates to Cartesian coordinates
            float x = std::cos(phi) * std::sin(theta);
            float y = std::cos(theta);
            float z = std::sin(phi) * std::sin(theta);

            // Positions
            sphereVertices.push_back(x);
            sphereVertices.push_back(y);
            sphereVertices.push_back(z);

            // Normals (normalized positions)
            glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
            sphereVertices.push_back(normal.x);
            sphereVertices.push_back(normal.y);
            sphereVertices.push_back(normal.z);

            // Texture coordinates
            sphereVertices.push_back(static_cast<float>(segment) / numSphereSegments);
            sphereVertices.push_back(static_cast<float>(ring) / numSphereRings);

            // Color based on normals (odd: red, even: blue)
            if (useNormalsForColor) {
                if (segment % 2 == 0) {
                    // Even segment
                    sphereVertices.push_back(1.0f);  //Red     (note: this is not working, so i changed the color from the .frag file)
                    sphereVertices.push_back(0.0f);  //Green   (note: this is not working, so i changed the color from the .frag file)
                    sphereVertices.push_back(0.0f);  //Blue    (note: this is not working, so i changed the color from the .frag file)
                }
                else {
                    // Odd segment
                    sphereVertices.push_back(1.0f);  //Red     (note: this is not working, so i changed the color from the .frag file)
                    sphereVertices.push_back(0.0f);  //Green   (note: this is not working, so i changed the color from the .frag file)
                    sphereVertices.push_back(0.0f);  //Blue    (note: this is not working, so i changed the color from the .frag file)

                    // for more info about the issue plz check readme file
                }
            }
        }
    }

    // Generate indices for triangles
    for (int ring = 0; ring < numSphereRings; ++ring) {
        for (int segment = 0; segment < numSphereSegments; ++segment) {
            int nextRow = ring + 1;
            int nextColumn = segment + 1;

            // Define indices for two triangles forming a quad
            sphereIndices.push_back(ring * (numSphereSegments + 1) + segment);
            sphereIndices.push_back(nextRow * (numSphereSegments + 1) + segment);
            sphereIndices.push_back(ring * (numSphereSegments + 1) + nextColumn);

            sphereIndices.push_back(nextRow * (numSphereSegments + 1) + segment);
            sphereIndices.push_back(nextRow * (numSphereSegments + 1) + nextColumn);
            sphereIndices.push_back(ring * (numSphereSegments + 1) + nextColumn);
        }
    }
}
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//same thing for particles
std::vector<GLfloat> ParticleVertices;
std::vector<GLuint> ParticleIndices;

void generateParticle(bool useNormalsForColor = false) {
    float phi, theta;

    for (int ring = 0; ring <= numParticleRings; ++ring) {
        for (int segment = 0; segment <= numParticleSegments; ++segment) {
            theta = ring * glm::pi<float>() / numParticleRings;
            phi = segment * 2.0f * glm::pi<float>() / numParticleSegments;

            float x = std::cos(phi) * std::sin(theta);
            float y = std::cos(theta);
            float z = std::sin(phi) * std::sin(theta);

            // Positions
            ParticleVertices.push_back(x);
            ParticleVertices.push_back(y);
            ParticleVertices.push_back(z);

            // Normals (normalized positions)
            glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
            ParticleVertices.push_back(normal.x);
            ParticleVertices.push_back(normal.y);
            ParticleVertices.push_back(normal.z);

            // Texture coordinates
            ParticleVertices.push_back(static_cast<float>(segment) / numParticleSegments);
            ParticleVertices.push_back(static_cast<float>(ring) / numParticleRings);

            // Color based on normals (odd:red, even:blue)
            if (useNormalsForColor) {
                if (segment % 2 == 0) {
                    // Even segment
                    ParticleVertices.push_back(0.0f);  // Red    (note: this is not working, so i changed the color from the .frag file)
                    ParticleVertices.push_back(0.0f);  // Green  (note: this is not working, so i changed the color from the .frag file)
                    ParticleVertices.push_back(0.0f);  // Blue   (note: this is not working, so i changed the color from the .frag file)
                }
                else {
                    // Odd segment
                    ParticleVertices.push_back(0.0f);  // Red    (note: this is not working, so i changed the color from the .frag file)
                    ParticleVertices.push_back(0.0f);  // Green  (note: this is not working, so i changed the color from the .frag file)
                    ParticleVertices.push_back(0.0f);  // Blue   (note: this is not working, so i changed the color from the .frag file)

                    // for more info about the issue plz check readme file

                }
            }
        }
    }

    for (int ring = 0; ring < numParticleRings; ++ring) {
        for (int segment = 0; segment < numParticleSegments; ++segment) {
            int nextRow = ring + 1;
            int nextColumn = segment + 1;

            ParticleIndices.push_back(ring * (numParticleSegments + 1) + segment);
            ParticleIndices.push_back(nextRow * (numParticleSegments + 1) + segment);
            ParticleIndices.push_back(ring * (numParticleSegments + 1) + nextColumn);

            ParticleIndices.push_back(nextRow * (numParticleSegments + 1) + segment);
            ParticleIndices.push_back(nextRow * (numParticleSegments + 1) + nextColumn);
            ParticleIndices.push_back(ring * (numParticleSegments + 1) + nextColumn);
        }
    }
}


// Call this function once to generate sphere data

GLfloat* sphereVerticesArray = sphereVertices.data(); // Move the pointer initialization here
GLuint* sphereIndicesArray = sphereIndices.data(); // Move the pointer initialization here
std::vector<Particles> currentParticles; // Vector to store the current particles

//--------------------------------------------------------------------------------------------------------------
// flags to control the program                                                
bool pauseParticles = false; // Flag to control particle motion, pause or not by pressing P key 
bool refreshParticles = false; // Flag to control particle refresh by pressing F key
bool wireframeMode = false; // Flag to activate wireframe mode so you can check how the sphere actaully looks like, by pressing O key  
bool drawBackgroundGradient = true; // Flag to control the background gradient, enable or disable by pressing I key
//--------------------------------------------------------------------------------------------------------------


int numParticles = 10; // int to store the number of particles, since can the number can be changed by pressing G or H key, and reset by pressing J key

void initializeParticles() { // Function to initialize particles

    // Seed for random number generation, specific seed value is not important since the shpere is generated randomly and everytime the program
    // is refreshed, you might notice that the particles have different positions, and also diffrent velocity. 
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    for (int i = 0; i < numParticles; ++i) { // Loop to generate particles with random positions
        glm::vec3 position(
            static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f,
            static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f,
            static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f
        );

        glm::vec3 velocity( // Loop to generate particles with random velocity
            static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f,
            static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f,
            static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f
        );

        float radius = 0.1f; //downscale size of particles compared to a sphere container, and it's set to 0.1f as the optimal ratio

        currentParticles.emplace_back(position, velocity, radius); // Add the particle to the vector
    }
}

/* Function to handle key presses for the program
* F: Refresh particles
* G: Decrease the number of particles by 1, with a minimum of 1
* H: Increase the number of particles by 1
* J: Reset the number of particles to 10
* O: Toggle wireframe mode
* P: Toggle pause
* I: Toggle background gradient
*/

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        // Set a flag to indicate that particles should be refreshed
        refreshParticles = true;
    }
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_O) {
            // Toggle wireframe mode
            wireframeMode = !wireframeMode;
            glPolygonMode(GL_FRONT_AND_BACK, wireframeMode ? GL_LINE : GL_FILL);
        }
        if (key == GLFW_KEY_G) {
            // Decrease the number of particles by 1, with a minimum of 1
            numParticles = std::max(1, numParticles - 1);
            refreshParticles = true;
        }
        else if (key == GLFW_KEY_H) {
            // Increase the number of particles by 1
            numParticles += 1;
            refreshParticles = true;
        }
        else if (key == GLFW_KEY_J) {
            numParticles = 10; // Adjust to your initial value
            refreshParticles = true;
        }
        if (key == GLFW_KEY_P && action == GLFW_PRESS) {
            // Toggle the pause flag
            pauseParticles = !pauseParticles;
        }
        if (key == GLFW_KEY_I && action == GLFW_PRESS) {
            // Toggle the flag to enable/disable background gradient
            drawBackgroundGradient = !drawBackgroundGradient;
        }

    }
}

/* wannna check FPS? we got you, check the consile for the FPS,
Function to get current time in milliseconds, used for FPS calculation,
but for some reason we got a fps cap limit which is 40.
even though i tried to disable V-Sync, and i tried to change the glfwSwapInterval(0) to glfwSwapInterval(1)
*/

#include <chrono>
long long getCurrentTimeMillis() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}
//_______________________________________________________________________________________________________________________
int main() { // main function for everything


    // Move the pointer initialization here
    GLfloat* sphereVerticesArray = sphereVertices.data();
    GLuint* sphereIndicesArray = sphereIndices.data();
    // Move the pointer initialization here
    GLfloat* ParticleVerticesArray = ParticleVertices.data();
    GLuint* ParticleIndicesArray = ParticleIndices.data();

    // Call this function once to generate sphere data
    generateSphere();
    generateParticle();

    // Initialize GLFW
    glfwInit();

    // Set GLFW window hints for OpenGL version and profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Core profile

    // Create GLFW window
    GLFWwindow* window = glfwCreateWindow(width, height, "Particles Collision", NULL, NULL); // Create a windowed mode window and its OpenGL context
    if (window == NULL) { // Check if the window was created
        std::cout << "Failed to create GLFW window" << std::endl; // Print error message
        glfwTerminate(); // Terminate GLFW
        return -1; // Return error code
    }

    glfwMakeContextCurrent(window); // Make the window's context current

    // Load OpenGL functions using GLAD, GLAD manages function pointers for OpenGL
    gladLoadGL(); // Initialize GLAD before calling any OpenGL functions

    // Set the viewport dimensions
    glViewport(0, 0, width, height); // Set the size of the rendering window

    // Create shader program and set up OpenGL settings for container 
    Shader SphereShader("container.vert", "container.frag"); // imports the shader program object for the container only(check the "Resources Files/Shaders")

    CheckOpenGLError("after shader program and set up OpenGL settings for sphere container"); // Check for errors, if there is any, it will print the error in the console

    // Create shader program and set up OpenGL settings for particles  
    Shader ParticleShader("particles.vert", "particles.frag"); // imports the shader program object for the particles only(check the "Resources Files/Shaders")
    CheckOpenGLError("after shader program and set up OpenGL settings for particles"); // Check for errors, if there is any, it will print the error in the console

    // Create shader program and set up OpenGL settings for background  
    Shader backgroundShader("background.vert", "background.frag"); // imports the shader program object for the background only(check the "Resources Files/Shaders")
    CheckOpenGLError("after shader program and set up OpenGL settings for background"); // Check for errors, if there is any, it will print the error in the console

    // Create and set up Vertex Array Object (VAO) for container sphere, 
    BufferManagerSphere::SphereVAO sphereVAO; // imports the Vertex Array Object (VAO) for the container sphere
    sphereVAO.SphereBind(); // Bind the Vertex Array Object (VAO) for the container sphere
    CheckOpenGLError("after creating and set up Vertex Array Object (VAO) for sphere"); // Check for errors, if there is any, it will print the error in the console

    // Create and set up Vertex Array Object (VAO) for particles
    BufferManagerParticles::ParticlesVAO particlesVAO; // imports the Vertex Array Object (VAO) for the particles
    particlesVAO.ParticlesBind(); // Bind the Vertex Array Object (VAO) for the particles
    CheckOpenGLError("after creating and set up Vertex Array Object (VAO) for particles"); // Check for errors, if there is any, it will print the error in the console

    // Create and set up Vertex Buffer Object (VBO) and Element Buffer Object (EBO) for container sphere
    BufferManagerSphere::SphereVBO sphereVBO(&sphereVertices[0], sphereVertices.size() * sizeof(GLfloat)); // imports the Vertex Buffer Object (VBO) for the container sphere
    BufferManagerSphere::SphereEBO sphereEBO(&sphereIndices[0], sphereIndices.size() * sizeof(GLuint)); // imports the Element Buffer Object (EBO) for the container sphere
    CheckOpenGLError("after creating Vertex Buffer Object (VBO) and Element Buffer Object (EBO) for sphere "); // Check for errors, if there is any, it will print the error in the console

    // Create and set up Vertex Buffer Object (VBO) and Element Buffer Object (EBO) for particles
    BufferManagerParticles::ParticlesVBO particlesVBO(&ParticleVertices[0], ParticleVertices.size() * sizeof(GLfloat)); // imports the Vertex Buffer Object (VBO) for the particles
    BufferManagerParticles::ParticlesEBO particlesEBO(&ParticleIndices[0], ParticleIndices.size() * sizeof(GLuint)); // imports the Element Buffer Object (EBO) for the particles

    CheckOpenGLError("after creating Vertex Buffer Object (VBO) and Element Buffer Object (EBO) for particles "); // Check for errors, if there is any, it will print the error in the console

    // Link attributes in VAO to the VBO for container sphere 
    sphereVAO.LinkAttrib(sphereVBO, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0); // Link the Vertex Buffer Object (VBO) for the container sphere
    sphereVAO.LinkAttrib(sphereVBO, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float))); // Link the Vertex Buffer Object (VBO) for the container sphere
    sphereVAO.LinkAttrib(sphereVBO, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float))); // Link the Vertex Buffer Object (VBO) for the container sphere
    sphereVAO.SphereUnbind(); // Unbind the Vertex Array Object (VAO) for the container sphere
    sphereVBO.SphereUnbind(); // Unbind the Vertex Buffer Object (VBO) for the container sphere
    sphereEBO.SphereUnbind(); // Unbind the Element Buffer Object (EBO) for the container sphere

    // Link attributes in VAO to the VBO for particles
    CheckOpenGLError("before setting attributes particles before mnyok define ParticlesUnbind "); // Check for errors, if there is any, it will print the error in the console
    particlesVAO.LinkAttrib(particlesVBO, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0); // Link the Vertex Buffer Object (VBO) for the particles

    CheckOpenGLError("after setting attributes particles before mnyok define ParticlesUnbind "); // Check for errors, if there is any, it will print the error in the console
    particlesVAO.LinkAttrib(particlesVBO, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float))); // Link the Vertex Buffer Object (VBO) for the particles

    CheckOpenGLError("after setting attributes particles before mm7on define ParticlesUnbind "); // Check for errors, if there is any, it will print the error in the console
    particlesVAO.LinkAttrib(particlesVBO, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float))); // Link the Vertex Buffer Object (VBO) for the particles

    CheckOpenGLError("after setting attributes particles before define ParticlesUnbind "); // Check for errors, if there is any, it will print the error in the console

    particlesVAO.ParticlesUnbind(); // Unbind the Vertex Array Object (VAO) for the particles
    particlesVBO.ParticlesUnbind(); // Unbind the Vertex Buffer Object (VBO) for the particles
    particlesEBO.ParticlesUnbind(); // Unbind the Element Buffer Object (EBO) for the particles
    CheckOpenGLError("after setting attributes particles and define ParticlesUnbind "); // Check for errors, if there is any, it will print the error in the console

    // Enable depth testing
    glEnable(GL_DEPTH_TEST); 

    // Set the depth function to less than or equal
    glDepthFunc(GL_LEQUAL);

    Camera camera(width, height, glm::vec3(0.0f, 0.0f, 3.0f));  // Move the camera back along the z-axis 

    // Initialize particles
    initializeParticles(); // Function to initialize particles

    // Set the key callback function
    glfwSetKeyCallback(window, key_callback); // Set the key callback function

    GLuint quadVAO, quadVBO, quadEBO; // Create and set up Vertex Array Object (VAO) for background
    float quadVertices[] = { // Vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        -100.0f, -100.0f, 0.0f,   // Bottom-left
         100.0f, -100.0f, 0.0f,   // Bottom-right
         100.0f,  100.0f, 0.0f,   // Top-right
        -100.0f,  100.0f, 0.0f    // Top-left
    };

    GLuint quadIndices[] = { // Note that we start from 0!
        0, 1, 2,
        2, 3, 0
    };

    // Initialize quad VAO, VBO, and EBO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glGenBuffers(1, &quadEBO);
    glBindVertexArray(quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    const int frameCountToAverage = 60; // Number of frames to average for FPS calculation
    int frameCount = 0; // Frame count for FPS calculation
    double totalFrameTime = 0.0; // Total frame time for FPS calculation
    long long lastFrameTime = getCurrentTimeMillis(); // Initialize last frame time

    CheckOpenGLError("before main loop"); // Check for errors, if there is any, it will print the error in the console
    //--------------------------------------------------------------------------------------------------------------
   






    // Main rendering loop, keep rendering until the window is closed.

    // Set the key callback function
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE); // Enable double buffering
    glfwSwapInterval(0); // Disable V-Sync, so we can check the FPS more than 40, but for some reason we got a fps cap limit which is 40.

    while (!glfwWindowShouldClose(window)) { // Loop until the user closes the window
        CheckOpenGLError("before  "); // Check for errors, if there is any, it will print the error in the console
        // Measure frame time

        long long currentFrameTime = getCurrentTimeMillis();// Get the current time in milliseconds
        double frameTime = static_cast<double>(currentFrameTime - lastFrameTime) / 1000.0; // Calculate the frame time in seconds
        lastFrameTime = currentFrameTime; // Update the last frame time

        // Update total frame time and frame count for FPS calculation
        totalFrameTime += frameTime; // Update the total frame time
        frameCount++; // Update the frame count

        // If enough frames have passed, calculate and display FPS
        if (frameCount >= frameCountToAverage) { // If enough frames have passed
            double averageFPS = frameCount / totalFrameTime; // Calculate the average FPS
            std::ostringstream oss; // Create a string stream
            oss << "FPS: " << std::fixed << std::setprecision(2) << averageFPS; // Set the output string

            // Print FPS in the same line without newline
            std::cout << "\r" << oss.str() << std::flush; // Print the FPS

            // Reset frame count and total frame time for the next average
            frameCount = 0; // Reset the frame count
            totalFrameTime = 0.0; // Reset the total frame time
        }

        // Clear the color and depth buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the color and depth buffers

        CheckOpenGLError("glClear"); // Check for errors, if there is any, it will print the error in the console

        // Draw the background gradient only if the flag is set
        if (drawBackgroundGradient) { // If the flag is set
            backgroundShader.Activate(); // Activate the background shader program
            glDisable(GL_DEPTH_TEST); // Disable depth testing, because we want the background to be behind everything

            // Convert hexadecimal colors to RGB,  gradient colors are from https://www.color-hex.com/color-palette/831
            glm::vec3 colorLeft = glm::vec3(167.0f / 255.0f, 197.0f / 255.0f, 132.0f / 255.0f); // Left color
            glm::vec3 colorRight = glm::vec3(234.0f / 255.0f, 227.0f / 255.0f, 192.0f / 255.0f); // Right color

            // Set the gradient colors in the shader
            glUniform3fv(glGetUniformLocation(backgroundShader.ID, "colorLeft"), 1, glm::value_ptr(colorLeft));    // Left color
            glUniform3fv(glGetUniformLocation(backgroundShader.ID, "colorMiddle"), 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));  // Middle color (white)
            glUniform3fv(glGetUniformLocation(backgroundShader.ID, "colorRight"), 1, glm::value_ptr(colorRight));  // Right color
            CheckOpenGLError("Setting gradient colors");// Check for errors, if there is any, it will print the error in the console


            // Render the background quad
            glBindVertexArray(quadVAO); // Bind the background quad VAO
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // Draw the background quad
            glBindVertexArray(0); // Unbind the background quad VAO
            CheckOpenGLError("Rendering background quad"); // Check for errors, if there is any, it will print the error in the console

            glEnable(GL_DEPTH_TEST); // Re-enable depth testing
        }

        // Enable wireframe mode if necessary
        if (wireframeMode) { // If wireframe mode is enabled 
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Enable wireframe mode
        } 
        else { // If wireframe mode is disabled
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Disable wireframe mode
        }
        CheckOpenGLError("Setting polygon mode"); // Check for errors, if there is any, it will print the error in the console

        // Activate the shader programs
        SphereShader.Activate(); // Activate the sphere shader program
        camera.Inputs(window); // Update camera 
        camera.Matrix(45.0f, 0.1f, 100.0f, SphereShader, "camMatrixParticles"); // Update camera matrix in the sphere shader program
        CheckOpenGLError("Activating SphereShader"); // Check for errors, if there is any, it will print the error in the console

        // Draw the main container sphere
        glm::mat4 modelSphere = glm::scale(glm::mat4(10.0f), glm::vec3(2.0f)); // Adjust scale as needed, the scale is set to 2.0f as the optimal ratio
        glUniformMatrix4fv(glGetUniformLocation(SphereShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(modelSphere)); // Set the model matrix in the shader program
        CheckOpenGLError("Setting model matrix for container sphere"); // Check for errors, if there is any, it will print the error in the console
        sphereVAO.SphereBind(); // Bind VAO and draw container sphere 
        sphereEBO.SphereBind(); // Bind VAO and draw container sphere
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0); // Draw container sphere 
        sphereEBO.SphereUnbind(); // Bind VAO and draw container sphere 
        sphereVAO.SphereUnbind(); // Bind VAO and draw container sphere
        CheckOpenGLError("Rendering container sphere"); // Check for errors, if there is any, it will print the error in the console

        // Check if particles need to be refreshed
        if (refreshParticles) { // If particles need to be refreshed
            // Clear the current particles and reinitialize
            currentParticles.clear(); // Clear the current particles
            initializeParticles(); // Reinitialize the particles

            // Reset the flag
            refreshParticles = false; // Reset the flag for refreshing particles
        }

        // Activate the particle shader program
        ParticleShader.Activate(); // Activate the particle shader program
        camera.Matrix(45.0f, 0.1f, 100.0f, ParticleShader, "camMatrixParticles"); // Update camera matrix in the particle shader program
        CheckOpenGLError("Activating ParticleShader"); // Check for errors, if there is any, it will print the error in the console

        for (auto& particle : currentParticles) { // Loop through all particles
            if (!pauseParticles) { // If particles are not paused
                particle.update(0.05f); // Adjust delta time as needed
                particle.handleWallCollisions(); // Handle wall collisions

                for (auto& other : currentParticles) { // Loop through all particles
                    if (&particle != &other) { // If the particles are not the same then
                        particle.handleParticlesCollisions(other); // Handle particle collisions
                    }
                }
            }

            // Draw particles, activate the particle shader program
            glm::mat4 modelParticles = glm::translate(glm::mat4(1.0), particle.getPosition()) // Adjust translation as needed
                * glm::scale(glm::mat4(1.0f), glm::vec3(particle.getRadius())); // Adjust scale as needed

            // Set the model matrix in the shader program
            glUniformMatrix4fv(glGetUniformLocation(ParticleShader.ID, "modelParticles"), 1, GL_FALSE, glm::value_ptr(modelParticles)); // Set the model matrix in the shader program
            CheckOpenGLError("Setting model matrix for particles"); // Check for errors, if there is any, it will print the error in the console

            // Bind VAO and draw small Particle for each particle
            particlesVAO.ParticlesBind();// Bind VAO and draw small Particle for each particle
            particlesEBO.ParticlesBind();// Bind VAO and draw small Particle for each particle
            glDrawElements(GL_TRIANGLES, ParticleIndices.size(), GL_UNSIGNED_INT, 0); // Draw small Particle for each particle
            particlesEBO.ParticlesUnbind(); // Bind VAO and draw small Particle for each particle
            particlesVAO.ParticlesUnbind(); // Bind VAO and draw small Particle for each particle
            CheckOpenGLError("Rendering particles");// Check for errors, if there is any, it will print the error in the console
        }

        // Swap buffers and poll events
        glfwSwapBuffers(window); // Swap front and back buffers
        glfwPollEvents(); // Poll for and process events
        CheckOpenGLError("Swap buffers and poll events"); // Check for errors, if there is any, it will print the error in the console
    }

    // Clean up resources
    sphereVAO.SphereDelete(); // Delete the Vertex Array Object (VAO) for the container sphere
    sphereVBO.SphereDelete(); // Delete the Vertex Buffer Object (VBO) for the container sphere
    sphereEBO.SphereDelete();  // Delete the Element Buffer Object (EBO) for the container sphere
    particlesVAO.ParticlesDelete(); // Delete the Vertex Array Object (VAO) for the particles
    particlesVBO.ParticlesDelete(); // Delete the Vertex Buffer Object (VBO) for the particles
    particlesEBO.ParticlesDelete(); // Delete the Element Buffer Object (EBO) for the particles
    SphereShader.Delete(); // Delete the shader program for the container sphere
    ParticleShader.Delete(); // Delete the shader program for the particles
    backgroundShader.Delete(); // Delete the shader program for the background
    glfwDestroyWindow(window); // Destroy the window and its context
    glfwTerminate(); // Terminate GLFW
    CheckOpenGLError("Clean up resources"); // Check for errors, if there is any, it will print the error in the console
    return 0; // Return success code

}
