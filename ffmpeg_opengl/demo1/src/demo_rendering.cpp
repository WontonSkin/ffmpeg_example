#include <iostream>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_s.h>

#include "demo_rendering.h"

enum color_format { YUV420_10BE, YUV420_10LE, YUV444, YUV422, YUV420, UYVY, YUYV, YYY, PACKED_YUV444, NV12, NV21, RGB32, RGB24, RGB16 };
#define clip(var) ((var>=255)?255:(var<=0)?0:var)


namespace DEMO
{

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// stores how much we're seeing of either texture
float mixValue = 0.2f;

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    std::cout << "enter framebuffer_size_callback." << std::endl;
    
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        mixValue += 0.001f; // change this value accordingly (might be too slow or too fast based on system hardware)
        if(mixValue >= 1.0f)
            mixValue = 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        mixValue -= 0.001f; // change this value accordingly (might be too slow or too fast based on system hardware)
        if (mixValue <= 0.0f)
            mixValue = 0.0f;
    }
}

int doGenerateMipmap(int texture1, int width, int height, unsigned char *data)
{

    if ((0 == width) || (0 == height) || (NULL == data)) {
        std::cout << "illegal parameter" << std::endl;
        return -1;
    }

    
    // texture 1
    // ---------
    glBindTexture(GL_TEXTURE_2D, texture1);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // load image, create texture and generate mipmaps
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    return 0;
}


//unsigned char* rgba;
//unsigned char* misc;
//unsigned char* segment;
//rgba    = new unsigned char[t_width*t_height*4];
//segment = new unsigned char[t_width*t_height*4];

// reset alpha value to 255
//memset( rgba, 255, sizeof(unsigned char)*t_width*t_height*4 );

// used for NV12, NV21, UYVY, RGB32, RGB24, RGB16
//misc = new unsigned char[width*height*4];

#if 0
void yuv2rgb(color_format m_color, int width, int height)
{

    int j, i;
    int c, d, e;

    int stride_uv;

    int r, g, b;
    
    unsigned char* line = rgba;
    unsigned char* cur;

    short* rgb16;

    if( m_color == PACKED_YUV444) {
        for( j = 0 ; j < height ; j++ ){
            cur = line;
            for( i = 0 ; i < width ; i++ ){
                c = misc[(j*width+i)*3    ] - 16;
                d = misc[(j*width+i)*3 + 1] - 128;
                e = misc[(j*width+i)*3 + 2] - 128;

                (*cur) = clip(( 298 * c           + 409 * e + 128) >> 8);cur++;
                (*cur) = clip(( 298 * c - 100 * d - 208 * e + 128) >> 8);cur++;
                (*cur) = clip(( 298 * c + 516 * d           + 128) >> 8);cur+=2;
            }
            line += t_width<<2;
        }
    }
    else if( m_color == YUV444 ){
        for( j = 0 ; j < height ; j++ ){
            cur = line;
            for( i = 0 ; i < width ; i++ ){
                c = y[j*width+i] - 16;
                d = u[j*width+i] - 128;
                e = v[j*width+i] - 128;

                (*cur) = clip(( 298 * c           + 409 * e + 128) >> 8);cur++;
                (*cur) = clip(( 298 * c - 100 * d - 208 * e + 128) >> 8);cur++;
                (*cur) = clip(( 298 * c + 516 * d           + 128) >> 8);cur+=2;
            }
            line += t_width<<2;
        }
    }
    else if( m_color == YUV422 ){
        stride_uv = (width+1)>>1;

        for( j = 0 ; j < height ; j++ ){
            cur = line;
            for( i = 0 ; i < width ; i++ ){
                c = y[j*width+i] - 16;
                d = u[j*stride_uv+(i>>1)] - 128;
                e = v[j*stride_uv+(i>>1)] - 128;

                (*cur) = clip(( 298 * c           + 409 * e + 128) >> 8);cur++;
                (*cur) = clip(( 298 * c - 100 * d - 208 * e + 128) >> 8);cur++;
                (*cur) = clip(( 298 * c + 516 * d           + 128) >> 8);cur+=2;
            }
            line += t_width<<2;
        }
    }

    else if( m_color == UYVY ){
        unsigned char* t = misc;
        for( j = 0 ; j < height ; j++ ){
            cur = line;
            for( i = 0 ; i < width ; i+=2 ){
                c = *(t+1) - 16;    // Y1
                d = *(t+0) - 128;   // U
                e = *(t+2) - 128;   // V

                (*cur) = clip(( 298 * c           + 409 * e + 128) >> 8);cur++;
                (*cur) = clip(( 298 * c - 100 * d - 208 * e + 128) >> 8);cur++;
                (*cur) = clip(( 298 * c + 516 * d           + 128) >> 8);cur+=2;

                c = *(t+3) - 16;    // Y2
                (*cur) = clip(( 298 * c           + 409 * e + 128) >> 8);cur++;
                (*cur) = clip(( 298 * c - 100 * d - 208 * e + 128) >> 8);cur++;
                (*cur) = clip(( 298 * c + 516 * d           + 128) >> 8);cur+=2;

                t += 4;
            }
            line += t_width<<2;
        }
    }

    else if( m_color == YUYV ){
        unsigned char* t = misc;
        for( j = 0 ; j < height ; j++ ){
            cur = line;
            for( i = 0 ; i < width ; i+=2 ){
                c = *(t+0) - 16;    // Y1
                d = *(t+1) - 128;   // U
                e = *(t+3) - 128;   // V

                (*cur) = clip(( 298 * c           + 409 * e + 128) >> 8);cur++;
                (*cur) = clip(( 298 * c - 100 * d - 208 * e + 128) >> 8);cur++;
                (*cur) = clip(( 298 * c + 516 * d           + 128) >> 8);cur+=2;

                c = *(t+2) - 16;    // Y2
                (*cur) = clip(( 298 * c           + 409 * e + 128) >> 8);cur++;
                (*cur) = clip(( 298 * c - 100 * d - 208 * e + 128) >> 8);cur++;
                (*cur) = clip(( 298 * c + 516 * d           + 128) >> 8);cur+=2;

                t += 4;
            }
            line += t_width<<2;
        }
    }

    else if( m_color == YUV420 || m_color == NV12 || m_color == NV21 ){
        stride_uv = (width+1)>>1;

        for( j = 0 ; j < height ; j++ ){
            cur = line;
            for( i = 0 ; i < width ; i++ ){
                c = y[j*width+i] - 16;

                if (m_color == YUV420)
                {
                    d = u[(j>>1)*stride_uv+(i>>1)] - 128;
                    e = v[(j>>1)*stride_uv+(i>>1)] - 128;
                }
                else if (m_color == NV12)
                {
                    d = misc[(j>>1)*width+(i>>1<<1)  ] - 128;
                    e = misc[(j>>1)*width+(i>>1<<1)+1] - 128;
                }
                else // if (m_color == NV21)
                {
                    d = misc[(j>>1)*width+(i>>1<<1)+1] - 128;
                    e = misc[(j>>1)*width+(i>>1<<1)  ] - 128;
                }

                (*cur) = clip(( 298 * c           + 409 * e + 128) >> 8);cur++;
                (*cur) = clip(( 298 * c - 100 * d - 208 * e + 128) >> 8);cur++;
                (*cur) = clip(( 298 * c + 516 * d           + 128) >> 8);cur+=2;
            }
            line += t_width<<2;
        }
    }
    
    else if( m_color == YUV420_10LE || m_color == YUV420_10BE ){
        for( j = 0 ; j < height ; j++ ){
            cur = line;
            for( i = 0 ; i < width ; i++ ){

                if (m_color == YUV420_10BE)
                {
                    c = (y[j*width*2 +  i*2] << 8) | y[j*width*2 +  i*2 + 1];
                    d = (u[(j>>1)*width+(i>>1<<1)  ] << 8) | u[(j>>1)*width+(i>>1<<1)+1];
                    e = (v[(j>>1)*width+(i>>1<<1)  ] << 8) | v[(j>>1)*width+(i>>1<<1)+1];
                }
                else
                {
                    c = (y[j*width*2 +  i*2 + 1] << 8)  | y[j*width*2 +  i*2];
                    d = (u[(j>>1)*width+(i>>1<<1)+1] << 8)  | u[(j>>1)*width+(i>>1<<1)  ];
                    e = (v[(j>>1)*width+(i>>1<<1)+1] << 8)  | v[(j>>1)*width+(i>>1<<1)  ];
                }

                c = c - (16<<2);
                d = d - (128<<2);
                e = e - (128<<2);

                (*cur) = clip(( 298 * c           + 409 * e + (128<<2)) >> 10);cur++;
                (*cur) = clip(( 298 * c - 100 * d - 208 * e + (128<<2)) >> 10);cur++;
                (*cur) = clip(( 298 * c + 516 * d           + (128<<2)) >> 10);cur+=2;
            }
            line += t_width<<2;
        }
    }

    else if (m_color == RGB32 || m_color == RGB24 || m_color == RGB16) {
        for( j = 0 ; j < height ; j++ ){
            cur = line;
            for( i = 0 ; i < width ; i++ ){
                if (m_color == RGB32) {
                    r = misc[(j*width+i)*4  ];
                    g = misc[(j*width+i)*4+1];
                    b = misc[(j*width+i)*4+2];
                }
                else if (m_color == RGB24) {
                    r = misc[(j*width+i)*3  ];
                    g = misc[(j*width+i)*3+1];
                    b = misc[(j*width+i)*3+2];
                }
                else {
                    rgb16 = (short*)misc;

                    r = ((rgb16[j*width+i] >> 11)&0x1F) << 3;
                    g = ((rgb16[j*width+i] >> 5 )&0x3F) << 2;
                    b = ((rgb16[j*width+i]      )&0x1F) << 3;
                }

                (*cur) = r; cur++;
                (*cur) = g; cur++;
                (*cur) = b; cur+=2;
            }
            line += t_width<<2;
        }    
    }

    else { // YYY
        for( j = 0 ; j < height ; j++ ){
            cur = line;
            for( i = 0 ; i < width ; i++ ){
                (*cur) = y[j*width+i]; cur++;
                (*cur) = y[j*width+i]; cur++;
                (*cur) = y[j*width+i]; cur+=2;
            }
            line += t_width<<2;
        }    
    }
}
#endif

int RenderObj::doRendering()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("resources/4.5.texture.vs", "resources/4.5.texture.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // positions          // colors           // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    // load and create a texture 
    // -------------------------
    unsigned int texture1, texture2;
    // texture 1
    // ---------
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    unsigned char *data = stbi_load("resources/container.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    // texture 2
    // ---------
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
	data = stbi_load("resources/awesomeface.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        // note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------
    ourShader.use(); // don't forget to activate/use the shader before setting uniforms!
    // either set it manually like so:
    glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), 0);
    // or set it via the texture class
    ourShader.setInt("texture2", 1);

    int cnt = 0;
    
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        
        //查看帧队列是否有数据，有则渲染
        AV_FRAME_DATA_PTR pData = m_pDateQue->getData();
        if (pData.get() == nullptr) {
            std::this_thread::sleep_for(std::chrono::milliseconds(40));
            continue;
        }        

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, texture2);

        // set the texture mix value in the shader
        ourShader.setFloat("mixValue", mixValue);

        // render container
        ourShader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}






RenderObj::RenderObj(AvDataQueue* pDateQue) : m_stop(false), m_pDateQue(pDateQue)
{
}

RenderObj::~RenderObj()
{
    if (m_stop != true) {
        m_stop = true;
        if (m_renderingThr.get() != nullptr) {
            std::cout << "stop renderingThread.\n";
            m_renderingThr->join();
        }
    }

    std::cout << "~RenderObj suc.\n";
}

int RenderObj::init()
{
        
    try {
        m_renderingThr = std::make_shared<std::thread>([this]() { renderingThread(); });
    } catch (...) {
        std::cout << "create AvConsumer Thread failed.\n";
        return -1;
    }

    return 0;
}

void RenderObj::renderingThread() 
{
    std::cout << "enter renderingThread.\n";
    
    // render loop
    // -----------
    while (!m_stop)
    {
        doRendering();
    }

    std::cout << "exit renderingThread.\n";
}

}

